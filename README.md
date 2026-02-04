# Spectral Graph Analysis Engine

# Elements 

* Matrix Representations: Efficient computation of Adjacency and Degree matrices using `Eigen3`. 
* Graph Laplacian: Implements L = D - A to capture graph topology, where D, A = Adjacency and Degree matrices.
* Eigen-Decomposition: Solves Lx = (lambda) x to identify spectral properties.
* Graph Generators: Procedural generation of Hub, Bridge, and Random graphs.


## Usage
```bash
mkdir build 
cd build
cmake ..
make
./SpectralSolver

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

Note: Node 9 is disconnected from the graph, mathematically the Fiedler Value is zero. Intuitively, Fiedler Value gives a sense of connectivity and a disconnected node is consistent with a Fiedler Value of zero. 

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