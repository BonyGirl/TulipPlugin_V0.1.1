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
 *
 **/

#include<fstream>
#include <algorithm>
#include <string>
#include <set>
#include "RouteAnalysis_All.h"

#include <tulip/GlScene.h>
#include <tulip/BooleanProperty.h>
#include <tulip/StringProperty.h>
#include <tulip/IntegerProperty.h>
#include <tulip/ColorProperty.h>
#include "fabric.h"
#include "ibautils/ib_fabric.h"
#include "ibautils/ib_parser.h"
#include "ibautils/ib_port.h"
#include "ibautils/regex.h"

using namespace std;
using namespace tlp;

PLUGIN(RouteAnalysis_All)

static const char * paramHelp[] = {
  // File to Open
  HTML_HELP_OPEN() \
  HTML_HELP_DEF( "type", "pathname" ) \
  HTML_HELP_BODY() \
  "Path to ibdiagnet2.fdbs file to import" \
  HTML_HELP_CLOSE()
};

RouteAnalysis_All::RouteAnalysis_All(tlp::PluginContext* context)
        : tlp::Algorithm(context)
{
    addInParameter<std::string>("file::filename", paramHelp[0],"");
}


namespace ib = infiniband;
namespace ibp = infiniband::parser;

const ib::entity_t * RouteAnalysis_All::getMyEntity(const tlp::node node,ib::tulip_fabric_t * const fabric){
    for(ib::tulip_fabric_t::entity_nodes_t::iterator it1 = fabric->entity_nodes.begin(); it1 != fabric->entity_nodes.end(); ++it1)
        if(it1->second.id == node.id)
            return it1->first;

    return nullptr;
}

int RouteAnalysis_All::help_count(ib::tulip_fabric_t * const fabric, tlp::Graph * const graph,
                        std::vector<ib::entity_t *> &tmp, const ib::entity_t * real_target)
{
    int count = 0;
    bool find_next = false;
    ib::lid_t target_lid = real_target->lid();
  
    while(tmp.back()->guid!= real_target->guid) {
        const ib::entity_t & temp = *tmp.back();
      
        for (
                ib::entity_t::routes_t::const_iterator
                        ritr = temp.get_routes().begin(),
                        reitr = temp.get_routes().end();
                ritr != reitr;
                ++ritr
                ) {
            std::set<ib::lid_t>::const_iterator itr = ritr->second.find(target_lid);
            if (itr != ritr->second.end()) {
                const ib::entity_t::portmap_t::const_iterator port_itr = temp.ports.find(ritr->first);
                if (port_itr != temp.ports.end()) {
                    const ib::port_t *const port = port_itr->second;
                    const ib::tulip_fabric_t::port_edges_t::const_iterator edge_itr = fabric->port_edges.find(
                            const_cast<ib::port_t *>(port));
                    if (edge_itr != fabric->port_edges.end()) {
                        const tlp::edge &edge = edge_itr->second;
                        const ib::entity_t * entity = getMyEntity(graph->target(edge),fabric);
                        tmp.push_back(const_cast<ib::entity_t *> (entity));
                        count++;
                        find_next = true;
                    }
                }
            }
        }
      
        if(!find_next)
        return -1;
    }


    return count;
}


