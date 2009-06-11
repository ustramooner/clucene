/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * Storage.cpp - Redland C++ Storage classes
 *
 * Copyright (C) 2008, David Beckett http://www.dajobe.org/
 *
 * This package is Free Software and part of Redland http://librdf.org/
 *
 * It is licensed under the following three licenses as alternatives:
 *   1. GNU Lesser General Public License (LGPL) V2.1 or any newer version
 *   2. GNU General Public License (GPL) V2 or any newer version
 *   3. Apache License, V2.0 or any newer version
 *
 * You may not use this file except in compliance with at least one of
 * the above three licenses.
 *
 * See LICENSE.html or LICENSE.txt at the top of this package for the
 * complete terms and further detail along with the license texts for
 * the licenses in COPYING.LIB, COPYING and LICENSE-2.0.txt respectively.
 *
 *
 */


#ifdef HAVE_CONFIG_H
#include <redlandpp_config.h>
#endif

#include <iostream>
#include <redland.h>
#include <rdf_storage.h>
#include "redlandpp/Wrapper.hpp"
#include "redlandpp/CLuceneStorage.hpp"

#ifndef LIBRDF_INTERNAL
  #define LIBRDF_ITERATOR_GET_METHOD_GET_OBJECT  0
  #define LIBRDF_ITERATOR_GET_METHOD_GET_CONTEXT 1
#endif

namespace Redland {

using namespace std;
using namespace lucene::document;
using namespace lucene::index;
using namespace lucene::store;
using namespace lucene::search;

// ================ Single statement stream ===================
class SingleStatementStream : public CLuceneStream {
  librdf_statement* the_statement;
  bool first;
public:
  SingleStatementStream(librdf_statement* the_statement, librdf_world* world):
  CLuceneStream(world)
  {
    this->the_statement = the_statement;
    this->first = true;
  }
  ~SingleStatementStream(){
    if ( this->the_statement ) librdf_free_statement(this->the_statement);
  }
  bool is_end(){
    return !this->first;
  }
  virtual bool goto_next(){
    return false;
  }
  virtual librdf_statement* get_statement(int flags){
    this->first = false;
    return this->the_statement;
  }

};

// ================ Common stream stuff ============
class CommonStream : public CLuceneStream {
public:
  bool is_literal_match;
  librdf_statement* current_statement;
  librdf_node *current_context;
  librdf_statement *query_statement;
  librdf_node *query_context;

  CommonStream(librdf_world* world, librdf_statement* statement, librdf_node* context_node, bool is_literal_match):
    CLuceneStream(world)
  {
    this->is_literal_match = is_literal_match;
    this->current_statement=librdf_new_statement(this->world);
    this->current_context = NULL;
    this->query_statement = NULL;
    this->query_context = NULL;

    if ( statement )
      this->query_statement = librdf_new_statement_from_statement(statement);
    if ( context_node )
      this->query_context = librdf_new_node_from_node(context_node);
  }
  virtual ~CommonStream(){
    if ( this->current_statement ) librdf_free_statement(this->current_statement);
    if ( this->current_context ) librdf_free_node(this->current_context);
    if ( this->query_context ) librdf_free_node(this->query_context);
  }
  virtual bool goto_next(){
    librdf_node *subject=NULL, *predicate=NULL, *object=NULL;

    /* Get ready for context */
    if(this->current_context)
      librdf_free_node(this->current_context);
    this->current_context=NULL;

    /* Is this a query with statement parts? */
    if(this->query_statement) {
      subject=librdf_statement_get_subject(this->query_statement);
      predicate=librdf_statement_get_predicate(this->query_statement);
      if(this->is_literal_match)
        object=NULL;
      else
        object=librdf_statement_get_object(this->query_statement);
    }

    /* Query without variables? */
    if(subject && predicate && object && this->query_context) {
      librdf_statement_set_subject(this->current_statement,librdf_new_node_from_node(subject));
      librdf_statement_set_predicate(this->current_statement,librdf_new_node_from_node(predicate));
      librdf_statement_set_object(this->current_statement,librdf_new_node_from_node(object));
      this->current_context=librdf_new_node_from_node(this->query_context);
    } else {
      /* Subject - constant or from row? */
      if(subject) {
        librdf_statement_set_subject(this->current_statement,librdf_new_node_from_node(subject));
      } else {
        assert( librdf_statement_get_subject(this->current_statement) != NULL );
      }
      /* Predicate - constant or from row? */
      if(predicate) {
        librdf_statement_set_predicate(this->current_statement,librdf_new_node_from_node(predicate));
      } else {
        assert( librdf_statement_get_predicate(this->current_statement) != NULL );
      }
      /* Object - constant or from row? */
      if(object) {
        librdf_statement_set_object(this->current_statement,librdf_new_node_from_node(object));
      } else {
        assert( librdf_statement_get_object(this->current_statement) != NULL );
      }



      /* Context - constant or from row? */
      if(this->query_context) {
        this->current_context=librdf_new_node_from_node(this->query_context);
      } else {
        //TODO:.....
      }
    }
    return true;
  }

