#ifndef RDF_STORAGE_CLUCENE_H
#define RDF_STORAGE_CLUCENE_H

#ifdef HAVE_CONFIG_H
#include <redlandpp_config.h>
#endif

#include <redland.h>

//the only 2 functions you should need...
extern void librdf_storage_clucene_initialise(librdf_world* world);
extern void librdf_storage_clucene_shutdown();

/* prototypes for local functions */
extern int librdf_storage_clucene_init(librdf_storage* storage, const char *name, librdf_hash* options);
extern void librdf_storage_clucene_terminate(librdf_storage* storage);
extern int librdf_storage_clucene_clone(librdf_storage* new_storage, librdf_storage* old_storage);
extern int librdf_storage_clucene_open(librdf_storage* storage, librdf_model* model);
extern int librdf_storage_clucene_close(librdf_storage* storage);
extern int librdf_storage_clucene_size(librdf_storage* storage);
extern int librdf_storage_clucene_contains_statement(librdf_storage* storage, librdf_statement* statement);
extern librdf_stream* librdf_storage_clucene_serialise(librdf_storage* storage);
extern librdf_stream* librdf_storage_clucene_find_statements(librdf_storage* storage, librdf_statement* statement);
extern librdf_stream* librdf_storage_clucene_find_statements_in_context(librdf_storage* storage, librdf_statement* statement,
  librdf_node* context_node);
extern librdf_stream* librdf_storage_clucene_find_statements_with_options(librdf_storage* storage, librdf_statement* statement,
  librdf_node* context_node,
  librdf_hash* options);
extern librdf_iterator* librdf_storage_clucene_find_sources(librdf_storage* storage, librdf_node* arc, librdf_node *target);
extern librdf_iterator* librdf_storage_clucene_find_arcs(librdf_storage* storage, librdf_node* source, librdf_node *target);
extern librdf_iterator* librdf_storage_clucene_find_targets(librdf_storage* storage, librdf_node* source, librdf_node *arc);
extern void librdf_storage_clucene_register_factory(librdf_storage_factory *factory);


#endif
