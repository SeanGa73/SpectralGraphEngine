#include <iostream>
#include "Graph.h"

using namespace std;

int main() {
    int N = 10;
    
    // --- Test 1: Hub Graph ---
    cout << "Generating Hub Graph (N=10)..." << endl;
    Graph g = Graph::createHubGraph(N, 0.2, 0.0, 0.9);
    
    g.printGraph();
    
    double fiedler = g.getFiedlerValue();
    cout << "Fiedler Value: " << fiedler << endl;
    
    if(fiedler > 0) cout << "Status: CONNECTED" << endl;
    else cout << "Status: DISCONNECTED" << endl;

    // --- Test 2: Bridge Graph ---
    cout << "\nGenerating Bridge Graph (N=10)..." << endl; 
    
    g = Graph::createBridgeGraph(N); 
    
    g.printGraph();
    
    fiedler = g.getFiedlerValue();
    cout << "Fiedler Value: " << fiedler << endl;
    
    if(fiedler > 0) cout << "Status: CONNECTED" << endl;
    else cout << "Status: DISCONNECTED" << endl;

    return 0;
}