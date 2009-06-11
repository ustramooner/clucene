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

//#define LIBRDF_INTERNAL //we need access to internals...

#ifdef HAVE_CONFIG_H
#include <rdf_config.h>
#endif

#ifdef WIN32
#include <win32_rdf_config.h>
#endif

#include <sys/types.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h> /* for abort() as used in errors */
#endif
#include <iostream>

#include <redland.h>
#include <rdf_storage.h>
#include "redlandpp/CLuceneStorage.hpp"
#include "redlandpp/Wrapper.hpp"

#ifndef LIBRDF_INTERNAL
/** A storage object */
struct librdf_storage_s
{
  librdf_world *world;
  int usage;
  librdf_model *model;
  void *context;
  int index_contexts;
  struct librdf_storage_factory_s* factory;
};

/** A Storage Factory */
struct librdf_storage_factory_s {
  char* name;
  char* label;

  size_t context_length;
  int (*init)(librdf_storage* storage, const char *name, librdf_hash* options);
  int (*clone)(librdf_storage* new_storage, librdf_storage* old_storage);
  void (*terminate)(librdf_storage* storage);
  int (*open)(librdf_storage* storage, librdf_model* model);
  int (*close)(librdf_storage* storage);
  int (*size)(librdf_storage* storage);
  int (*add_statement)(librdf_storage* storage, librdf_statement* statement);
  int (*add_statements)(librdf_storage* storage, librdf_stream* statement_stream);
  int (*remove_statement)(librdf_storage* storage, librdf_statement* statement);
  librdf_stream* (*serialise)(librdf_storage* storage);
  librdf_stream* (*find_statements)(librdf_storage* storage, librdf_statement* statement);
  librdf_stream* (*find_statements_with_options)(librdf_storage* storage, librdf_statement* statement, librdf_node* context_node, librdf_hash* options);
  librdf_iterator* (*find_sources)(librdf_storage* storage, librdf_node *arc, librdf_node *target);
  librdf_iterator* (*find_arcs)(librdf_storage* storage, librdf_node *source, librdf_node *target);
  librdf_iterator* (*find_targets)(librdf_storage* storage, librdf_node *source, librdf_node *target);
  librdf_iterator* (*get_arcs_in)(librdf_storage *storage, librdf_node *node);
  librdf_iterator* (*get_arcs_out)(librdf_storage *storage, librdf_node *node);
  int (*context_add_statement)(librdf_storage* storage, librdf_node* context, librdf_statement *statement);
  int (*context_remove_statement)(librdf_storage* storage, librdf_node* context, librdf_statement *statement);
  librdf_stream* (*context_serialise)(librdf_storage* storage, librdf_node* context);
  int (*sync)(librdf_storage* storage);
  int (*context_add_statements)(librdf_storage* storage, librdf_node* context, librdf_stream *stream);
  int (*context_remove_statements)(librdf_storage* storage, librdf_node* context);
  librdf_stream* (*find_statements_in_context)(librdf_storage* storage, librdf_statement* statement, librdf_node* context_node);
  librdf_iterator* (*get_contexts)(librdf_storage* storage);
  librdf_node* (*get_feature)(librdf_storage* storaage, librdf_uri* feature);
  int (*set_feature)(librdf_storage* storage, librdf_uri* feature, librdf_node* value);
  int (*transaction_start)(librdf_storage* storage);
  int (*transaction_start_with_handle)(librdf_storage* storage, void* handle);
  int (*transaction_commit)(librdf_storage* storage);
  int (*transaction_rollback)(librdf_storage* storage);
  void* (*transaction_get_handle)(librdf_storage* storage);
};
#endif

void
librdf_storage_clucene_initialise(librdf_world* world){
      //tell rdf about our storage type...
      librdf_storage_register_factory(world, "clucene", "clucene backend",
                                  &librdf_storage_clucene_register_factory);
}
void
librdf_storage_clucene_shutdown(){
}

/** Local entry point for dynamically loaded storage module */
void
librdf_storage_clucene_register_factory(librdf_storage_factory *factory)
{
  assert(!strcmp(factory->name, "clucene"));

  factory->init                            = librdf_storage_clucene_init;
  factory->clone                           = librdf_storage_clucene_clone;
  factory->terminate                       = librdf_storage_clucene_terminate;
  factory->open                            = librdf_storage_clucene_open;
  factory->close                           = librdf_storage_clucene_close;
  factory->size                            = librdf_storage_clucene_size;

  //factory->serialise                       = librdf_storage_clucene_serialise;

  factory->find_statements                 = librdf_storage_clucene_find_statements;
  //factory->find_statements_in_context      = librdf_storage_clucene_find_statements_in_context;
  //factory->find_statements_with_options    = librdf_storage_clucene_find_statements_with_options;
  //factory->find_sources                    = librdf_storage_clucene_find_sources;
  //factory->find_arcs                       = librdf_storage_clucene_find_arcs;
  //factory->contains_statement              = librdf_storage_clucene_contains_statement;
  //factory->find_arcs_in                    = librdf_storage_clucene_find_arcs_in;
  //factory->find_arcs_out                   = librdf_storage_clucene_find_arcs_out;
  //factory->has_arc_in                      = librdf_storage_clucene_has_arcs_out;
  //factory->has_arc_out                     = librdf_storage_clucene_has_arcs_out;
  //factory->find_targets                    = librdf_storage_clucene_find_targets;

  //factory->sync                            = librdf_storage_clucene_sync;
  //factory->get_contexts                    = librdf_storage_clucene_get_contexts;
  //factory->get_feature                     = librdf_storage_clucene_get_feature;

  //factory->transaction_start               = librdf_storage_clucene_transaction_start;
  //factory->transaction_commit              = librdf_storage_clucene_transaction_commit;
}


