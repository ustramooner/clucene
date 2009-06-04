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
#include "redlandpp/Storage.hpp"
#include "redlandpp/Model.hpp"
#include "redlandpp/Parser.hpp"

using namespace std;

using namespace Redland;


int main(int argc, char * argv[])
{
  if ( argc < 2 ){
      fprintf(stderr, "Usage: %s query\n", argv[0]);
      //return -1;
  }

  World world;
  librdf_storage_clucene_initialise(world.cobj());

  cout << "Initialised " << world << endl;

  //CLuceneStorage storage(world, "", "dir='/home/ben/dev/svn_strigi/testindex'");
  MemoryStorage storage(world);

  Model model(world, storage);

  cout << "Redland is " + world.shortCopyrightString() << endl;


  Uri uri(&world, "file:///home/ben/dev/clucene-rdf/foaf.rdf");

  cout << "URI is " << uri << endl;

#if 0
  std::vector<RaptorParserDescription> v=r.getParserDescriptions();
  for (unsigned int i = 0; i < v.size(); i++ ) {
    cout << "Parser " << i << endl << v[i] << endl;
  }

  if (r.isParserName("rdfxml")) {
    cout << "rdfxml IS a parser name\n";
  }

  if (!r.isParserName("foobar")) {
    cout << "foobar IS NOT a parser name\n";
  }
#endif

  Parser parser(&world, string("rdfxml"));

  cout << "Parser is " << parser << endl;

#if 0
  try {
    Stream* s = parser.parseUri(&uri, NULL);
    while(true) {
      Statement* st=s->get();
      if(st == NULL)
        break;
      cout << "Triple " << st << endl;
      s->next();
      delete st;
    }
    delete s;
  }
  catch (Exception &e) {
    cerr << "parseUri(" << uri << ") failed with exception " << e.what() << endl;
  }
#endif

  try {
    Stream* s=parser.parseUri(&uri, NULL);
    model.add(s);
    delete s;
  }
  catch (Exception &e) {
    cerr << "parseUri(" << uri << ") failed with exception " << e.what() << endl;
  }

  cout << "Model has " << model.size() << " triples" << endl;

  const unsigned char* query_string = (const unsigned char*) "select ?nick, ?name where "
      "(?x rdf:type foaf:Person) (?x foaf:nick ?nick) (?x foaf:name ?name) (?x foaf:name \"Eric Miller\")"
      "using foaf for <http://xmlns.com/foaf/0.1/>";
  //const unsigned char* query_string = (const unsigned char*)argv[1];
  //const unsigned char* query_string = (const unsigned char*)"PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> PREFIX iemsr: <http://www.ukoln.ac.uk/projects/iemsr/terms/> PREFIX rdfs: <http://www.w3.org/2000/01/rdf-schema#> SELECT $number $name $description WHERE {   $r rdf:type iemsr:RootDataElement .   $n iemsr:isChildOf $r .   $n iemsr:refNumber $number .   $n rdfs:label $name .   $n rdfs:comment $description }";
  //const unsigned char* query_string = (const unsigned char*)"PREFIX : <http://www.commonobjects.example.org/gmlrss> PREFIX bio: <http://purl.org/vocab/bio/0.1/> PREFIX foaf: <http://xmlns.com/foaf/0.1/>  SELECT ?name ?birthDate ?deathDate WHERE { ?bridge a :Bridge; foaf:maker ?person [ foaf:name ?name; bio:event [ a bio:Birth; bio:date ?birthDate ]; bio:event [ a bio:Death; bio:date ?deathDate ] ] }";
  //const unsigned char* query_string = (const unsigned char*)"PREFIX fd: <http://freedesktop.org/standards/xesam/1.0/core#> SELECT ?url WHERE { ?x fd:fileExtension \"cpp\" . ?x fd:url ?url }";
  librdf_query* query = librdf_new_query(world.cobj(), "rdql", NULL, query_string, NULL);

  librdf_query_results* results=librdf_model_query_execute(model.cobj(), query);
  if(!results) {
    fprintf(stderr, "Query of model with '%s' failed\n", query_string);
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

  fprintf(stdout, "Query returned %d results\n", librdf_query_results_get_count(results));

  librdf_free_query_results(results);
  librdf_free_query(query);



  return 0;
}
