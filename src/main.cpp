#include <iostream>

#include "Graph.h"

using namespace std;

namespace {

void report(const Graph& g) {
    g.printGraph();
    cout << "Fiedler Value: " << g.getFiedlerValue() << endl;
    cout << "Status: " << (g.isConnected() ? "CONNECTED" : "DISCONNECTED") << endl;
}

}  // namespace

int main() {
    const int N = 10;

    // --- Example 1: Hub Graph ---
    cout << "Generating Hub Graph (N=10)..." << endl;
    report(Graph::createHubGraph(N, 0.2, 0.0, 0.9));

    // --- Example 2: Bridge Graph ---
    cout << "\nGenerating Bridge Graph (N=10)..." << endl;
    report(Graph::createBridgeGraph(N));

    return 0;
}
