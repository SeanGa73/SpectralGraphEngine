#include <gtest/gtest.h>

#include <cmath>
#include <string>

#include <Eigen/Dense>

#include "Graph.h"

using Eigen::MatrixXd;
using Eigen::SelfAdjointEigenSolver;
using Eigen::VectorXd;

namespace {

const double kTol = 1e-9;
const double kPi = std::acos(-1.0);

bool hasEdge(const Graph& g, int u, int v) {
    return g.getLaplacian()(u, v) == -1.0;
}

int edgeCount(const Graph& g) {
    return static_cast<int>(std::lround(g.getDegree().trace() / 2.0));
}

VectorXd laplacianEigenvalues(const Graph& g) {
    SelfAdjointEigenSolver<MatrixXd> es(g.getLaplacian());
    return es.eigenvalues();
}

int zeroEigenvalueCount(const Graph& g) {
    VectorXd evals = laplacianEigenvalues(g);
    int count = 0;
    for (int i = 0; i < evals.size(); i++) {
        if (std::abs(evals(i)) < 1e-8) count++;
    }
    return count;
}

// Checks the invariants every graph Laplacian must satisfy.
void expectValidLaplacian(const Graph& g) {
    MatrixXd L = g.getLaplacian();
    int n = g.getN();
    ASSERT_EQ(L.rows(), n);
    ASSERT_EQ(L.cols(), n);
    EXPECT_TRUE(L.isApprox(L.transpose()));
    for (int i = 0; i < n; i++) {
        EXPECT_NEAR(L.row(i).sum(), 0.0, kTol) << "row " << i;
    }
    if (n > 0) {
        EXPECT_GE(laplacianEigenvalues(g).minCoeff(), -1e-9);
    }
}

}  // namespace

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

TEST(GraphConstruction, StoresNodeCount) {
    EXPECT_EQ(Graph(0).getN(), 0);
    EXPECT_EQ(Graph(1).getN(), 1);
    EXPECT_EQ(Graph(7).getN(), 7);
}

TEST(GraphConstruction, NewGraphHasNoEdges) {
    Graph g(5);
    EXPECT_EQ(edgeCount(g), 0);
    EXPECT_TRUE(g.getLaplacian().isZero());
    EXPECT_TRUE(g.getDegree().isZero());
}

TEST(GraphConstruction, MatricesHaveCorrectDimensions) {
    Graph g(4);
    EXPECT_EQ(g.getDegree().rows(), 4);
    EXPECT_EQ(g.getDegree().cols(), 4);
    EXPECT_EQ(g.getLaplacian().rows(), 4);
    EXPECT_EQ(g.getLaplacian().cols(), 4);
}

// ---------------------------------------------------------------------------
// addEdge
// ---------------------------------------------------------------------------

TEST(AddEdge, IsUndirected) {
    Graph g(3);
    g.addEdge(0, 2);
    EXPECT_TRUE(hasEdge(g, 0, 2));
    EXPECT_TRUE(hasEdge(g, 2, 0));
    EXPECT_FALSE(hasEdge(g, 0, 1));
    EXPECT_FALSE(hasEdge(g, 1, 2));
}

TEST(AddEdge, DuplicateEdgeIsIdempotent) {
    Graph g(3);
    g.addEdge(0, 1);
    g.addEdge(0, 1);
    g.addEdge(1, 0);
    EXPECT_EQ(edgeCount(g), 1);
    EXPECT_DOUBLE_EQ(g.getDegree()(0, 0), 1.0);
}

TEST(AddEdge, SelfLoopIsRejected) {
    Graph g(3);
    testing::internal::CaptureStderr();
    g.addEdge(1, 1);
    std::string err = testing::internal::GetCapturedStderr();
    EXPECT_EQ(edgeCount(g), 0);
    EXPECT_NE(err.find("Invalid edge"), std::string::npos);
}

TEST(AddEdge, OutOfRangeIndexIsRejected) {
    Graph g(3);
    testing::internal::CaptureStderr();
    g.addEdge(0, 3);
    g.addEdge(3, 0);
    g.addEdge(10, 20);
    std::string err = testing::internal::GetCapturedStderr();
    EXPECT_EQ(edgeCount(g), 0);
    EXPECT_NE(err.find("Invalid edge"), std::string::npos);
}

