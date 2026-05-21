#include "Graph.h"
#include <iostream>
#include <random>

using namespace Eigen;
using namespace std;

bool Graph::validEdge(int u, int v) const { return (u <= n - 1 && 0 <= u) && (v <= n - 1 && 0 <= v) && (u != v); }

Graph::Graph(int n) : n(n)
{
    adj.resize(n, n);
    adj.setZero();
}

int Graph::getN() const { return n; }

void Graph::addEdge(int u, int v)
{
    if (!validEdge(u, v))
    {
        cerr << "Warning: Invalid edge ignored." << endl;
        return;
    }
    adj(u, v) = 1;
    adj(v, u) = 1;
}

void Graph::printGraph() const
{
    cout << "--- Adjacency Matrix ---" << endl;
    cout << adj << endl;
}

Eigen::MatrixXd Graph::getDegree() const
{
    VectorXd degrees = adj.rowwise().sum();

    return degrees.asDiagonal();
}

Eigen::MatrixXd Graph::getLaplacian() const
{
    return getDegree() - adj;
}

double Graph::getFiedlerValue()
{
    MatrixXd L = getLaplacian();
    SelfAdjointEigenSolver<MatrixXd> es(L);

    if (es.info() != Success)
    {
        return -1.0;
    }

    VectorXd evals = es.eigenvalues();

    if (n < 2)
    {
        return 0.0;
    }
    return evals(1);
}

// Graph Creators

Graph Graph::createPath(int n)
{
    Graph graph = Graph(n);
    for (int i = 0; i < n - 1; i++)
    {
        graph.addEdge(i, i + 1);
    }
    return graph;
}

Graph Graph::createCycle(int n)
{
    Graph graph = createPath(n);
    graph.addEdge(0, n - 1);
    return graph;
}

Graph Graph::createConnectedGraph(int n)
{
    Graph graph = Graph(n);
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < i; j++)
        {
            graph.addEdge(i, j);
        }
    }

    return graph;
}

Graph Graph::createBridgeGraph(int n)
{
    Graph graph = Graph(n);
    int half_point = n / 2;

    for (int i = 0; i < half_point; i++)
    {
        for (int j = 0; j < i; j++)
        {
            graph.addEdge(i, j);
        }
    }

    graph.addEdge(half_point - 1, half_point);

    for (int i = half_point; i < n; i++)
    {
        for (int j = half_point; j < i; j++)
        {
            graph.addEdge(i, j);
        }
    }

    return graph;
}

Graph Graph::createRandomGraph(int n, double p)
{
    Graph graph = Graph(n);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < i; j++)
        {
            if (dis(gen) <= p)
            {
                graph.addEdge(i, j);
            }
        }
    }

    return graph;
}

Graph Graph::createHubGraph(int n, double p_hub, double p_not_hub, double p_for_hub)
{
    Graph graph = Graph(n);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);

    std::vector<bool> isHub(n);

    for (int i = 0; i < n; i++)
    {
        if (dis(gen) <= p_hub)
        {
            isHub[i] = true;
        }
        else
        {
            isHub[i] = false;
        }
    }

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < i; j++)
        {
            double p_in_use = 0;
            if (isHub[j] || isHub[i])
            {
                p_in_use = p_for_hub;
            }
            else
            {
                p_in_use = p_not_hub;
            }
            if (dis(gen) <= p_in_use)
            {
                graph.addEdge(i, j);
            }
        }
    }

    return graph;
}