//
// Created by zliu58 on 6/27/17.
//

#include<fstream>
#include <algorithm>
#include <set>
#include <string>
#include "RouteAnalysis.h"

#include <tulip/GlScene.h>
#include <tulip/BooleanProperty.h>
#include <tulip/StringProperty.h>
#include "fabric.h"
#include "ibautils/ib_fabric.h"
#include "ibautils/ib_parser.h"
#include "ibautils/ib_port.h"
#include "ibautils/regex.h"

using namespace std;
using namespace tlp;

PLUGIN(RouteAnalysis)

static const char * paramHelp[]={

HTML_HELP_OPEN() \
HTML_HELP_DEF("type","String")\
HTML_HELP_DEF("values","print the routing path")\
HTML_HELP_DEF("default","print the routing path")\
HTML_HELP_BODY() \
HTML_HELP_CLOSE()

};

static const string IMPORT_TYPE_STRING=  "print the routing path";

RouteAnalysis::RouteAnalysis(tlp::PluginContext *context)
: tlp::Algorithm(context){
    addInParameter<tlp::StringCollection>("Test",paramHelp[0],IMPORT_TYPE_STRING);
}

namespace ib = infiniband;
namespace ibp = infiniband::parser;

bool RouteAnalysis::run(){
    assert(graph);

    static const size_t STEPS=3;
    if(pluginProgress){
        pluginProgress->showPreview(false);
        pluginProgress->setComment("Starting to Import Routes");
        pluginProgress->progress(0,STEPS);
    }

    ib::tulip_fabric_t * const fabric = ib::tulip_fabric_t::find_fabric(graph, false);

    if(!fabric)
    {
        if(pluginProgress)
            pluginProgress->setError("Unable find fabric. Make sure to preserve data when importing data.");

        return false;
    }

    if(pluginProgress){
        pluginProgress->setComment("Found fabric");
        pluginProgress->progress(1,STEPS);
    }

    //find the source and target nodes of the path
    BooleanProperty *selectBool = graph->getLocalProperty<BooleanProperty>("viewSelection");
    StringProperty *getGuid = graph->getLocalProperty<StringProperty>("ibGuid");
    vector<tlp::node> nodes_guid;

    tlp::Iterator<tlp::node> *selections = selectBool->getNodesEqualTo(true,NULL);
    const ib::fabric_t::entities_t &entities_map = fabric->get_entities();
    
    while(selections->hasNext()){
        const tlp::node &mynode = selections->next();
        nodes_guid.push_back(mynode);
    }
        
    //const unsigned long long int key1 = std::stol((getGuid->getNodeStringValue(nodes_guid[0])).c_str(),NULL,0);
    //const unsigned long long int key2 = std::stol((getGuid->getNodeStringValue(nodes_guid[1])).c_str(),NULL,0);

    ib::lid_t target_lid = target_node.lid();
    std::vector<ib::entity_t> tmp;
    tmp.push_back(const_cast<ib::entity_t &> (source_node));

    while(tmp.back().lid()!= target_lid) {
        const ib::entity_t & temp = tmp.back();
        cout<<temp.lid()<<endl;
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
                        tmp.push_back(const_cast<ib::entity_t &> (node));
                    }
                }
            }
        }
    }
    
    if (pluginProgress) {
        pluginProgress->setComment("Found path source and target");
        pluginProgress->progress(2, STEPS);
    }
    
    if(pluginProgress)
    {
        pluginProgress->setComment("Print the Real hops");
        pluginProgress->progress(STEPS, STEPS);
    }

    return true;
}
