//
// Created by zliu58 on 6/21/17.
//
#include <tulip/TulipPluginHeaders.h>
#include <tulip/TlpTools.h>
#include <tulip/Graph.h>
#include <map>
#include <vector>

#ifndef TULIPTEST_MAIN_H
#define TULIPTEST_MAIN_H

using namespace tlp;
using namespace std;

class InfinibandAnalysis: public tlp::Algorithm{
public:
    PLUGININFORMATION("InfinibandAnalysis",
    "zzl",
    "06/21/17",
    "Import IB and change some of its porperties",
    "alpha",
    "Infiniband")

    InfinibandAnalysis(tlp::PluginContext* context);

    class nodes_map{
    private:
        int v; //number of node;
        int **adjacent_matrix;
    public:
        class myNode{
           int from;
           int dist;
        public:
            myNode(int from, int dist = INT_MAX){
                this->from = from;
                this->dist = dist;
            };

            void setDist(int d){dist = d; }
            int getDist(){return dist;}

            void setFrom(int f){from = f;}
            int getFrom(){return from;}

        };

    nodes_map(const Graph *graph, int v){
        initMap(graph, v);
    }
    ~nodes_map(){
        delete adjacent_matrix[v];
    }

    void initMap(const Graph *graph, int v){
        this->v = v;
        adjacent_matrix = new int* [v];
        for(int i = 0; i<v; i++){
            adjacent_matrix[i] = new int[v];
        }

        //Initialize the adjacent_matrix
        for(int i = 0; i<v; i++){
            for(int j = 0; j<v; j++){
                adjacent_matrix[i][j] = 0;
            }
        }

        //Set up the adjacent_matrix
        tlp::Iterator<edge> *ite = graph->getEdges();
        while(ite->hasNext()){
            edge e = ite->next();
            int s = graph->source(e).id, t = graph->target(e).id;
            if(!adjacent_matrix[s][t]){
                adjacent_matrix[s][t] = 1;
                adjacent_matrix[t][s] = 1;
            }
        }
    }

    int min_distance(map<int, myNode*> map1, bool visited[]);
    map<int,myNode*> dijkstra(int src);
    void printResult(map<int, myNode*> map1);
    vector<unsigned int> tracePath(map<int, myNode*> distmap, int target,int src);
    };

    bool run();
    const tlp::node & find_node(int id);
};
#endif //TULIPTEST_MAIN_H
