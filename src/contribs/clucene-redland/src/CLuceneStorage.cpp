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
#include <rdf_storage_module.h>
#include "redlandpp/Wrapper.hpp"
#include "redlandpp/CLuceneStorage.hpp"

namespace Redland {

using namespace std;
using namespace lucene::document;
using namespace lucene::index;
using namespace lucene::store;
using namespace lucene::search;

// ================ Common stream stuff ============
typedef struct{
  CLuceneStorageImpl* storage;
  bool is_literal_match;
  librdf_statement* current_statement;
  librdf_node *current_context;
  librdf_statement *query_statement;
  librdf_node *query_context;
} librdf_storage_clucene_common_stream_data;

void
librdf_storage_clucene_common_stream_init(librdf_storage_clucene_common_stream_data* data,
          CLuceneStorageImpl* storage, bool is_literal_match ){
    data->storage = storage;
    data->is_literal_match = is_literal_match;
    data->current_statement = NULL;
    data->current_context = NULL;
    data->query_statement = NULL;
    data->query_context = NULL;
}
static void
librdf_storage_clucene_common_stream_free(librdf_storage_clucene_common_stream_data* data){
  if ( data->current_statement ) librdf_free_statement(data->current_statement);
  if ( data->current_context ) librdf_free_node(data->current_context);
  if ( data->query_context ) librdf_free_node(data->query_context);
}
static int
librdf_storage_clucene_common_stream_goto_next(librdf_storage_clucene_common_stream_data* stream){
  librdf_node *subject=NULL, *predicate=NULL, *object=NULL;
  librdf_node *node;

  /* Get ready for context */
  if(stream->current_context)
    librdf_free_node(stream->current_context);
  stream->current_context=NULL;

  /* Is this a query with statement parts? */
  if(stream->query_statement) {
    subject=librdf_statement_get_subject(stream->query_statement);
    predicate=librdf_statement_get_predicate(stream->query_statement);
    if(stream->is_literal_match)
      object=NULL;
    else
      object=librdf_statement_get_object(stream->query_statement);
  }

/*printf("===========XXXX==========\n");
if ( subject )
printf("subject=%s\n", (char*)librdf_node_to_string(subject));
if ( (predicate) )
printf("predicate=%s\n", (char*)librdf_node_to_string(predicate));
if ( (object) )
printf("object=%s\n", (char*)librdf_node_to_string((object)));
*/

  /* Query without variables? */
  if(subject && predicate && object && stream->query_context) {
    librdf_statement_set_subject(stream->current_statement,librdf_new_node_from_node(subject));
    librdf_statement_set_predicate(stream->current_statement,librdf_new_node_from_node(predicate));
    librdf_statement_set_object(stream->current_statement,librdf_new_node_from_node(object));
    stream->current_context=librdf_new_node_from_node(stream->query_context);
  } else {
    /* Subject - constant or from row? */
    if(subject) {
      librdf_statement_set_subject(stream->current_statement,librdf_new_node_from_node(subject));
    } else {
      /* Resource or Bnode?
      if(row[part]) {
        if(!(node=librdf_new_node_from_uri_string(stream->storage->world,
                                                   (const unsigned char*)row[part])))
          return 1;
      } else if(row[part+1]) {
        if(!(node=librdf_new_node_from_blank_identifier(stream->storage->world,
                                                         (const unsigned char*)row[part+1])))
          return 1;
      } else
        return 1;

      librdf_statement_set_subject(stream->current_statement,node);
      part+=2;*/
      assert( librdf_statement_get_subject(stream->current_statement) != NULL );
    }
    /* Predicate - constant or from row? */
    if(predicate) {
      librdf_statement_set_predicate(stream->current_statement,librdf_new_node_from_node(predicate));
    } else {
      /* Resource?
      if(row[part]) {
        if(!(node=librdf_new_node_from_uri_string(stream->storage->world,
                                                   (const unsigned char*)row[part])))
          return 1;
      } else
        return 1;

      librdf_statement_set_predicate(stream->current_statement,node);
      part+=1;*/
      assert( librdf_statement_get_predicate(stream->current_statement) != NULL );
    }
    /* Object - constant or from row? */
    if(object) {
      librdf_statement_set_object(stream->current_statement,librdf_new_node_from_node(object));
    } else {
      /* Resource, Bnode or Literal?
      if(row[part]) {
        if(!(node=librdf_new_node_from_uri_string(stream->storage->world,
                                                   (const unsigned char*)row[part])))
          return 1;
      } else if(row[part+1]) {
        if(!(node=librdf_new_node_from_blank_identifier(stream->storage->world,
                                                         (const unsigned char*)row[part+1])))
          return 1;
      } else if(row[part+2]) {
        // Typed literal?
        librdf_uri *datatype=NULL;
        if(row[part+4] && strlen(row[part+4]))
          datatype=librdf_new_uri(stream->storage->world,
                                  (const unsigned char*)row[part+4]);
        if(!(node=librdf_new_node_from_typed_literal(stream->storage->world,
                                                      (const unsigned char*)row[part+2],
                                                      row[part+3],
                                                      datatype)))
          return 1;
      } else
        return 1;

      librdf_statement_set_object(stream->current_statement,node);
      part+=5;*/
      assert( librdf_statement_get_object(stream->current_statement) != NULL );
    }
    /* Context - constant or from row? */
    if(stream->query_context) {
      stream->current_context=librdf_new_node_from_node(stream->query_context);
    } else {
      /* Resource, Bnode or Literal?
      if(row[part]) {
        if(!(node=librdf_new_node_from_uri_string(stream->storage->world,
                                                   (const unsigned char*)row[part])))
          return 1;
      } else if(row[part+1]) {
        if(!(node=librdf_new_node_from_blank_identifier(stream->storage->world,
                                                         (const unsigned char*)row[part+1])))
          return 1;
      } else if(row[part+2]) {
        // Typed literal?
        librdf_uri *datatype=NULL;
        if(row[part+4] && strlen(row[part+4]))
          datatype=librdf_new_uri(stream->storage->world,
                                  (const unsigned char*)row[part+4]);
        if(!(node=librdf_new_node_from_typed_literal(stream->storage->world,
                                                      (const unsigned char*)row[part+2],
                                                      row[part+3],
                                                      datatype)))
          return 1;
      } else
        // no context
        node=NULL;
        */
      //assert(false);
      //stream->current_context=node;
    }
  }
  return 0;

}

// ================ TermDocs stream ============
typedef struct{
  Term* term;
  TermDocs* termDocs;
  char textBuffer[CLuceneStorageImpl::documentNumberUriSize + 11];
  librdf_storage_clucene_common_stream_data common;
} librdf_storage_clucene_termdocs_stream_data;

static void
librdf_storage_clucene_termdocs_stream_init(librdf_storage_clucene_termdocs_stream_data* data,
          CLuceneStorageImpl* storage, bool is_literal_match ){
    data->term = NULL;
    data->termDocs = NULL;
    *data->textBuffer = 0;
    librdf_storage_clucene_common_stream_init(&data->common, storage, is_literal_match);
}
static void
librdf_storage_clucene_termdocs_stream_free(void* context){
  librdf_storage_clucene_termdocs_stream_data* stream=(librdf_storage_clucene_termdocs_stream_data*)context;

  _CLDECDELETE(stream->term);
  _CLDELETE(stream->termDocs);
  librdf_storage_clucene_common_stream_free(&stream->common);
  free(stream);
}
static int
librdf_storage_clucene_termdocs_stream_is_end(void* context){
  librdf_storage_clucene_termdocs_stream_data* stream=(librdf_storage_clucene_termdocs_stream_data*)context;

  return stream->termDocs == NULL  < 0 ? 1 : 0;
}
static int
librdf_storage_clucene_termdocs_stream_goto_next(void* context){
  librdf_storage_clucene_termdocs_stream_data* stream=(librdf_storage_clucene_termdocs_stream_data*)context;

  if ( stream->termDocs->next() ){

    /* Make sure we have a statement object to return */
    if(!stream->common.current_statement) {
      if(!(stream->common.current_statement=librdf_new_statement(stream->common.storage->world)))
        return 1;
    }else{
      librdf_statement_clear(stream->common.current_statement);
    }

    //fill in the resource
    sprintf(stream->textBuffer + CLuceneStorageImpl::documentNumberUriSize, "%d", stream->termDocs->doc());
    librdf_statement_set_subject(stream->common.current_statement,
      librdf_new_node_from_uri_string(stream->common.storage->world,
      (const unsigned char*)stream->textBuffer));

    return librdf_storage_clucene_common_stream_goto_next(&stream->common);
  }else {
    if(stream->common.current_statement)
      librdf_free_statement(stream->common.current_statement);
    stream->common.current_statement=NULL;

    if(stream->common.current_context)
      librdf_free_node(stream->common.current_context);

    stream->common.current_context=NULL;

    _CLDELETE( stream->termDocs );
    return 1;
  }

}

static void*
librdf_storage_clucene_termdocs_stream_get_statement(void* context, int flags)
{
  librdf_storage_clucene_termdocs_stream_data* stream=(librdf_storage_clucene_termdocs_stream_data*)context;

  switch(flags) {
    case LIBRDF_ITERATOR_GET_METHOD_GET_OBJECT:

/*
  printf("===========RETURNING==========\n");
if ( librdf_statement_get_subject(stream->common.current_statement) )
  printf("subject=%s\n", (char*)librdf_node_to_string(librdf_statement_get_subject(stream->common.current_statement)));
if ( librdf_statement_get_predicate(stream->common.current_statement) )
  printf("predicate=%s\n", (char*)librdf_node_to_string(librdf_statement_get_predicate(stream->common.current_statement)));
if ( librdf_statement_get_object(stream->common.current_statement) )
  printf("object=%s\n", (char*)librdf_node_to_string(librdf_statement_get_object(stream->common.current_statement)));
*/
      return stream->common.current_statement;
    case LIBRDF_ITERATOR_GET_METHOD_GET_CONTEXT:
      assert(false);
    default:
      abort();
  }
}

// ================ Single Doc stream ============
typedef struct{
  string* buffer;
  librdf_storage_clucene_common_stream_data common;
} librdf_storage_clucene_singledoc_stream_data;

static void
librdf_storage_clucene_singledoc_stream_init(librdf_storage_clucene_singledoc_stream_data* data, CLuceneStorageImpl* storage,  bool is_literal_match){
    librdf_storage_clucene_common_stream_init(&data->common, storage, is_literal_match);
    data->buffer = new string;
}
static void
librdf_storage_clucene_singledoc_stream_free(void* context){
  librdf_storage_clucene_singledoc_stream_data* stream=(librdf_storage_clucene_singledoc_stream_data*)context;
  librdf_storage_clucene_common_stream_free(&stream->common);
  delete stream->buffer;
  free(stream);
}
static int
librdf_storage_clucene_singledoc_stream_is_end(void* context){
  librdf_storage_clucene_singledoc_stream_data* stream=(librdf_storage_clucene_singledoc_stream_data*)context;

  return stream->buffer->empty() ? 1 : 0;
}
static int
librdf_storage_clucene_singledoc_stream_goto_next(void* context){
  librdf_storage_clucene_singledoc_stream_data* stream=(librdf_storage_clucene_singledoc_stream_data*)context;

  //only one doc available...

  /* Make sure we have a statement object to return */
  if(!stream->common.current_statement) {
    if(!(stream->common.current_statement=librdf_new_statement(stream->common.storage->world)))
      return 1;
  }else{
    librdf_statement_clear(stream->common.current_statement);
  }

  //fill in the resource

  librdf_statement_set_object(stream->common.current_statement,
    librdf_new_node_from_literal(stream->common.storage->world,
      (const unsigned char*)stream->buffer->c_str(), NULL,0));

  int ret = librdf_storage_clucene_common_stream_goto_next(&stream->common);

  return ret;
}

static void*
librdf_storage_clucene_singledoc_stream_get_statement(void* context, int flags)
{
  librdf_storage_clucene_singledoc_stream_data* stream=(librdf_storage_clucene_singledoc_stream_data*)context;

  switch(flags) {
    case LIBRDF_ITERATOR_GET_METHOD_GET_OBJECT:
/*
  printf("===========RETURNING==========\n");
if ( librdf_statement_get_subject(stream->common.current_statement) )
  printf("subject=%s\n", (char*)librdf_node_to_string(librdf_statement_get_subject(stream->common.current_statement)));
if ( librdf_statement_get_predicate(stream->common.current_statement) )
  printf("predicate=%s\n", (char*)librdf_node_to_string(librdf_statement_get_predicate(stream->common.current_statement)));
if ( librdf_statement_get_object(stream->common.current_statement) )
  printf("object=%s\n", (char*)librdf_node_to_string(librdf_statement_get_object(stream->common.current_statement)));
*/
stream->buffer->clear();

      return stream->common.current_statement;
    case LIBRDF_ITERATOR_GET_METHOD_GET_CONTEXT:
      assert(false);
    default:
      abort();
  }
}

// ================ CLuceneStorageImpl class ============

const std::string CLuceneStorageImpl::documentNumberUri = "http://clucene.sf.net/document#";

CLuceneStorageImpl::CLuceneStorageImpl(librdf_storage* storage, librdf_world* world_):
  Redland::Wrapper<librdf_storage>((redland_object_free*)librdf_free_storage, storage),
  directory(NULL),
  world(world_),
  reader(NULL)
{
}
/*
CLuceneStorageImpl::CLuceneStorageImpl(const CLuceneStorageImpl& clone)
  Redland::Wrapper<librdf_storage>((redland_object_free*)librdf_free_storage, storage),
{
}*/
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
  if ( reader == NULL ){
    reader = IndexReader::open(directory);
  }
}

