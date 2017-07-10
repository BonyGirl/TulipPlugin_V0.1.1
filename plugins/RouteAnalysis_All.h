//
// Created by zliu58 on 7/5/17.
//

#include <tulip/TulipPluginHeaders.h>
#include <tulip/StringProperty.h>
#include <tulip/TlpTools.h>
#include <tulip/Graph.h>
#include "fabric.h"
#include <map>

#ifndef TULIPTEST_ROUTEANALYSIS_H
#define TULIPTEST_ROUTEANALYSIS_H

namespace ib = infiniband;

class RouteAnalysis_All: public tlp::Algorithm{
    PLUGININFORMATION("Real Rohte",
                      "zz",
                      "06/27/2017",
                      "print out the real routing path",
                      "alphe",
                      "Infiniband");
public:
    RouteAnalysis_All(tlp::PluginContext* context);

    bool run();
    unsigned int count_hops(const ib::entity_t * source_entity, const ib::entity_t * target_entity,tlp::Graph * const graph);
    unsigned int help_count(ib::tulip_fabric_t * const fabric, tlp::Graph * const graph,
                            std::vector<ib::entity_t *> &tmp, const ib::entity_t * real_target,
                            ib::lid_t target_lid, tlp::StringProperty *getGuid);
};
#endif //TULIPTEST_ROUTEANALYSIS_H
