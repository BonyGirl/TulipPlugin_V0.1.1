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

class RouteAnalysis: public tlp::Algorithm{
    PLUGININFORMATION("Real Rohte",
                      "zz",
                      "06/27/2017",
                      "print out the real routing path",
                      "alphe",
                      "Infiniband");
public:
    RouteAnalysis(tlp::PluginContext* context);

    bool run();
};
#endif //TULIPTEST_ROUTEANALYSIS_H
