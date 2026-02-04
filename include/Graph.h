#pragma once
#include <Eigen/Dense>
#include <vector>

class Graph {
private:
    int n;
    Eigen::MatrixXd adj;

    // Internal helper
    bool validEdge(int u, int v) const;

public:
    // Constructor
    Graph(int n);

    // Getters
    int getN() const;

    // Core Logic
    void addEdge(int u, int v);
    void printGraph() const;

    // Spectral Math
    Eigen::MatrixXd getDegree() const;
    Eigen::MatrixXd getLaplacian() const;
    double getFiedlerValue();

    // Static Generators
    static Graph createPath(int n);
    static Graph createCycle(int n);
    static Graph createConnectedGraph(int n);
    static Graph createBridgeGraph(int n);
    static Graph createRandomGraph(int n, double p);
    static Graph createHubGraph(int n, double p_hub, double p_not_hub, double p_for_hub);
};
