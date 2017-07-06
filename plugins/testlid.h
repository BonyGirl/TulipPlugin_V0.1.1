//
// Created by zliu58 on 7/6/17.
//
#include <tulip/TulipPluginHeaders.h>
#include <tulip/TlpTools.h>
#include <tulip/Graph.h>
#include "fabric.h"
#include <map>

#ifndef TULIPTEST_ROUTEANALYSIS_H
#define TULIPTEST_ROUTEANALYSIS_H


class TestLid: public tlp::Algorithm{
    PLUGININFORMATION("Test Unreachable",
                      "zz",
                      "06/27/2017",
                      "print out the real routing path",
                      "alphe",
                      "Infiniband");
public:
    TestLid(tlp::PluginContext* context);

    bool run();

};
#endif //TULIPTEST_ROUTEANALYSIS_H
