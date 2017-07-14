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

#include <fstream>
#include <algorithm>
#include <vector>

#include <tulip/TlpTools.h>
#include <tulip/Graph.h>
#include <tulip/GlScene.h>
#include <tulip/GraphIterator.h>
#include <tulip/BooleanProperty.h>
#include <tulip/ColorProperty.h>

#include "InfinibandAnalysis.h"

#include "fabric.h"
#include "ibautils/ib_fabric.h"
#include "ibautils/ib_parser.h"
#include "ibautils/regex.h"

using namespace tlp;
using namespace std;

/*PLUGIN(ImportInfinibandRoutes)*/
PLUGIN(InfinibandAnalysis)

static const char * paramHelp[] = {
   // File to Open
   HTML_HELP_OPEN() \
  /*HTML_HELP_DEF( "type", "pathname")*/ \
  HTML_HELP_BODY() \
  "Hello World" \
  HTML_HELP_CLOSE()
};

InfinibandAnalysis::InfinibandAnalysis(tlp::PluginContext* context)
        : tlp::Algorithm(context)
{
    addInParameter<std::string>("file::filename", paramHelp[0],"");

}

namespace ib = infiniband;
namespace ibp = infiniband::parser;

//Implementing min_distance
int InfinibandAnalysis::nodes_map::min_distance(map<int, InfinibandAnalysis::nodes_map::myNode*> map1, bool visited[]){
    int min = INT_MAX;
    int min_index = 0;

    for(int i = 0; i<v; i++){
        if(!visited[i] && map1[i]->getDist() < min)
            min = map1[i]->getDist(), min_index = i;
    }

    return min_index;
}

void InfinibandAnalysis::nodes_map::printResult(map<int, InfinibandAnalysis::nodes_map::myNode*> map1) {
    for(int i = 0; i<v; i++){
        cout<<i<<" is from: "<<map1[i]->getFrom()<<" its distance is: "<<map1[i]->getDist()<<endl;
    }
}

map<int,InfinibandAnalysis::nodes_map::myNode*> InfinibandAnalysis::nodes_map::dijkstra(int src) {
    map<int, InfinibandAnalysis::nodes_map::myNode*> distmap;
    bool visited[v];

    for (int i = 0; i < v; i++) {
        distmap[i] = new InfinibandAnalysis::nodes_map::myNode(i, INT_MAX);
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
    //printResult(distmap);
    //cout<<"***********************************"<<endl;
    return distmap;
}

vector<unsigned int> InfinibandAnalysis::nodes_map::tracePath(map<int, InfinibandAnalysis::nodes_map::myNode*> distmap, int target, int src){
    cout<<"the destination is: "<<target<<endl;
    vector<unsigned int> path;
    int pos = target;
    for(int i = 0; i<v-1; i++){
       path.push_back(pos);
       cout<<"next step is: "<<distmap[pos]->getFrom()<<" ";
       pos = distmap[pos]->getFrom();
       if(pos == src){
         path.push_back(pos);
         break;
       }
    }
    cout<<" "<<endl;
   
   return path;
}


const tlp::node & InfinibandAnalysis::find_node(unsigned int id){
     tlp::Iterator<tlp::node> *itnodes = graph->getNodes();
       
       while(itnodes->hasNext()){
          const tlp::node &node = itnodes->next();
             if(node.id == id)
                return node;  
       }
   
   return nullptr;
}


bool InfinibandAnalysis::run()
{
    assert(graph);

    static const size_t STEPS = 5;
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

    ibp::ibdiagnet_fwd_db parser;
    /*if(!parser.parse(*fabric, ifs))
    {
      if(pluginProgress)
        pluginProgress->setError("Unable parse routes file.");
      return false;
    }*/

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
    if(pluginProgress)
    {
        pluginProgress->setComment("Calculating Route oversubscription.");
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
    
    int path_node[2];
    path_node[1]=0;// Default source node is 0
    int path_id = 0;
    bool found_path = false;
   
    while(selections->hasNext()){
        const node &mynode = selections->next();
        path_node[path_id++] = mynode.id;
    }
   
   if(path_id >1) found_path = true;
   
    nodes_map *graphAnalysis = new nodes_map(graph,v);
    //test first and then modify to select source by user
    map<int, InfinibandAnalysis::nodes_map::myNode*> mymap = graphAnalysis->dijkstra(path_node[0]);

    int max = 1;
    int avg = 1;
    //Print Distance and find out the max and min numbers
    for(int i = 0; i<v; i++){
        max = std::max(max,mymap[i]->getDist());
        cout<<"Node id: "<<i<<" ---- The shortest distance: "<<mymap[i]->getDist()<<endl;
    }
   
    cout<<"*************************************************************************"<<endl;
    cout<<""<<endl;
    std::vector<unsigned int> mypath;
   
    avg = (1+max)/2;

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

    //show the found_path in the tulip
    if(found_path)
    {
       mypath = graphAnalysis->tracePath(mymap,path_node[1],path_node[0]);
       tlp::ColorProperty * resetColor = graph->getLocalProperty<tlp::ColorProperty>("viewColor");
       itnodes = graph->getNodes();
       
       while(itnodes->hasNext()){
          const tlp::node &node = itnodes->next();
          for(unsigned int ID : mypath){
             if(node.id == ID){
                resetColor->setNodeValue(node, Color::SpringGreen);
                selectBool->setNodeValue(node, true);
             }
          }
       }
       
       for(int i = 0; i<mypath.size()-1; i++){
          const tlp::edge &edge = graph->getEdges(find_node(mypath(i), find_node(mypath(i+1))));
          selectBool->setEdgeValue(edge, true);
       }
    }
   
    if(pluginProgress)
    {
        pluginProgress->setComment("Calculating Route oversubscription complete.");
        pluginProgress->progress(STEPS, STEPS);
    }

    return true;
}
