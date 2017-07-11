//
// Created by zliu58 on 6/27/17.
//
#include <tulip/TulipPluginHeaders.h>
#include <tulip/TlpTools.h>
#include <tulip/Graph.h>
#include "fabric.h"
#include <map>

#ifndef TULIPTEST_ROUTEANALYSIS_H
#define TULIPTEST_ROUTEANALYSIS_H

namespace ib = infiniband;

class RouteAnalysis: public tlp::Algorithm{
    PLUGININFORMATION("Real Routes",
                      "zz",
                      "06/27/2017",
                      "print out the real routing path",
                      "alphe",
                      "Infiniband");
public:
    RouteAnalysis(tlp::PluginContext* context);

    bool run();
    
    const ib::entity_t * getMyEntity(const tlp::node node,ib::tulip_fabric_t * const fabric);
};
#endif //TULIPTEST_ROUTEANALYSIS_H
