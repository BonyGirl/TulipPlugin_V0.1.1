/**
 *
 * This file is part of Tulip (www.tulip-software.org)
 *
 * Authors: David Auber and the Tulip development Team
 * from LaBRI, University of Bordeaux, University Corporation 
 * for Atmospheric Research
 *
 * Tulip is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3
 * of the License, or (at your option) any later version.
 *
 * Tulip is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 */

#include<fstream>
#include <algorithm>
#include "helloworld.h"
#include "fabric.h"
#include "ibautils/ib_fabric.h"
#include "ibautils/ib_parser.h"
#include "ibautils/regex.h"

using namespace tlp;
using namespace std;

/*PLUGIN(ImportInfinibandRoutes)*/
PLUGIN(HelloWorld)

static const char * paramHelp[] = {
  // File to Open
  HTML_HELP_OPEN() \
  /*HTML_HELP_DEF( "type", "pathname")*/ \
  HTML_HELP_BODY() \
  "Hello World" \
  HTML_HELP_CLOSE()
};

HelloWorld::HelloWorld(tlp::PluginContext* context)
  : tlp::Algorithm(context)
{
  addInParameter<std::string>("file::filename", paramHelp[0],"");
    
}

namespace ib = infiniband;
namespace ibp = infiniband::parser;

//Implementing min_distance
int HelloWorld::min_distance(int dist[], bool visited[], int v){
  int min = INT_MAX;
    int min_index = 0;

    for(int i = 0; i<v; i++){
        if(!visited[i] && dist[i] < min)
            min = dist[i], min_index = i;
    }

    return min_index;
}





bool HelloWorld::run()
{
  assert(graph);

  static const size_t STEPS = 6;
  if(pluginProgress)
  {
    pluginProgress->showPreview(false);
    pluginProgress->setComment("Starting to Import Routes");
    pluginProgress->progress(0, STEPS);
  }

  /**
   * while this does not import
   * nodes/edges, it imports properties
   * for an existing fabric
   */

  ib::tulip_fabric_t * const fabric = ib::tulip_fabric_t::find_fabric(graph, false);
  if(!fabric)
  {
    if(pluginProgress)
      pluginProgress->setError("Unable find fabric. Make sure to preserve data when importing data.");

    return false;
  }

  if(pluginProgress)
  {
    pluginProgress->setComment("Found Fabric");
    pluginProgress->progress(1, STEPS);
  }

  /**
   * Open file to read and import per type
   */
  std::string filename;
  
  dataSet->get("file::filename", filename);
  std::ifstream ifs(filename.c_str());
  if(!ifs)
  {
    if(pluginProgress)
      pluginProgress->setError("Unable open source file.");

    return false;
  }

  if(pluginProgress)
  {
    pluginProgress->progress(2, STEPS);
    pluginProgress->setComment("Parsing Routes.");
  }

  if(pluginProgress)
  {
    pluginProgress->setComment("Parsing Routes complete.");
    pluginProgress->progress(3, STEPS);
  }

  ifs.close();
      
  /**
   * calculate routes outbound
   * from every port on the fabric
   */
  tlp::IntegerProperty * ibRoutesOutbound = graph->getProperty<tlp::IntegerProperty >("ibRoutesOutbound");
  assert(ibRoutesOutbound);

  if(pluginProgress)
  {
    pluginProgress->setComment("Calculating Route oversubscription.");
    pluginProgress->progress(4, STEPS);
  }

  tlp::Iterator<node> *itnod = graph->getNodes();
  int v = 0; 
  
  
  while( itnod->hasNext()){
    itnod->next();
    v++;
  }
  
  //initialize matrix
  int **adjacent_matrix;
  adjacent_matrix = new int* [v];
  for(int i = 0; i<v; i++){
    adjacent_matrix[i] = new int[v];
  }
  
  for(int i = 0; i<v; i++){
    for(int j = 0; j<v; j++){
      adjacent_matrix[i][j] = 0;
    }
  }
  tlp::Iterator<edge> *ite = graph->getEdges();
  while(ite->hasNext()){
    edge e = ite->next();
    int s = graph->source(e).id, t = graph->target(e).id;
    if(!adjacent_matrix[s][t]){
      adjacent_matrix[s][t]=1;
      adjacent_matrix[t][s]=1;
    }
  }
  
  cout<<"Hello World!"<<endl;

  if(pluginProgress)
  {
    pluginProgress->setComment("Calculating Route oversubscription complete.");
    pluginProgress->progress(STEPS, STEPS);
  }

  return true;
}
