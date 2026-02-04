#include <iostream>
#include "Graph.h" // Only include the header!

using namespace std;

int main() {
    cout << "--- Spectral Engine (Refactored) ---" << endl;

    int N = 10;
    
    // Test the Hub Graph Logic
    cout << "Generating Hub Graph (N=10)..." << endl;
    Graph g = Graph::createHubGraph(N, 0.2, 0.0, 0.9);
    
    g.printGraph();
    
    double fiedler = g.getFiedlerValue();
    cout << "Fiedler Value: " << fiedler << endl;
    
    if(fiedler > 0) cout << "Status: CONNECTED" << endl;
    else cout << "Status: DISCONNECTED" << endl;

    return 0;
}