int RouteAnalysis_All::count_hops(const tlp::node source_node, const tlp::node target_node,tlp::Graph * const graph){
    //Get the fabric from the graph
    ib::tulip_fabric_t * const fabric = ib::tulip_fabric_t::find_fabric(graph, false);

    //Set the tulip Properties
    IntegerProperty *getPortNum = graph->getLocalProperty<IntegerProperty>("ibPortNum");

    //Get the entities' map <guid, entity_t> = entities_t
    //const ib::fabric_t::entities_t &entities_map = fabric->get_entities();

    const ib::entity_t * source_entity = getMyEntity(source_node,fabric);
    const ib::entity_t * target_entity = getMyEntity(target_node,fabric);
    
    cout<<"Test source_entity guid: "<<source_entity->guid<<endl;
    cout<<"Test target_entity guid: "<<target_entity->guid<<endl;
    cout<<"----------Test the source & target entities end------------"<<endl;
    
    //ib::lid_t target_lid = target_entity->lid();
    std::vector<ib::entity_t *> tmp;

    int count = 0;
    if(getPortNum->getNodeValue(source_node)==1){
        //find the only port in HCA
        const ib::entity_t::portmap_t::const_iterator Myport = source_entity->ports.begin();

        //use the typedef std::map<port_t*, tlp::edge> port_edges_t to find the edge
        ib::tulip_fabric_t::port_edges_t::iterator Myedge = fabric->port_edges.find(Myport->second);
        if(graph->source(Myedge->second).id == source_node.id){
            const tlp::edge &e = Myedge->second;
            tmp.push_back(const_cast<ib::entity_t *> (getMyEntity(graph->target(e),fabric)));
        }else{
            const tlp::edge &e = Myedge->second;
            tmp.push_back(const_cast<ib::entity_t *> (getMyEntity(graph->source(e),fabric)));
        }
        count++;
    }else{
        tmp.push_back(const_cast<ib::entity_t *> (source_entity));
    }

    //Whether the target is HCE or not
    if(getPortNum->getNodeValue(target_node)== 1){
        //find the only port in HCA
        const ib::entity_t::portmap_t::const_iterator Myport = target_entity->ports.begin();

        //use the typedef std::map<port_t*, tlp::edge> port_edges_t to find the edge
        ib::tulip_fabric_t::port_edges_t::iterator Myedge = fabric->port_edges.find(Myport->second);
        if(graph->source(Myedge->second).id == source_node.id){
            const tlp::edge &e = Myedge->second;
            const ib::entity_t * real_target = getMyEntity(graph->target(e),fabric);
                    
            int count_temp = help_count(fabric, graph, tmp, real_target);
            if(count_temp == -1)
              return -1;
            else
              count += count_temp;


        }else {
            //find the only port in HCA
            const ib::entity_t::portmap_t::const_iterator Myport = target_entity->ports.begin();

            //use the typedef std::map<port_t*, tlp::edge> port_edges_t to find the edge
            ib::tulip_fabric_t::port_edges_t::iterator Myedge = fabric->port_edges.find(Myport->second);
            const tlp::edge &e = Myedge->second;
            const ib::entity_t * real_target = getMyEntity(graph->source(e),fabric);
            int count_temp = help_count(fabric, graph, tmp, real_target);
            if(count_temp == -1)
              return -1;
            else
              count += count_temp;
        }
            count++;
    }
    else{
        int count_temp = help_count(fabric, graph, tmp, target_entity);
        if(count_temp == -1)
          return -1;
        else
          count += count_temp;

    }
    return count;
}


bool RouteAnalysis_All::run(){
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
    if(!parser.parse(*fabric, ifs))
    {
        if(pluginProgress)
            pluginProgress->setError("Unable parse routes file.");

        return false;
    }

    if(pluginProgress)
    {
        pluginProgress->setComment("Parsing Routes complete.");
        pluginProgress->progress(3, STEPS);
    }

    ifs.close();


    if (pluginProgress) {
        pluginProgress->setComment("Found path source and target");
        pluginProgress->progress(4, STEPS);
    }

    BooleanProperty *selectSource = graph->getLocalProperty<BooleanProperty>("viewSelection");
    tlp::IntegerProperty * ibRealHop = graph->getProperty<tlp::IntegerProperty>("ibRealHop");
    assert(ibRealHop);

    //const ib::fabric_t::entities_t &entities_map = fabric->get_entities();
    tlp::Iterator<tlp::node> *other = graph->getNodes();

    tlp::Iterator<tlp::node> *selections = selectSource->getNodesEqualTo(true,NULL);
    const tlp::node mySource = selections->next();

    cout<<"My Source ID: "<<mySource.id<<endl;
    cout<<"-------------------mySource test end-----------------------"<<endl;

    while(other->hasNext()){
        const tlp::node &node = other->next();
        if(node.id == mySource.id){
            ibRealHop->setNodeValue(node, 0);
            continue;
        }
        else
        {
            const int &temp = count_hops(mySource,node,graph);
            ibRealHop->setNodeValue(node, temp);
        }
    }


    if(pluginProgress)
    {
        pluginProgress->setComment("Print the Real hops");
        pluginProgress->progress(STEPS, STEPS);
    }

    return true;
}

