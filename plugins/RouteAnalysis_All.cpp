//
// Created by zliu58 on 7/5/17.
//

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

unsigned int RouteAnalysis_All::count_myhops(const ib::entity_t * source_entity, const ib::entity_t * target_entity,tlp::Graph * const graph){
    ib::tulip_fabric_t * const fabric = ib::tulip_fabric_t::find_fabric(graph, false);

    IntegerProperty *getPortNum = graph->getLocalProperty<IntegerProperty>("ibPortNum");
    StringProperty *getGuid = graph->getLocalProperty<StringProperty>("ibGuid");

    const ib::fabric_t::entities_t &entities_map = fabric->get_entities();
    ib::tulip_fabric_t::entity_nodes_t::iterator source = fabric->entity_nodes.find(const_cast<ib::entity_t *>(source_entity));
    const tlp::node source_node = source->second;

    ib::tulip_fabric_t::entity_nodes_t::iterator target = fabric->entity_nodes.find(const_cast<ib::entity_t *>(target_entity));
    const tlp::node target_node = target->second;

    ib::lid_t target_lid = target_entity->lid();
    std::vector<ib::entity_t *> tmp;

    unsigned int count = 0;
    if(getPortNum->getNodeValue(source_node)==1){
        //find the only port in HCA
        const ib::entity_t::portmap_t::const_iterator Myport = source_entity->ports.begin();

        //use the typedef std::map<port_t*, tlp::edge> port_edges_t to find the edge
        ib::tulip_fabric_t::port_edges_t::iterator Myedge = fabric->port_edges.find(Myport->second);
        if(graph->source(Myedge->second).id == source_node.id){
            const tlp::edge &e = Myedge->second;
            tmp.push_back(const_cast<ib::entity_t *> ( & entities_map.find(std::stol(getGuid->getNodeStringValue(graph->target(e)).c_str(),NULL,0))->second));
        }else{
            tmp.push_back(const_cast<ib::entity_t *> (source_entity));
        }
        count++;
    }

    //Whether the target is HCE or not
    if(getPortNum->getNodeValue(target_node)== 1){
        //find the only port in HCA
        const ib::entity_t::portmap_t::const_iterator Myport = target_entity->ports.begin();

        //use the typedef std::map<port_t*, tlp::edge> port_edges_t to find the edge
        ib::tulip_fabric_t::port_edges_t::iterator Myedge = fabric->port_edges.find(Myport->second);
        if(graph->source(Myedge->second).id == source_node.id){
            const tlp::edge &e = Myedge->second;
            const ib::entity_t & real_target = entities_map.find(std::stol((getGuid->getNodeStringValue(graph->target(e))).c_str(),NULL,0))->second;
            while(tmp.back()->guid!= real_target.guid) {
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
                                const ib::entity_t &node = entities_map.find(std::stol((getGuid->getNodeStringValue(graph->target(edge))).c_str(), NULL, 0))->second;
                                tmp.push_back(const_cast<ib::entity_t *> (&node));
                                count++;
                            }
                        }
                    }
                }
            }


        }else{
            //find the only port in HCA
            const ib::entity_t::portmap_t::const_iterator Myport = target_entity->ports.begin();

            //use the typedef std::map<port_t*, tlp::edge> port_edges_t to find the edge
            ib::tulip_fabric_t::port_edges_t::iterator Myedge = fabric->port_edges.find(Myport->second);
            const tlp::edge &e = Myedge->second;
            const ib::entity_t & real_target = entities_map.find(std::stol((getGuid->getNodeStringValue(graph->source(e))).c_str(),NULL,0))->second;
            while(tmp.back()->guid!= real_target.guid) {
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
                                const ib::entity_t &node = entities_map.find(std::stol((getGuid->getNodeStringValue(graph->target(edge))).c_str(), NULL, 0))->second;
                                tmp.push_back(const_cast<ib::entity_t *> (&node));
                                count++;
                            }
                        }
                    }
                }
            }

        }
        count++;
    }
    else{

        while(tmp.back()->guid!= target_entity->guid) {
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
                            const ib::entity_t &node = entities_map.find(std::stol((getGuid->getNodeStringValue(graph->target(edge))).c_str(), NULL, 0))->second;
                            tmp.push_back(const_cast<ib::entity_t *> (&node));
                            count++;
                        }
                    }
                }
            }
        }
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
    StringProperty *getGuid = graph->getLocalProperty<StringProperty>("ibGuid");

    const ib::fabric_t::entities_t &entities_map = fabric->get_entities();

    tlp::Iterator<tlp::node> *selections = selectSource->getNodesEqualTo(true,NULL);
    const tlp::node mySource = selections->next();
    const ib::entity_t & source_entity = entities_map.find(std::stol((getGuid->getNodeStringValue(mySource)).c_str(),NULL,0))->second;
    
    //const ib::entity_t & target_entity = entities_map.find(std::stol((getGuid->getNodeStringValue(other->next())).c_str(),NULL,0))->second;
    //const unsigned int &temp = count_myhops(& source_entity, & target_entity,graph);
    //cout<<temp<<endl;
        
    tlp::IntegerProperty * ibRealHop = graph->getProperty<tlp::IntegerProperty>("ibRealHop");
    assert(ibRealHop);
    
    tlp::Iterator<tlp::node> *other = graph->getNodes();
    if(pluginProgress)
    {
        pluginProgress->setComment("Show the max min average steps");
        pluginProgress->progress(5, STEPS);
    }
        
    while(other->hasNext()){
        const tlp::node &node = other->next();
        if(node.id == mySource.id)
        {    
            ibRealHop->setNodeValue(node, 0);
            cout<<"This is the source"<<endl;
            continue;
        }
        else
        {
            const ib::entity_t & target_entity = entities_map.find(std::stol((getGuid->getNodeStringValue(node)).c_str(),NULL,0))->second;
            const int &temp = count_myhops(& source_entity, & target_entity, graph);
            cout<<"The hops from source to node"<<node.id<<"is: "<<temp<<endl;
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