TEST(AddEdge, NegativeIndexIsRejected) {
    Graph g(3);
    testing::internal::CaptureStderr();
    g.addEdge(-1, 0);
    g.addEdge(0, -1);
    std::string err = testing::internal::GetCapturedStderr();
    EXPECT_EQ(edgeCount(g), 0);
    EXPECT_NE(err.find("Invalid edge"), std::string::npos);
}

TEST(AddEdge, ValidEdgeProducesNoWarning) {
    Graph g(3);
    testing::internal::CaptureStderr();
    g.addEdge(0, 1);
    EXPECT_TRUE(testing::internal::GetCapturedStderr().empty());
}

// ---------------------------------------------------------------------------
// printGraph
// ---------------------------------------------------------------------------

TEST(PrintGraph, PrintsHeaderAndMatrix) {
    Graph g(2);
    g.addEdge(0, 1);
    testing::internal::CaptureStdout();
    g.printGraph();
    std::string out = testing::internal::GetCapturedStdout();
    EXPECT_NE(out.find("--- Adjacency Matrix ---"), std::string::npos);
    EXPECT_NE(out.find("0 1"), std::string::npos);
    EXPECT_NE(out.find("1 0"), std::string::npos);
}

// ---------------------------------------------------------------------------
// Degree matrix
// ---------------------------------------------------------------------------

TEST(DegreeMatrix, IsDiagonalWithVertexDegrees) {
    // Star: node 0 is connected to 1, 2, 3.
    Graph g(4);
    g.addEdge(0, 1);
    g.addEdge(0, 2);
    g.addEdge(0, 3);
    MatrixXd D = g.getDegree();
    EXPECT_TRUE(D.isApprox(MatrixXd(D.diagonal().asDiagonal())));
    EXPECT_DOUBLE_EQ(D(0, 0), 3.0);
    EXPECT_DOUBLE_EQ(D(1, 1), 1.0);
    EXPECT_DOUBLE_EQ(D(2, 2), 1.0);
    EXPECT_DOUBLE_EQ(D(3, 3), 1.0);
}

TEST(DegreeMatrix, TraceIsTwiceEdgeCount) {
    Graph g = Graph::createCycle(6);
    EXPECT_DOUBLE_EQ(g.getDegree().trace(), 12.0);
}

// ---------------------------------------------------------------------------
// Laplacian
// ---------------------------------------------------------------------------

TEST(Laplacian, EqualsDegreeMinusAdjacency) {
    Graph g(3);
    g.addEdge(0, 1);
    g.addEdge(1, 2);
    MatrixXd expected(3, 3);
    expected << 1, -1, 0,
               -1, 2, -1,
                0, -1, 1;
    EXPECT_TRUE(g.getLaplacian().isApprox(expected));
}

TEST(Laplacian, SatisfiesInvariantsForAllGenerators) {
    for (int n : {2, 3, 5, 8, 13}) {
        SCOPED_TRACE("n = " + std::to_string(n));
        expectValidLaplacian(Graph::createPath(n));
        expectValidLaplacian(Graph::createConnectedGraph(n));
        expectValidLaplacian(Graph::createBridgeGraph(n));
        expectValidLaplacian(Graph::createRandomGraph(n, 0.4));
        expectValidLaplacian(Graph::createHubGraph(n, 0.3, 0.1, 0.8));
        if (n >= 3) expectValidLaplacian(Graph::createCycle(n));
    }
}

TEST(Laplacian, SmallestEigenvalueIsZero) {
    for (int n : {1, 2, 5, 10}) {
        SCOPED_TRACE("n = " + std::to_string(n));
        EXPECT_NEAR(laplacianEigenvalues(Graph::createPath(n)).minCoeff(), 0.0, 1e-9);
    }
}

TEST(Laplacian, ZeroEigenvalueMultiplicityEqualsComponentCount) {
    Graph g(6);  // components: {0,1,2}, {3,4}, {5}
    g.addEdge(0, 1);
    g.addEdge(1, 2);
    g.addEdge(3, 4);
    EXPECT_EQ(zeroEigenvalueCount(g), 3);

    EXPECT_EQ(zeroEigenvalueCount(Graph(4)), 4);
    EXPECT_EQ(zeroEigenvalueCount(Graph::createConnectedGraph(4)), 1);
}