typedef struct
{
  Redland::CLuceneStorageImpl* instance;
} librdf_storage_clucene_instance;


//initialise the storage...
int librdf_storage_clucene_init(librdf_storage* storage, const char *name, librdf_hash* options){
  //create the instance data...
  librdf_storage_clucene_instance* context=(librdf_storage_clucene_instance*)calloc(1, sizeof(librdf_storage_clucene_instance));
  //and assign it to the storage
  storage->context = context;

  //and assign the C++ object.
  context->instance = new Redland::CLuceneStorageImpl(storage, storage->world);

  string _name = "";
  if ( name != 0 ) _name = name;
  context->instance->Init(_name,options);
  return 0;
}
void librdf_storage_clucene_terminate(librdf_storage* storage){
  librdf_storage_clucene_instance* context=(librdf_storage_clucene_instance*)storage->context;

  delete context->instance;
}
int librdf_storage_clucene_clone(librdf_storage* new_storage, librdf_storage* old_storage){
  assert(false);
  librdf_storage_clucene_instance* old_context=(librdf_storage_clucene_instance*)old_storage->context;

  //create the instance data...
  librdf_storage_clucene_instance* newcontext=(librdf_storage_clucene_instance*)calloc(1, sizeof(librdf_storage_clucene_instance));
  //and assign it to the storage
  new_storage->context = newcontext;
  //and clone the c++ object.
  newcontext->instance = new Redland::CLuceneStorageImpl(*old_context->instance);
}
int librdf_storage_clucene_open(librdf_storage* storage, librdf_model* model){
  librdf_storage_clucene_instance* context=(librdf_storage_clucene_instance*)storage->context;
  return context->instance->Open(model);
}
int librdf_storage_clucene_close(librdf_storage* storage){
  librdf_storage_clucene_instance* context=(librdf_storage_clucene_instance*)storage->context;
  return context->instance->Close();
}
int librdf_storage_clucene_size(librdf_storage* storage){
  librdf_storage_clucene_instance* context=(librdf_storage_clucene_instance*)storage->context;
  return context->instance->Size();
}
int librdf_storage_clucene_contains_statement(librdf_storage* storage, librdf_statement* statement){
  assert(false);
  librdf_storage_clucene_instance* context=(librdf_storage_clucene_instance*)storage->context;
  return 0; //TODO:
}
/*librdf_iterator* librdf_storage_clucene_find_sources(librdf_storage* storage, librdf_node* arc, librdf_node *target){
  librdf_storage_clucene_instance* context=(librdf_storage_clucene_instance*)storage->instance;
  assert(false);
  return NULL; //TODO:
}
librdf_iterator* librdf_storage_clucene_find_targets(librdf_storage* storage, librdf_node* source, librdf_node *arc){
  librdf_storage_clucene_instance* context=(librdf_storage_clucene_instance*)storage->instance;
  assert(false);
  return NULL; //TODO:
}
librdf_stream* librdf_storage_clucene_serialise(librdf_storage* storage){
  assert(false);
  librdf_storage_clucene_instance* context=(librdf_storage_clucene_instance*)storage->instance;
  return NULL; //TODO:
}*/
librdf_stream* librdf_storage_clucene_find_statements(librdf_storage* storage, librdf_statement* statement){
  librdf_storage_clucene_instance* context=(librdf_storage_clucene_instance*)storage->context;
  return context->instance->FindStatements(statement);
}
librdf_iterator* librdf_storage_clucene_find_arcs(librdf_storage* storage, librdf_node* source, librdf_node *target){
  librdf_storage_clucene_instance* context=(librdf_storage_clucene_instance*)storage->context;
  assert(false);
  return NULL; //TODO:
}


//==============Stream wrapper===============
void
librdf_storage_clucene_stream_free(void* context){
  Redland::CLuceneStream* stream=(Redland::CLuceneStream*)context;
  delete stream;
}
int
librdf_storage_clucene_stream_is_end(void* context){
  Redland::CLuceneStream* stream=(Redland::CLuceneStream*)context;
  return stream->is_end() ? 1 : 0;
}
int
librdf_storage_clucene_stream_goto_next(void* context){
  Redland::CLuceneStream* stream=(Redland::CLuceneStream*)context;
  return stream->goto_next() ? 0 : 1;
}
void*
librdf_storage_clucene_stream_get_statement(void* context, int flags)
{
  Redland::CLuceneStream* stream=(Redland::CLuceneStream*)context;
  return stream->get_statement(flags);
}


