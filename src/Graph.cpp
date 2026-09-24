#include "Graph.h"

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace Eigen;
using namespace std;

bool Graph::validEdge(int u, int v) const {
    return (u >= 0) && (v >= 0) && (u <= n - 1) && (v <= n - 1) && (u != v);
}

void Graph::checkNode(int u) const {
    if (u < 0 || u >= n) {
        throw out_of_range("Node " + to_string(u) + " is out of range for a graph with " +
                           to_string(n) + " nodes.");
    }
}

Graph::Graph(int numNodes) : n(numNodes) {
    if (numNodes < 0) {
        throw invalid_argument("Graph size must be non-negative, got " + to_string(numNodes) + ".");
    }
    adj = MatrixXd::Zero(numNodes, numNodes);
}

int Graph::getN() const {
    return n;
}

bool Graph::hasEdge(int u, int v) const {
    checkNode(u);
    checkNode(v);
    return adj(u, v) != 0.0;
}

int Graph::degree(int u) const {
    checkNode(u);
    return static_cast<int>(adj.row(u).sum());
}

int Graph::edgeCount() const {
    return static_cast<int>(adj.sum() / 2.0);
}

void Graph::addEdge(int u, int v) {
    if (!validEdge(u, v)) {
        throw invalid_argument("Invalid edge (" + to_string(u) + ", " + to_string(v) +
                               "): nodes must be distinct and in [0, " + to_string(n - 1) + "].");
    }
    adj(u, v) = 1;
    adj(v, u) = 1;
}

void Graph::printGraph() const {
    cout << "--- Adjacency Matrix ---" << endl;
    cout << adj << endl;
}

Eigen::MatrixXd Graph::getDegree() const {
    VectorXd degrees = adj.rowwise().sum();
    return degrees.asDiagonal();
}

Eigen::MatrixXd Graph::getLaplacian() const {
    return getDegree() - adj;
}

double Graph::getFiedlerValue() const {
    if (n < 2) {
        return 0.0;
    }

    SelfAdjointEigenSolver<MatrixXd> es(getLaplacian(), EigenvaluesOnly);
    if (es.info() != Success) {
        throw runtime_error("Eigen-decomposition of the Laplacian failed.");
    }

    return es.eigenvalues()(1);
}

bool Graph::isConnected(double tol) const {
    if (n < 2) {
        return true;
    }
    return getFiedlerValue() > tol;
}

// Graph Creators

Graph Graph::createPath(int n) {
    Graph graph(n);
    for (int i = 0; i < n - 1; i++) {
        graph.addEdge(i, i + 1);
    }
    return graph;
}

Graph Graph::createCycle(int n) {
    Graph graph = createPath(n);
    // For n < 3 the closing edge is a self-loop or duplicates the path edge.
    if (n >= 3) {
        graph.addEdge(0, n - 1);
    }
    return graph;
}

Graph Graph::createConnectedGraph(int n) {
    Graph graph(n);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < i; j++) {
            graph.addEdge(i, j);
        }
    }
    return graph;
}

Graph Graph::createBridgeGraph(int n) {
    Graph graph(n);
    int half_point = n / 2;

    for (int i = 0; i < half_point; i++) {
        for (int j = 0; j < i; j++) {
            graph.addEdge(i, j);
        }
    }

    // With fewer than two nodes there is nothing to bridge.
    if (half_point >= 1) {
        graph.addEdge(half_point - 1, half_point);
    }

    for (int i = half_point; i < n; i++) {
        for (int j = half_point; j < i; j++) {
            graph.addEdge(i, j);
        }
    }

    return graph;
}

Graph Graph::createRandomGraph(int n, double p, std::uint32_t seed) {
    Graph graph(n);

    std::mt19937 gen(seed);
    std::uniform_real_distribution<> dis(0.0, 1.0);

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < i; j++) {
            if (dis(gen) <= p) {
                graph.addEdge(i, j);
            }
        }
    }

    return graph;
}

Graph Graph::createHubGraph(int n, double p_hub, double p_not_hub, double p_for_hub,
                            std::uint32_t seed) {
    Graph graph(n);

    std::mt19937 gen(seed);
    std::uniform_real_distribution<> dis(0.0, 1.0);

    std::vector<bool> isHub(n);
    for (int i = 0; i < n; i++) {
        isHub[i] = dis(gen) <= p_hub;
    }

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < i; j++) {
            double p_in_use = (isHub[i] || isHub[j]) ? p_for_hub : p_not_hub;
            if (dis(gen) <= p_in_use) {
                graph.addEdge(i, j);
            }
        }
    }

    return graph;
}