// ---------------------------------------------------------------------------
// Fiedler value (algebraic connectivity)
// ---------------------------------------------------------------------------

TEST(FiedlerValue, TrivialGraphsReturnZero) {
    Graph single(1);
    EXPECT_DOUBLE_EQ(single.getFiedlerValue(), 0.0);
}

TEST(FiedlerValue, SingleEdgeIsTwo) {
    Graph g(2);
    g.addEdge(0, 1);
    EXPECT_NEAR(g.getFiedlerValue(), 2.0, kTol);
}

TEST(FiedlerValue, EmptyGraphIsZero) {
    Graph g(5);
    EXPECT_NEAR(g.getFiedlerValue(), 0.0, kTol);
}

TEST(FiedlerValue, DisconnectedGraphIsZero) {
    Graph g(6);
    g.addEdge(0, 1);
    g.addEdge(1, 2);
    g.addEdge(3, 4);
    g.addEdge(4, 5);
    EXPECT_NEAR(g.getFiedlerValue(), 0.0, kTol);
}

TEST(FiedlerValue, IsolatedNodeMakesGraphDisconnected) {
    Graph g = Graph::createConnectedGraph(5);
    Graph h(6);
    for (int i = 0; i < 5; i++)
        for (int j = 0; j < i; j++) h.addEdge(i, j);
    EXPECT_GT(g.getFiedlerValue(), 0.0);
    EXPECT_NEAR(h.getFiedlerValue(), 0.0, kTol);
}

TEST(FiedlerValue, PathMatchesClosedForm) {
    for (int n = 2; n <= 20; n++) {
        SCOPED_TRACE("n = " + std::to_string(n));
        Graph g = Graph::createPath(n);
        double expected = 2.0 * (1.0 - std::cos(kPi / n));
        EXPECT_NEAR(g.getFiedlerValue(), expected, 1e-8);
    }
}

TEST(FiedlerValue, CycleMatchesClosedForm) {
    for (int n = 3; n <= 20; n++) {
        SCOPED_TRACE("n = " + std::to_string(n));
        Graph g = Graph::createCycle(n);
        double expected = 2.0 * (1.0 - std::cos(2.0 * kPi / n));
        EXPECT_NEAR(g.getFiedlerValue(), expected, 1e-8);
    }
}

TEST(FiedlerValue, CompleteGraphEqualsN) {
    for (int n = 2; n <= 15; n++) {
        SCOPED_TRACE("n = " + std::to_string(n));
        EXPECT_NEAR(Graph::createConnectedGraph(n).getFiedlerValue(), n, 1e-8);
    }
}

TEST(FiedlerValue, StarGraphIsOne) {
    for (int n = 3; n <= 12; n++) {
        SCOPED_TRACE("n = " + std::to_string(n));
        Graph g(n);
        for (int i = 1; i < n; i++) g.addEdge(0, i);
        EXPECT_NEAR(g.getFiedlerValue(), 1.0, 1e-8);
    }
}

TEST(FiedlerValue, AddingEdgesNeverDecreasesIt) {
    Graph g = Graph::createPath(8);
    double prev = g.getFiedlerValue();
    for (int i = 0; i < 8; i++) {
        for (int j = i + 2; j < 8; j++) {
            g.addEdge(i, j);
            double next = g.getFiedlerValue();
            EXPECT_GE(next, prev - 1e-9) << "after adding edge " << i << "-" << j;
            prev = next;
        }
    }
    EXPECT_NEAR(prev, 8.0, 1e-8);  // now complete
}

TEST(FiedlerValue, BridgeGraphIsWeaklyConnected) {
    Graph bridge = Graph::createBridgeGraph(10);
    Graph complete = Graph::createConnectedGraph(10);
    double fb = bridge.getFiedlerValue();
    EXPECT_GT(fb, 0.0);
    EXPECT_LT(fb, 1.0);
    EXPECT_LT(fb, complete.getFiedlerValue());
}

