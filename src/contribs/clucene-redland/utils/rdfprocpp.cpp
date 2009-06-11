/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * rdfprocpp.cpp - Redland C++ Utility
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


#include <iostream>
#include <string>
#include <vector>

#include "redlandpp/rdf_storage_clucene.h"

using namespace std;

int main(int argc, char * argv[])
{
  char *program=argv[0];
  librdf_world* world;
  librdf_storage *storage;
  librdf_model* model;
  const char *parser_name;
  librdf_query* query;
  librdf_query_results* results;
  const unsigned char *query_string=NULL;

  world=librdf_new_world();
  librdf_world_open(world);
  librdf_storage_clucene_initialise(world);

/*
  if(argc !=3) {
    fprintf(stderr, "USAGE: %s CLUCENE-INDEX SPARQL-QUERY-STRING\n", program);
    //return 1;
  }

  query_string=(const unsigned char*)argv[2];

  string dir = "dir='";
  dir += argv[1];
  dir += "'";
*/
  query_string = (const unsigned char*)"PREFIX www: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> PREFIX fdo: <http://www.semanticdesktop.org/ontologies/2007/01/19/nie#> PREFIX fd: <http://freedesktop.org/standards/xesam/1.0/core#> PREFIX strigi: <http://strigi.sf.net/ontologies/0.9#> SELECT ?url, ?xB WHERE { ?x fd:fileExtension \"pdf\" . ?x fd:url ?url . ?x fdo:isPartOf ?parent . ?y fd:url ?parent . ?y www:type ?xB }";
  string dir = "dir='/home/ben/dev/svn_strigi/testindex/'";


  model=librdf_new_model(world, storage=librdf_new_storage(world, "clucene", "test", dir.c_str()), NULL);
  if(!model || !storage) {
    fprintf(stderr, "%s: Failed to make model or storage\n", program);
    return 1;
  }

  query=librdf_new_query(world, "sparql", NULL, query_string, NULL);

  results=librdf_model_query_execute(model, query);
  if(!results) {
    fprintf(stderr, "%s: Query of model with '%s' failed\n",
            program, query_string);
    return 1;
  }

  while(!librdf_query_results_finished(results)) {
    const char **names=NULL;
    librdf_node* values[10];

    if(librdf_query_results_get_bindings(results, &names, values))
      break;

    fputs("result: [", stdout);
    if(names) {
      int i;

      for(i=0; names[i]; i++) {
        fprintf(stdout, "%s=", names[i]);
        if(values[i]) {
          librdf_node_print(values[i], stdout);
          librdf_free_node(values[i]);
        } else
          fputs("NULL", stdout);
        if(names[i+1])
          fputs(", ", stdout);
      }
    }
    fputs("]\n", stdout);

    librdf_query_results_next(results);
  }

  fprintf(stdout, "%s: Query returned %d results\n", program,
          librdf_query_results_get_count(results));

  librdf_free_query_results(results);
  librdf_free_query(query);

  librdf_free_model(model);
  librdf_free_storage(storage);

  librdf_free_world(world);

#ifdef LIBRDF_MEMORY_DEBUG
  librdf_memory_report(stderr);
#endif

  /* keep gcc -Wall happy */
  return(0);
}
