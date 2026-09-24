# Spectral Graph Analysis Engine

[![CI](https://github.com/SeanGa73/SpectralGraphEngine/actions/workflows/ci.yml/badge.svg)](https://github.com/SeanGa73/SpectralGraphEngine/actions/workflows/ci.yml)

# Elements 

* Matrix Representations: Dense Adjacency and Degree matrices using `Eigen3`.
* Graph Laplacian: Implements L = D - A to capture graph topology, where D and A are the Degree and Adjacency matrices.
* Eigen-Decomposition: Solves Lx = (lambda) x to identify spectral properties, including the Fiedler value (algebraic connectivity).
* Connectivity: `isConnected(tol)` checks whether the Fiedler value is above a tolerance, so rounding noise on a disconnected graph is not mistaken for connectivity.
* Graph Generators: Procedural generation of Path, Cycle, Complete, Bridge, Random, and Hub graphs. The random generators take an optional seed for reproducible output.

## API at a glance
```cpp
Graph g = Graph::createRandomGraph(50, 0.1, /*seed=*/42);
g.addEdge(0, 1);            // throws std::invalid_argument on self-loops or out-of-range nodes
g.hasEdge(0, 1);            // true
g.degree(0);                // number of neighbours
g.edgeCount();              // number of undirected edges
g.getLaplacian();           // L = D - A
g.getFiedlerValue();        // second-smallest Laplacian eigenvalue
g.isConnected();            // Fiedler value > 1e-9
```


## Usage
```bash
mkdir build 
cd build
cmake ..
make
./SpectralSolver
```

## Running Tests
Tests use GoogleTest. An installed copy is used if found; otherwise CMake downloads it automatically.
```bash
cd build
cmake ..
make
ctest --output-on-failure
```
Pass `-DBUILD_TESTS=OFF` to `cmake` to skip building the tests, or `-DWARNINGS_AS_ERRORS=ON` to fail the build on compiler warnings (CI does this).

## Formatting
Code is formatted with `clang-format` (version 18) using the repo's `.clang-format`:
```bash
clang-format -i include/*.h src/*.cpp tests/*.cpp
```

## Example Outputs
Generating Hub Graph (N=10)...
--- Adjacency Matrix ---
0 0 0 0 0 0 0 1 0 0
0 0 0 0 0 0 0 1 0 0
0 0 0 0 0 0 0 1 0 0
0 0 0 0 0 0 0 1 0 0
0 0 0 0 0 0 0 1 0 0
0 0 0 0 0 0 0 1 0 0
0 0 0 0 0 0 0 1 0 0
1 1 1 1 1 1 1 0 1 0
0 0 0 0 0 0 0 1 0 0
0 0 0 0 0 0 0 0 0 0
Fiedler Value: 0
Status: DISCONNECTED

The hub graph is random, so your matrix will differ from run to run. Note: Node 9 is disconnected from the graph, mathematically the Fiedler Value is zero. Intuitively, Fiedler Value gives a sense of connectivity and a disconnected node is consistent with a Fiedler Value of zero. 

Generating Bridge Graph (N=10)...
--- Adjacency Matrix ---
0 1 1 1 1 0 0 0 0 0
1 0 1 1 1 0 0 0 0 0
1 1 0 1 1 0 0 0 0 0
1 1 1 0 1 0 0 0 0 0
1 1 1 1 0 1 0 0 0 0
0 0 0 0 1 0 1 1 1 1
0 0 0 0 0 1 0 1 1 1
0 0 0 0 0 1 1 0 1 1
0 0 0 0 0 1 1 1 0 1
0 0 0 0 0 1 1 1 1 0
Fiedler Value: 0.298438
Status: CONNECTED
Note: We have two hub-like clusters connected by a single edge, the "bridge". This can be seen by the number of 1s in the top-left handside and bottom-right handside, and the number of 0s in the top-right handside and bottom-left handside. This highlights the graph is easy to "cut" and has a weak point. 