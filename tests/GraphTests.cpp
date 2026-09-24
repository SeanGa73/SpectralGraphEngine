#include <gtest/gtest.h>

#include <Eigen/Dense>
#include <cmath>
#include <stdexcept>
#include <string>

#include "Graph.h"

using Eigen::MatrixXd;
using Eigen::SelfAdjointEigenSolver;
using Eigen::VectorXd;

namespace {

const double kTol = 1e-9;
const double kPi = std::acos(-1.0);

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

TEST(GraphConstruction, NegativeSizeThrows) {
    EXPECT_THROW(Graph(-1), std::invalid_argument);
}

TEST(GraphConstruction, NewGraphHasNoEdges) {
    Graph g(5);
    EXPECT_EQ(g.edgeCount(), 0);
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
    EXPECT_TRUE(g.hasEdge(0, 2));
    EXPECT_TRUE(g.hasEdge(2, 0));
    EXPECT_FALSE(g.hasEdge(0, 1));
    EXPECT_FALSE(g.hasEdge(1, 2));
}

TEST(AddEdge, DuplicateEdgeIsIdempotent) {
    Graph g(3);
    g.addEdge(0, 1);
    g.addEdge(0, 1);
    g.addEdge(1, 0);
    EXPECT_EQ(g.edgeCount(), 1);
    EXPECT_DOUBLE_EQ(g.getDegree()(0, 0), 1.0);
}

TEST(AddEdge, SelfLoopThrows) {
    Graph g(3);
    EXPECT_THROW(g.addEdge(1, 1), std::invalid_argument);
    EXPECT_EQ(g.edgeCount(), 0);
}

TEST(AddEdge, OutOfRangeIndexThrows) {
    Graph g(3);
    EXPECT_THROW(g.addEdge(0, 3), std::invalid_argument);
    EXPECT_THROW(g.addEdge(3, 0), std::invalid_argument);
    EXPECT_THROW(g.addEdge(10, 20), std::invalid_argument);
    EXPECT_EQ(g.edgeCount(), 0);
}

TEST(AddEdge, NegativeIndexThrows) {
    Graph g(3);
    EXPECT_THROW(g.addEdge(-1, 0), std::invalid_argument);
    EXPECT_THROW(g.addEdge(0, -1), std::invalid_argument);
    EXPECT_EQ(g.edgeCount(), 0);
}

TEST(AddEdge, ValidEdgeProducesNoOutput) {
    Graph g(3);
    testing::internal::CaptureStderr();
    g.addEdge(0, 1);
    EXPECT_TRUE(testing::internal::GetCapturedStderr().empty());
}

// ---------------------------------------------------------------------------
// Queries: hasEdge, degree, edgeCount
// ---------------------------------------------------------------------------

TEST(Queries, DegreeMatchesDegreeMatrix) {
    Graph g = Graph::createHubGraph(12, 0.3, 0.2, 0.8, 7);
    MatrixXd D = g.getDegree();
    for (int i = 0; i < g.getN(); i++) EXPECT_EQ(g.degree(i), D(i, i));
}

TEST(Queries, EdgeCountIsHalfDegreeSum) {
    Graph g = Graph::createRandomGraph(20, 0.4, 3);
    int degreeSum = 0;
    for (int i = 0; i < g.getN(); i++) degreeSum += g.degree(i);
    EXPECT_EQ(g.edgeCount() * 2, degreeSum);
}

TEST(Queries, OutOfRangeNodeThrows) {
    Graph g(3);
    EXPECT_THROW(g.hasEdge(0, 3), std::out_of_range);
    EXPECT_THROW(g.hasEdge(-1, 0), std::out_of_range);
    EXPECT_THROW(g.degree(3), std::out_of_range);
    EXPECT_THROW(g.degree(-1), std::out_of_range);
}

TEST(Queries, NodeHasNoEdgeToItself) {
    Graph g = Graph::createConnectedGraph(4);
    for (int i = 0; i < 4; i++) EXPECT_FALSE(g.hasEdge(i, i));
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
    // clang-format off
    expected <<  1, -1,  0,
                -1,  2, -1,
                 0, -1,  1;
    // clang-format on
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
// isConnected
// ---------------------------------------------------------------------------

TEST(IsConnected, TrivialGraphsAreConnected) {
    EXPECT_TRUE(Graph(0).isConnected());
    EXPECT_TRUE(Graph(1).isConnected());
}

TEST(IsConnected, MatchesComponentCount) {
    EXPECT_TRUE(Graph::createPath(10).isConnected());
    EXPECT_TRUE(Graph::createBridgeGraph(10).isConnected());
    EXPECT_FALSE(Graph(2).isConnected());

    Graph g(6);
    g.addEdge(0, 1);
    g.addEdge(1, 2);
    g.addEdge(3, 4);
    g.addEdge(4, 5);
    EXPECT_FALSE(g.isConnected());
    g.addEdge(2, 3);
    EXPECT_TRUE(g.isConnected());
}

TEST(IsConnected, IgnoresRoundingNoiseOnDisconnectedGraphs) {
    // Two dense components: lambda_2 is zero in exact arithmetic but may come
    // back as a tiny nonzero value, which must not be read as "connected".
    for (int n = 4; n <= 40; n += 6) {
        SCOPED_TRACE("n = " + std::to_string(n));
        Graph g(n);
        for (int i = 0; i < n / 2; i++)
            for (int j = 0; j < i; j++) g.addEdge(i, j);
        for (int i = n / 2; i < n; i++)
            for (int j = n / 2; j < i; j++) g.addEdge(i, j);
        EXPECT_FALSE(g.isConnected());
    }
}

TEST(IsConnected, ToleranceIsRespected) {
    Graph g = Graph::createPath(50);  // lambda_2 ~= 0.0039
    EXPECT_TRUE(g.isConnected());
    EXPECT_FALSE(g.isConnected(0.01));
}

// ---------------------------------------------------------------------------
// Generators: deterministic
// ---------------------------------------------------------------------------

TEST(CreatePath, HasConsecutiveEdgesOnly) {
    Graph g = Graph::createPath(6);
    EXPECT_EQ(g.getN(), 6);
    EXPECT_EQ(g.edgeCount(), 5);
    for (int i = 0; i < 5; i++) EXPECT_TRUE(g.hasEdge(i, i + 1));
    EXPECT_FALSE(g.hasEdge(0, 5));
    EXPECT_DOUBLE_EQ(g.getDegree()(0, 0), 1.0);
    EXPECT_DOUBLE_EQ(g.getDegree()(5, 5), 1.0);
    for (int i = 1; i < 5; i++) EXPECT_DOUBLE_EQ(g.getDegree()(i, i), 2.0);
}

TEST(CreatePath, DegenerateSizes) {
    EXPECT_EQ(Graph::createPath(1).edgeCount(), 0);
    EXPECT_EQ(Graph::createPath(0).edgeCount(), 0);
}

TEST(CreateCycle, IsTwoRegularWithClosingEdge) {
    Graph g = Graph::createCycle(7);
    EXPECT_EQ(g.edgeCount(), 7);
    EXPECT_TRUE(g.hasEdge(0, 6));
    for (int i = 0; i < 7; i++) EXPECT_DOUBLE_EQ(g.getDegree()(i, i), 2.0);
}

TEST(CreateCycle, TriangleIsCompleteGraph) {
    EXPECT_TRUE(Graph::createCycle(3).getLaplacian().isApprox(
        Graph::createConnectedGraph(3).getLaplacian()));
}

TEST(CreateCycle, TwoNodesCollapsesToSingleEdge) {
    EXPECT_EQ(Graph::createCycle(2).edgeCount(), 1);
}

TEST(CreateCycle, DegenerateSizes) {
    EXPECT_EQ(Graph::createCycle(0).edgeCount(), 0);
    EXPECT_EQ(Graph::createCycle(1).edgeCount(), 0);
}

TEST(CreateConnectedGraph, IsComplete) {
    for (int n : {1, 2, 5, 9}) {
        SCOPED_TRACE("n = " + std::to_string(n));
        Graph g = Graph::createConnectedGraph(n);
        EXPECT_EQ(g.edgeCount(), n * (n - 1) / 2);
        for (int i = 0; i < n; i++) EXPECT_DOUBLE_EQ(g.getDegree()(i, i), n - 1);
    }
}

TEST(CreateBridgeGraph, IsTwoCliquesJoinedByOneEdge) {
    const int n = 10, h = 5;
    Graph g = Graph::createBridgeGraph(n);
    EXPECT_EQ(g.edgeCount(), 10 + 10 + 1);
    EXPECT_TRUE(g.hasEdge(h - 1, h));
    int crossEdges = 0;
    for (int i = 0; i < h; i++)
        for (int j = h; j < n; j++)
            if (g.hasEdge(i, j)) crossEdges++;
    EXPECT_EQ(crossEdges, 1);
    for (int i = 0; i < h; i++)
        for (int j = 0; j < i; j++) EXPECT_TRUE(g.hasEdge(i, j));
    for (int i = h; i < n; i++)
        for (int j = h; j < i; j++) EXPECT_TRUE(g.hasEdge(i, j));
}

TEST(CreateBridgeGraph, OddSizePutsExtraNodeInSecondClique) {
    Graph g = Graph::createBridgeGraph(7);  // cliques {0,1,2} and {3,4,5,6}
    EXPECT_EQ(g.edgeCount(), 3 + 6 + 1);
    EXPECT_TRUE(g.hasEdge(2, 3));
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

TEST(CreateBridgeGraph, DegenerateSizes) {
    EXPECT_EQ(Graph::createBridgeGraph(0).edgeCount(), 0);
    Graph single = Graph::createBridgeGraph(1);
    EXPECT_EQ(single.getN(), 1);
    EXPECT_EQ(single.edgeCount(), 0);
    Graph pair = Graph::createBridgeGraph(2);
    EXPECT_EQ(pair.edgeCount(), 1);
    EXPECT_TRUE(pair.hasEdge(0, 1));
}

// ---------------------------------------------------------------------------
// Generators: random
// ---------------------------------------------------------------------------

TEST(CreateRandomGraph, ProbabilityZeroGivesEmptyGraph) {
    EXPECT_EQ(Graph::createRandomGraph(20, 0.0).edgeCount(), 0);
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
        double density = Graph::createRandomGraph(n, p).edgeCount() / maxEdges;
        EXPECT_NEAR(density, p, 0.05);
    }
}

TEST(CreateRandomGraph, HasNoSelfLoops) {
    Graph g = Graph::createRandomGraph(15, 1.0);
    MatrixXd L = g.getLaplacian();
    for (int i = 0; i < 15; i++) EXPECT_DOUBLE_EQ(L(i, i), 14.0);
}

TEST(CreateRandomGraph, SameSeedGivesSameGraph) {
    Graph a = Graph::createRandomGraph(30, 0.3, 42);
    Graph b = Graph::createRandomGraph(30, 0.3, 42);
    EXPECT_TRUE(a.getLaplacian().isApprox(b.getLaplacian()));
}

TEST(CreateRandomGraph, DifferentSeedsGiveDifferentGraphs) {
    Graph a = Graph::createRandomGraph(30, 0.5, 1);
    Graph b = Graph::createRandomGraph(30, 0.5, 2);
    EXPECT_FALSE(a.getLaplacian().isApprox(b.getLaplacian()));
}

// ---------------------------------------------------------------------------
// Generators: hub
// ---------------------------------------------------------------------------

TEST(CreateHubGraph, AllZeroProbabilitiesGiveEmptyGraph) {
    EXPECT_EQ(Graph::createHubGraph(15, 0.5, 0.0, 0.0).edgeCount(), 0);
}

TEST(CreateHubGraph, NoHubsUsesNonHubProbability) {
    EXPECT_EQ(Graph::createHubGraph(10, 0.0, 1.0, 0.0).edgeCount(), 45);
    EXPECT_EQ(Graph::createHubGraph(10, 0.0, 0.0, 1.0).edgeCount(), 0);
}

TEST(CreateHubGraph, AllHubsUsesHubProbability) {
    EXPECT_EQ(Graph::createHubGraph(10, 1.0, 0.0, 1.0).edgeCount(), 45);
    EXPECT_EQ(Graph::createHubGraph(10, 1.0, 1.0, 0.0).edgeCount(), 0);
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
                EXPECT_EQ(g.hasEdge(i, j), iHub || jHub)
                    << "trial " << trial << ", edge " << i << "-" << j;
            }
        }
    }
}

TEST(CreateHubGraph, SameSeedGivesSameGraph) {
    Graph a = Graph::createHubGraph(25, 0.2, 0.1, 0.9, 123);
    Graph b = Graph::createHubGraph(25, 0.2, 0.1, 0.9, 123);
    EXPECT_TRUE(a.getLaplacian().isApprox(b.getLaplacian()));
}

TEST(CreateHubGraph, HasValidStructure) {
    Graph g = Graph::createHubGraph(30, 0.2, 0.05, 0.9);
    EXPECT_EQ(g.getN(), 30);
    expectValidLaplacian(g);
}