TEST(FiedlerValue, IsInvariantUnderRelabelling) {
    Graph a(4);
    a.addEdge(0, 1);
    a.addEdge(1, 2);
    a.addEdge(2, 3);
    Graph b(4);  // same path, nodes permuted
    b.addEdge(2, 0);
    b.addEdge(0, 3);
    b.addEdge(3, 1);
    EXPECT_NEAR(a.getFiedlerValue(), b.getFiedlerValue(), 1e-9);
}

// ---------------------------------------------------------------------------
// Generators: deterministic
// ---------------------------------------------------------------------------

TEST(CreatePath, HasConsecutiveEdgesOnly) {
    Graph g = Graph::createPath(6);
    EXPECT_EQ(g.getN(), 6);
    EXPECT_EQ(edgeCount(g), 5);
    for (int i = 0; i < 5; i++) EXPECT_TRUE(hasEdge(g, i, i + 1));
    EXPECT_FALSE(hasEdge(g, 0, 5));
    EXPECT_DOUBLE_EQ(g.getDegree()(0, 0), 1.0);
    EXPECT_DOUBLE_EQ(g.getDegree()(5, 5), 1.0);
    for (int i = 1; i < 5; i++) EXPECT_DOUBLE_EQ(g.getDegree()(i, i), 2.0);
}

TEST(CreatePath, DegenerateSizes) {
    EXPECT_EQ(edgeCount(Graph::createPath(1)), 0);
    EXPECT_EQ(edgeCount(Graph::createPath(0)), 0);
}

TEST(CreateCycle, IsTwoRegularWithClosingEdge) {
    Graph g = Graph::createCycle(7);
    EXPECT_EQ(edgeCount(g), 7);
    EXPECT_TRUE(hasEdge(g, 0, 6));
    for (int i = 0; i < 7; i++) EXPECT_DOUBLE_EQ(g.getDegree()(i, i), 2.0);
}

TEST(CreateCycle, TriangleIsCompleteGraph) {
    EXPECT_TRUE(Graph::createCycle(3).getLaplacian().isApprox(
        Graph::createConnectedGraph(3).getLaplacian()));
}

TEST(CreateCycle, TwoNodesCollapsesToSingleEdge) {
    EXPECT_EQ(edgeCount(Graph::createCycle(2)), 1);
}

TEST(CreateConnectedGraph, IsComplete) {
    for (int n : {1, 2, 5, 9}) {
        SCOPED_TRACE("n = " + std::to_string(n));
        Graph g = Graph::createConnectedGraph(n);
        EXPECT_EQ(edgeCount(g), n * (n - 1) / 2);
        for (int i = 0; i < n; i++) EXPECT_DOUBLE_EQ(g.getDegree()(i, i), n - 1);
    }
}

TEST(CreateBridgeGraph, IsTwoCliquesJoinedByOneEdge) {
    const int n = 10, h = 5;
    Graph g = Graph::createBridgeGraph(n);
    EXPECT_EQ(edgeCount(g), 10 + 10 + 1);
    EXPECT_TRUE(hasEdge(g, h - 1, h));
    int crossEdges = 0;
    for (int i = 0; i < h; i++)
        for (int j = h; j < n; j++)
            if (hasEdge(g, i, j)) crossEdges++;
    EXPECT_EQ(crossEdges, 1);
    for (int i = 0; i < h; i++)
        for (int j = 0; j < i; j++) EXPECT_TRUE(hasEdge(g, i, j));
    for (int i = h; i < n; i++)
        for (int j = h; j < i; j++) EXPECT_TRUE(hasEdge(g, i, j));
}

TEST(CreateBridgeGraph, OddSizePutsExtraNodeInSecondClique) {
    Graph g = Graph::createBridgeGraph(7);  // cliques {0,1,2} and {3,4,5,6}
    EXPECT_EQ(edgeCount(g), 3 + 6 + 1);
    EXPECT_TRUE(hasEdge(g, 2, 3));
    EXPECT_GT(g.getFiedlerValue(), 0.0);
}

