#pragma once
#include <Eigen/Dense>
#include <cstdint>
#include <random>

class Graph {
private:
    int n;
    Eigen::MatrixXd adj;

    // Internal helpers
    bool validEdge(int u, int v) const;
    void checkNode(int u) const;

public:
    // Default tolerance for treating an eigenvalue as zero.
    static constexpr double kDefaultTolerance = 1e-9;

    // Constructor. Throws std::invalid_argument if n is negative.
    explicit Graph(int n);

    // Getters
    int getN() const;
    bool hasEdge(int u, int v) const;
    int degree(int u) const;
    int edgeCount() const;

    // Core Logic. Throws std::invalid_argument on self-loops or out-of-range nodes.
    void addEdge(int u, int v);
    void printGraph() const;

    // Spectral Math
    Eigen::MatrixXd getDegree() const;
    Eigen::MatrixXd getLaplacian() const;
    // Throws std::runtime_error if the eigensolver fails.
    double getFiedlerValue() const;
    bool isConnected(double tol = kDefaultTolerance) const;

    // Static Generators. Random generators take an optional seed for reproducible output.
    static Graph createPath(int n);
    static Graph createCycle(int n);
    static Graph createConnectedGraph(int n);
    static Graph createBridgeGraph(int n);
    static Graph createRandomGraph(int n, double p, std::uint32_t seed = std::random_device{}());
    static Graph createHubGraph(int n, double p_hub, double p_not_hub, double p_for_hub,
                                std::uint32_t seed = std::random_device{}());
};
