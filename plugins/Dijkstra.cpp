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
 * refine by Zhenzhen Liu on 6/21/17
 */

#include <fstream>
#include <algorithm>
#include <vector>

#include <tulip/TlpTools.h>
#include <tulip/Graph.h>
#include <tulip/GlScene.h>
#include <tulip/GraphIterator.h>
#include <tulip/BooleanProperty.h>
#include <tulip/ColorProperty.h>

#include "Dijkstra.h"

#include "fabric.h"
#include "ibautils/ib_fabric.h"
#include "ibautils/ib_parser.h"
#include "ibautils/regex.h"

using namespace tlp;
using namespace std;


namespace ib = infiniband;
namespace ibp = infiniband::parser;


/*PLUGIN(ImportInfinibandRoutes)*/
PLUGIN(Dijkstra)

static const char * paramHelp[] = {
  // File to Open
  HTML_HELP_OPEN() \
  /*HTML_HELP_DEF( "type", "pathname")*/ \
  HTML_HELP_BODY() \
  "Hello World" \
  HTML_HELP_CLOSE()
};


Dijkstra::Dijkstra(tlp::PluginContext* context)
: tlp::Algorithm(context)
{
    addInParameter<std::string>("file::filename", paramHelp[0],"");

}


//Implementing min_distance
int Dijkstra::nodes_map::min_distance(map<int, Dijkstra::nodes_map::myNode*> map1, bool visited[]){
    int min = INT_MAX;
    int min_index = 0;

    for(int i = 0; i<v; i++){
        if(!visited[i] && map1[i]->getDist() < min)
            min = map1[i]->getDist(), min_index = i;
    }

    return min_index;
}


map<int,Dijkstra::nodes_map::myNode*> Dijkstra::nodes_map::dijkstra(int src) {
    map<int, Dijkstra::nodes_map::myNode*> distmap;
    bool visited[v];

    for (int i = 0; i < v; i++) {
        distmap[i] = new Dijkstra::nodes_map::myNode(i, INT_MAX);
        visited[i] = false;
    }
    distmap[src]->setDist(0);

    for (int count = 0; count < v - 1; count++) {
        int u = min_distance(distmap, visited);
        visited[u] = true;

        for (int i = 0; i < v; i++) {
            if (!visited[i] && adjacent_matrix[u][i] && distmap[u]->getDist() != INT_MAX &&
                distmap[u]->getDist() + adjacent_matrix[u][i] < distmap[i]->getDist())
            {
                distmap[i]->setDist(distmap[u]->getDist() + adjacent_matrix[u][i]);
                distmap[i]->setFrom(u);
            }
        }
    }

    return distmap;
}

vector<unsigned int> Dijkstra::nodes_map::tracePath(map<int, Dijkstra::nodes_map::myNode*> distmap, int target, int src){
    cout<<"the destination is: "<<target<<endl;
    vector<unsigned int> path;
    unsigned int pos = target;
    for(int i = 0; i<v-1; i++){
        path.push_back(pos);
        cout<<"next step is: "<<distmap[pos]->getFrom()<<" ";
        pos = distmap[pos]->getFrom();
        if(pos == (unsigned int)src){
            path.push_back(pos);
            break;
        }
    }
    cout<<" "<<endl;

    return path;
}


const tlp::node & Dijkstra::find_node(unsigned int id){
    tlp::Iterator<tlp::node> *itnodes = graph->getNodes();

    while(itnodes->hasNext()){
        const tlp::node &node = itnodes->next();
        if(node.id == id)
            return node;
    }

    itnodes = graph->getNodes();
    const tlp::node &temp = itnodes->next();
    return temp;
}


bool Dijkstra::run()
{
    assert(graph);

    static const size_t STEPS = 5;
     //PluginProcess interacts with users as the plugin runs
    if(pluginProgress)
    {
        pluginProgress->showPreview(false);
        pluginProgress->setComment("Starting to Import Routes");
        pluginProgress->progress(0, STEPS);
    }
        
    if(pluginProgress)
    {
        pluginProgress->setComment("Implementing Dijkstra's algorithm on the graph...");
        pluginProgress->progress(1, STEPS);
    }

    if(pluginProgress)
    {
        pluginProgress->progress(2, STEPS);
        pluginProgress->setComment("Finalizing the process..");
    }

    if(pluginProgress)
    {
        pluginProgress->setComment("Please wait..");
        pluginProgress->progress(3, STEPS);
    }

    if(pluginProgress)
    {
        pluginProgress->setComment("Calculating the minimum distances..");
        pluginProgress->progress(4, STEPS);
    }

    tlp::Iterator<tlp::node> *itnod = graph->getNodes();
    int v = 0;

    while(itnod->hasNext()){
        itnod->next();
        v++;
    }

    BooleanProperty *selectBool = graph->getLocalProperty<BooleanProperty>("viewSelection");

    tlp::Iterator<node> *selections = selectBool->getNodesEqualTo(true,NULL);

    int path_node[1];
    path_node[0]=0;// Default source node is 0
    int path_id = 0;

    while(selections->hasNext()){
        const node &mynode = selections->next();
      
         if(path_id>=1)
         {
           if(pluginProgress)
             pluginProgress->setError("More than one node are selected");

           return false;
         }
      
        path_node[path_id++] = mynode.id;
    }

    if(path_id == 0)
    {
      if(pluginProgress)
        pluginProgress->setError("No node is selected");

      return false;
    }

    nodes_map *graphAnalysis = new nodes_map(graph,v);
    //test first and then modify to select source by user
    map<int, Dijkstra::nodes_map::myNode*> mymap = graphAnalysis->dijkstra(path_node[0]);
    
    int max = 1;
  
    //Print Distance and find out the max and min numbers
    for(int i = 0; i<v; i++){
        max = std::max(max,mymap[i]->getDist());
        cout<<"Node id: "<<i<<" ---- The shortest distance: "<<mymap[i]->getDist()<<endl;
    }

    cout<<"*************************************************************************"<<endl;
    cout<<""<<endl;
    std::vector<unsigned int> mypath;


    tlp::IntegerProperty * ibHop = graph->getProperty<tlp::IntegerProperty>("ibHop");
    assert(ibHop);

    if(pluginProgress)
    {
        pluginProgress->setComment("Show the max min average steps");
        pluginProgress->progress(5, STEPS);
    }

    tlp::Iterator<tlp::node> *itnodes = graph->getNodes();
    while(itnodes->hasNext()){
        const tlp::node &node = itnodes->next();
        const int &temp = mymap[node.id]->getDist();
        ibHop->setNodeValue(node, temp);
    }

    if(pluginProgress)
    {
        pluginProgress->setComment("Calculating Route oversubscription complete.");
        pluginProgress->progress(STEPS, STEPS);
    }

    return true;
}