TEST(CreateBridgeGraph, RemovingBridgeDisconnects) {
    Graph g = Graph::createBridgeGraph(8);
    EXPECT_EQ(zeroEigenvalueCount(g), 1);
    Graph h(8);  // same cliques, no bridge
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < i; j++) h.addEdge(i, j);
    for (int i = 4; i < 8; i++)
        for (int j = 4; j < i; j++) h.addEdge(i, j);
    EXPECT_EQ(zeroEigenvalueCount(h), 2);
}

TEST(CreateBridgeGraph, SingleNodeDoesNotCrash) {
    testing::internal::CaptureStderr();
    Graph g = Graph::createBridgeGraph(1);
    testing::internal::GetCapturedStderr();
    EXPECT_EQ(g.getN(), 1);
    EXPECT_EQ(edgeCount(g), 0);
}

// ---------------------------------------------------------------------------
// Generators: random
// ---------------------------------------------------------------------------

TEST(CreateRandomGraph, ProbabilityZeroGivesEmptyGraph) {
    EXPECT_EQ(edgeCount(Graph::createRandomGraph(20, 0.0)), 0);
}

TEST(CreateRandomGraph, ProbabilityOneGivesCompleteGraph) {
    Graph g = Graph::createRandomGraph(12, 1.0);
    EXPECT_TRUE(g.getLaplacian().isApprox(Graph::createConnectedGraph(12).getLaplacian()));
}

TEST(CreateRandomGraph, EdgeDensityTracksProbability) {
    // 80 nodes -> 3160 possible edges; at p = 0.3 the std dev is ~26 edges,
    // so a +/-0.05 density window is ~6 sigma wide.
    const int n = 80;
    const double maxEdges = n * (n - 1) / 2.0;
    for (double p : {0.1, 0.3, 0.7}) {
        SCOPED_TRACE("p = " + std::to_string(p));
        double density = edgeCount(Graph::createRandomGraph(n, p)) / maxEdges;
        EXPECT_NEAR(density, p, 0.05);
    }
}

TEST(CreateRandomGraph, HasNoSelfLoops) {
    Graph g = Graph::createRandomGraph(15, 1.0);
    MatrixXd L = g.getLaplacian();
    for (int i = 0; i < 15; i++) EXPECT_DOUBLE_EQ(L(i, i), 14.0);
}

// ---------------------------------------------------------------------------
// Generators: hub
// ---------------------------------------------------------------------------

TEST(CreateHubGraph, AllZeroProbabilitiesGiveEmptyGraph) {
    EXPECT_EQ(edgeCount(Graph::createHubGraph(15, 0.5, 0.0, 0.0)), 0);
}

TEST(CreateHubGraph, NoHubsUsesNonHubProbability) {
    EXPECT_EQ(edgeCount(Graph::createHubGraph(10, 0.0, 1.0, 0.0)), 45);
    EXPECT_EQ(edgeCount(Graph::createHubGraph(10, 0.0, 0.0, 1.0)), 0);
}

TEST(CreateHubGraph, AllHubsUsesHubProbability) {
    EXPECT_EQ(edgeCount(Graph::createHubGraph(10, 1.0, 0.0, 1.0)), 45);
    EXPECT_EQ(edgeCount(Graph::createHubGraph(10, 1.0, 1.0, 0.0)), 0);
}

TEST(CreateHubGraph, EdgesOnlyTouchHubsWhenNonHubProbabilityIsZero) {
    // With p_not_hub = 0 and p_for_hub = 1, the graph is a union of stars:
    // every edge has at least one hub endpoint, and hubs link to everything.
    for (int trial = 0; trial < 20; trial++) {
        Graph g = Graph::createHubGraph(12, 0.25, 0.0, 1.0);
        MatrixXd D = g.getDegree();
        int n = g.getN();
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < i; j++) {
                bool iHub = D(i, i) == n - 1;
                bool jHub = D(j, j) == n - 1;
                EXPECT_EQ(hasEdge(g, i, j), iHub || jHub)
                    << "trial " << trial << ", edge " << i << "-" << j;
            }
        }
    }
}

TEST(CreateHubGraph, HasValidStructure) {
    Graph g = Graph::createHubGraph(30, 0.2, 0.05, 0.9);
    EXPECT_EQ(g.getN(), 30);
    expectValidLaplacian(g);
}
