//
// Created by zliu58 on 6/27/17.
//

#include<fstream>
#include <algorithm>
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

    static const size_t STEPS=4;
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
    vector<const tlp::node> nodes_guid;

    tlp::Iterator<tlp::node> *selections = selectBool->getNodesEqualTo(true,NULL);
    const ib::fabric_t::entities_t &entities_mymap = fabric->get_entities();
    
    while(selections->hasNext()){
        const tlp::node &mynode = selectBool->next();
        nodes_guid.push_back(mynode);
    }
    
    const ib::entity_t & source_node = entities_mymap.at(std::strtoull(getGuid->getNodeStringValue(nodes_guid.front()),NULL,0));
    
    ib::entity_t* current = const_cast<ib::entity_t*>(source_node);
    current = &(current->forward(*fabric, *current));
    cout<<current->guid<<endl;

    if(pluginProgress)
    {
        pluginProgress->setComment("Print the Real hops");
        pluginProgress->progress(STEPS, STEPS);
    }

    return true;
}