  librdf_statement* get_statement(int flags)
  {
    switch(flags) {
      case LIBRDF_ITERATOR_GET_METHOD_GET_OBJECT:
        return this->current_statement;
      case LIBRDF_ITERATOR_GET_METHOD_GET_CONTEXT:
        assert(false);
      default:
        abort();
    }
  }
};

// ================ TermDocs stream ============
class TermDocsStream: public CommonStream {
  TermDocs* termDocs;
  char textBuffer[CLuceneStorageImpl::documentNumberUriSize + 11];

public:
  TermDocsStream(TermDocs* termDocs, librdf_world* world, librdf_statement* statement, librdf_node* context_node, bool is_literal_match):
    CommonStream(world, statement, context_node, is_literal_match)
  {
    this->termDocs = termDocs;

    strcpy(this->textBuffer, CLuceneStorageImpl::documentNumberUri.c_str());
    assert ( CLuceneStorageImpl::documentNumberUri.length() == CLuceneStorageImpl::documentNumberUriSize );
  }
  ~TermDocsStream(){
    _CLDELETE(this->termDocs);
  }
  bool is_end(){
    return this->termDocs == NULL;
  }
  bool goto_next(){
    if ( termDocs->next() ){

      /* Make sure we have a statement object to return */
      if(!current_statement) {
        if(!(current_statement=librdf_new_statement(this->world)))
          return false;
      }else{
        librdf_statement_clear(current_statement);
      }

      //fill in the resource
      sprintf(this->textBuffer + CLuceneStorageImpl::documentNumberUriSize, "%d", this->termDocs->doc());
      librdf_statement_set_subject(this->current_statement,
        librdf_new_node_from_uri_string(this->world,
        (const unsigned char*)this->textBuffer));

      return CommonStream::goto_next();
    }else {
      if(this->current_statement)
        librdf_free_statement(this->current_statement);
      this->current_statement=NULL;

      if(this->current_context)
        librdf_free_node(this->current_context);

      this->current_context=NULL;

      _CLDELETE( this->termDocs );
      return false;
    }
  }
};

// ================ CLuceneStorageImpl class ============

const std::string CLuceneStorageImpl::documentNumberUri = "http://clucene.sf.net/document#";

CLuceneStorageImpl::CLuceneStorageImpl(librdf_storage* storage, librdf_world* world_):
  Redland::Wrapper<librdf_storage>((redland_object_free*)librdf_free_storage, storage),
  directory(NULL),
  reader(NULL),
  world(world_)
{
}
const TCHAR* CLuceneStorageImpl::idFieldName(){
  return _T("http://freedesktop.org/standards/xesam/1.0/core#url");
}
bool CLuceneStorageImpl::setStatementObject(const TCHAR* fromField, int32_t fromDocNum, librdf_statement* statement){

  //TODO: MapFieldSelector map;
  //map.add(field,FieldSelector::LOAD);

  Document* doc = reader->document(fromDocNum);
  if ( doc ){
    librdf_statement_set_object(statement,
      librdf_new_node_from_literal( this->world, (const unsigned char*)StringToChar(doc->get(CLuceneStorageImpl::idFieldName())).c_str(),
      NULL, 0) );
    _CLDELETE(doc);
    return true;
  }
  return false;
}
int CLuceneStorageImpl::Init(const std::string& name, librdf_hash* options){

  char* _dir = librdf_hash_get_del(options, "dir");
  if ( _dir == NULL ){
      fprintf(stderr, "CLucene dir wasn't specified\n");
      return -1;
  }
  string dir = _dir;
  free(_dir);

  try{
    this->directory = FSDirectory::getDirectory(dir.c_str(), false);
    return 0;
  }catch(CLuceneError& err){
    fprintf(stderr, "CLucene Error: %s\n", err.what());
  }
  return -1;
}

CLuceneStorageImpl::~CLuceneStorageImpl()
{
}

void CLuceneStorageImpl::ensureOpen(){
  assert(directory != NULL);
  if ( reader == NULL ){
    reader = IndexReader::open(directory);
  }
}

int CLuceneStorageImpl::Close(){
  return 0;
}
int CLuceneStorageImpl::Open(librdf_model*){
  return 0;
}

int CLuceneStorageImpl::Size(){
  ensureOpen();
  //TODO: this is not correct because rdf is asking for triplets, not documents...
  //go through each field and pull out how many terms are in it and add them all up...
  return reader->numDocs();
}

librdf_stream* CLuceneStorageImpl::FindStatements(librdf_statement* statement){
  ensureOpen();

  librdf_node *subject, *predicate, *object;
  bool is_literal_match = false;
  string where;
  string joins;

  //if ( options )
  //  is_literal_match = librdf_hash_get_as_boolean(options, "match-substring");

  if(statement) {
    subject=librdf_statement_get_subject(statement);
    predicate=librdf_statement_get_predicate(statement);
    object=librdf_statement_get_object(statement);
  }

  if (!subject && predicate && object ){
    //? P O
    Term* term = _CLNEW Term(
      NodeToKeyword(predicate).c_str(),
      NodeToKeyword(object).c_str() );
    TermDocs* termDocs = reader->termDocs(term);
    _CLDECDELETE(term);

    //simple term query...
    TermDocsStream* stream = new TermDocsStream(termDocs, world, statement, NULL, is_literal_match );

    if ( stream->goto_next() ){
      return stream->get_stream();
    }else{
      delete stream;
    }
  }else if (subject && predicate ){ //with or without object...

    //if we are looking the value of a field in an exact document...
    if ( librdf_node_is_resource(subject) ){
      librdf_uri* uri = librdf_node_get_uri (subject);
      assert(uri != NULL);
      const char* uristr = (const char*)librdf_uri_as_string(uri);
      assert(uristr != NULL);
      if ( strncmp(uristr, CLuceneStorageImpl::documentNumberUri.c_str(), CLuceneStorageImpl::documentNumberUri.length() ) == 0 ){
        //one document...
        int docNum = atoi(uristr+CLuceneStorageImpl::documentNumberUri.length());
        if ( docNum >= 0 && docNum < reader->maxDoc() && !reader->isDeleted(docNum) ){
          //single document...
          Document* doc = reader->document(docNum);//todo: use field selector when upgrade
          const TCHAR* wfield = doc->get( NodeToKeyword(predicate).c_str() );
          if ( wfield != NULL ){
            //S P ?
            string field = StringToChar(wfield);
            if ( object == NULL ){
              librdf_statement* the_statement = librdf_new_statement_from_statement(statement);
              librdf_statement_set_object(the_statement,
                librdf_new_node_from_literal(this->world,
                  (const unsigned char*)field.c_str(), NULL,0));

              SingleStatementStream* stream = new SingleStatementStream(the_statement, world);

              _CLDELETE(doc);
              return stream->get_stream();
            }else{
              //S P O
              string object_value = NodeToString(object);
              if ( field.compare(object_value) == 0 ){
                librdf_statement* the_statement = librdf_new_statement_from_statement(statement);
                SingleStatementStream* stream = new SingleStatementStream(the_statement, world);
                return stream->get_stream();
              }
            }
          }
        }
      }
    }
  }
  assert(false); //not all paths are implemented yet
  return librdf_new_empty_stream(this->world);
}




std::wstring CLuceneStorageImpl::CharToString(const unsigned char * str){
    return CharToString( (const char*) str);
}
std::wstring CLuceneStorageImpl::CharToString(const char* str){
    size_t l = strlen(str);
    TCHAR* tmp = _CL_NEWARRAY(TCHAR,l * sizeof(TCHAR) );
    l = lucene_utf8towcs(tmp,str,l);
    wstring ret = wstring(tmp,l);
    _CLDELETE_CARRAY(tmp);
    return ret;
}
std::string CLuceneStorageImpl::StringToChar(std::wstring str){
  //todo: new version has a help function for this...
  char buf[10];
  string ret;
  ret.reserve(str.length());
  size_t l = str.length();
  size_t t = 0;
  for (size_t i=0;i<l;i++){
    t = lucene_wctoutf8(buf,str[i]);
    ret.append(buf,t);
  }
  return ret;
}
std::wstring CLuceneStorageImpl::NodeToKeyword(librdf_node* node){
    librdf_node_type type = librdf_node_get_type (node);
    if ( type == LIBRDF_NODE_TYPE_RESOURCE ){
      librdf_uri* uri = librdf_node_get_uri (node);
      assert(uri != NULL);
      return CharToString(librdf_uri_as_string(uri));
    }else if ( type == LIBRDF_NODE_TYPE_LITERAL ){
      return CharToString(librdf_node_get_literal_value (node));
    }else{
      assert(false);///
    }
}
std::string CLuceneStorageImpl::NodeToString(librdf_node* node){
    librdf_node_type type = librdf_node_get_type (node);
    if ( type == LIBRDF_NODE_TYPE_RESOURCE ){
      librdf_uri* uri = librdf_node_get_uri (node);
      assert(uri != NULL);
      return (const char*)librdf_uri_to_string(uri);
    }else if ( type == LIBRDF_NODE_TYPE_LITERAL ){
      return strdup((const char*)librdf_node_get_literal_value (node));
    }else{
      assert(false);///
    }
}
} // namespace Redland
