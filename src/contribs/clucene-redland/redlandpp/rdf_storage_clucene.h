#ifndef RDF_STORAGE_CLUCENE_H
#define RDF_STORAGE_CLUCENE_H

#ifdef HAVE_CONFIG_H
#include <redlandpp_config.h>
#endif

#include <redland.h>

//the only 2 functions you should need...
extern void librdf_storage_clucene_initialise(librdf_world* world);
extern void librdf_storage_clucene_shutdown();

#endif