int CLuceneStorageImpl::Close(){
  return 0;
}
int CLuceneStorageImpl::Open(librdf_model* model){
  return 0;
}

int CLuceneStorageImpl::Size(){
  ensureOpen();
  //TODO: this is not correct because rdf is asking for triplets, not documents...
  //go through each field and pull out how many terms are in it and add them all up...
  return reader->numDocs();
}

librdf_stream* CLuceneStorageImpl::FindStatementsWithOptions(
  librdf_statement* statement,
  librdf_node* context_node,
  librdf_hash* options){
  ensureOpen();

  librdf_node *subject, *predicate, *object;
  bool is_literal_match = false;
  string where;
  string joins;

  if ( options )
    is_literal_match = librdf_hash_get_as_boolean(options, "match-substring");

  if(statement) {
    subject=librdf_statement_get_subject(statement);
    predicate=librdf_statement_get_predicate(statement);
    object=librdf_statement_get_object(statement);
  }

  printf("===========FindStatementsWithOptions==========\n");
  if ( subject )
    printf("subject=%s\n", NodeToString(subject).c_str());
  if ( predicate )
    printf("predicate=%s\n", NodeToString(predicate).c_str());
  if ( object )
    printf("object=%s\n", NodeToString(object).c_str());
  printf("\n");

  if (predicate && object && !subject){
    //simple term query...
    librdf_storage_clucene_termdocs_stream_data* streamData;
    streamData = (librdf_storage_clucene_termdocs_stream_data*)malloc(sizeof(librdf_storage_clucene_termdocs_stream_data));
    librdf_storage_clucene_termdocs_stream_init(streamData, this, is_literal_match );

    if ( context_node )
      streamData->common.query_context = librdf_new_node_from_node(context_node);
    if ( statement )
      streamData->common.query_statement = librdf_new_statement_from_statement(statement);

    streamData->term = _CLNEW Term(
      NodeToKeyword(predicate).c_str(),
      NodeToKeyword(object).c_str() );
    streamData->termDocs = reader->termDocs(streamData->term);

    strcpy(streamData->textBuffer, documentNumberUri.c_str());
    assert ( documentNumberUri.length() == documentNumberUriSize );

    if ( librdf_storage_clucene_termdocs_stream_goto_next(streamData) == 0 ){
      librdf_stream* stream= librdf_new_stream(this->world,(void*)streamData,
        &librdf_storage_clucene_termdocs_stream_is_end,
        &librdf_storage_clucene_termdocs_stream_goto_next,
        &librdf_storage_clucene_termdocs_stream_get_statement,
        &librdf_storage_clucene_termdocs_stream_free
       );
       return stream;
    }else{
      librdf_storage_clucene_termdocs_stream_free(streamData);
    }
  }else if (subject && predicate && !object){
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
          librdf_storage_clucene_singledoc_stream_data* streamData;
          streamData = (librdf_storage_clucene_singledoc_stream_data*)malloc(sizeof(librdf_storage_clucene_singledoc_stream_data));
          librdf_storage_clucene_singledoc_stream_init(streamData, this, is_literal_match );

          if ( context_node )
            streamData->common.query_context = librdf_new_node_from_node(context_node);
          if ( statement )
            streamData->common.query_statement = librdf_new_statement_from_statement(statement);

          Document* doc = reader->document(docNum);//todo: use field selector when upgrade
          wstring field = doc->get( NodeToKeyword(predicate).c_str() );
          streamData->buffer->assign(StringToChar(field));
          _CLDELETE(doc);

          //go to first...
          librdf_storage_clucene_singledoc_stream_goto_next(streamData);

          librdf_stream* stream= librdf_new_stream(this->world,(void*)streamData,
            &librdf_storage_clucene_singledoc_stream_is_end,
            &librdf_storage_clucene_singledoc_stream_goto_next,
            &librdf_storage_clucene_singledoc_stream_get_statement,
            &librdf_storage_clucene_singledoc_stream_free
           );
           return stream;
        }
        return librdf_new_empty_stream(this->world);
      }
    }

  }else{
    assert(false);
  }
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
