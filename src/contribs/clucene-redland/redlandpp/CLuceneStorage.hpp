/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * Storage.cpp - Redland C++ Storage class interface
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

#ifndef RDF_STORAGE_CLUCENE_HPP
#define RDF_STORAGE_CLUCENE_HPP

#ifdef HAVE_CONFIG_H
#include <redlandpp_config.h>
#endif

#include <redland.h>
#include <CLucene.h>
#include "redlandpp/Wrapper.hpp"


//the only 2 functions you should need...
void librdf_storage_clucene_initialise(librdf_world* world);
void librdf_storage_clucene_shutdown();

/* prototypes for local functions */
static int librdf_storage_clucene_init(librdf_storage* storage, const char *name, librdf_hash* options);
static void librdf_storage_clucene_terminate(librdf_storage* storage);
static int librdf_storage_clucene_clone(librdf_storage* new_storage, librdf_storage* old_storage);
static int librdf_storage_clucene_open(librdf_storage* storage, librdf_model* model);
static int librdf_storage_clucene_close(librdf_storage* storage);
static int librdf_storage_clucene_size(librdf_storage* storage);
static int librdf_storage_clucene_contains_statement(librdf_storage* storage, librdf_statement* statement);
static librdf_stream* librdf_storage_clucene_serialise(librdf_storage* storage);
static librdf_stream* librdf_storage_clucene_find_statements(librdf_storage* storage, librdf_statement* statement);
static librdf_stream* librdf_storage_clucene_find_statements_in_context(librdf_storage* storage, librdf_statement* statement,
  librdf_node* context_node);
static librdf_stream* librdf_storage_clucene_find_statements_with_options(librdf_storage* storage, librdf_statement* statement,
  librdf_node* context_node,
  librdf_hash* options);
static librdf_iterator* librdf_storage_clucene_find_sources(librdf_storage* storage, librdf_node* arc, librdf_node *target);
static librdf_iterator* librdf_storage_clucene_find_arcs(librdf_storage* storage, librdf_node* source, librdf_node *target);
static librdf_iterator* librdf_storage_clucene_find_targets(librdf_storage* storage, librdf_node* source, librdf_node *arc);
static void librdf_storage_clucene_register_factory(librdf_storage_factory *factory);


namespace Redland {


  class CLuceneStorageImpl : public Wrapper<librdf_storage> {
    lucene::store::Directory* directory;
    lucene::index::IndexReader* reader;
    void ensureOpen();
    std::wstring CharToString(const unsigned char*);
    std::wstring CharToString(const char*);
    std::string StringToChar(std::wstring str);
    std::wstring NodeToKeyword(librdf_node*);
    std::string NodeToString(librdf_node*);

  public:
    static const std::string documentNumberUri;//< uri used for doc numbers...
    enum { documentNumberUriSize = 31 }; //< size of documentNumberUri - used for buffer sizes
    librdf_world* world;
    static const TCHAR* idFieldName();

    CLuceneStorageImpl(librdf_storage* storage, librdf_world* world);
    CLuceneStorageImpl(const CLuceneStorageImpl&);
    ~CLuceneStorageImpl();
    int Init(const std::string& name, librdf_hash* options);
    void Terminate(librdf_storage* storage);
    int Open(librdf_model* model);
    int Close();
    int Size();
    //void ContainsStatement(Statement* statement);
    //Stream* Serialise();
    librdf_stream* FindStatementsWithOptions_SQL(librdf_statement* statement, librdf_node* context_node, librdf_hash* options);
    librdf_stream* FindStatementsWithOptions(librdf_statement* statement, librdf_node* context_node, librdf_hash* options);
    //FindSources(librdf_storage* storage, librdf_node* arc, librdf_node *target);
    //FindArcs(librdf_storage* storage, librdf_node* source, librdf_node *target);
    //FindTargets(librdf_storage* storage, librdf_node* source, librdf_node *arc);

    //helper functions for streams, etc
    bool setStatementObject(const TCHAR* fromField, int32_t fromDocNum, librdf_statement* statement);
  };



} // namespace Redland

#endif
