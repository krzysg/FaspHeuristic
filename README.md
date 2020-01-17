## TIGHT-CUT FASP heuristic

Library implementing **TIGHT-CUT** heuristic for solving **FASP** (Feedback Arc Set Problem).

If you use this library in an academic context, please cite the following paper:
- *Michael Hecht, Krzysztof Gonciarz, Szabolcs Horvát* [Tight Localizations of Feedback Sets](https://arxiv.org/abs/2001.01440)

To see 
### How to clone code
Run from command line following command:
```bash
git clone https://github.com/krzysg/FaspHeuristic.git
```

of if you prefere to access github with SSH keys:
```bash
git clone git@github.com:krzysg/FaspHeuristic.git
```

### How to compile
Library has these options available:
```
FASP_HEURISTIC_BUILD_TESTS (default: OFF)
FASP_HEURISTIC_BUILD_BENCHMARKS (default: OFF)
FASP_HEURISTIC_INSTALL (default: ON)
```

To compile, test and install it in default location please execute following commands (starting in root directory after downloading):
```bash
mkdir build
cd build
cmake -DFASP_HEURISTIC_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Release ..
make
ctest
make install
```

To install library in non default location in `cmake` line provide also prefix for install direcotry:
```bash
-DCMAKE_INSTALL_PREFIX=/user/specified/install/directory
```

### How to use in your project
TIGHT-CUT library can be used as a submodule of your project or can be used as a regular installed library.

### Simple example
Simple project using installed TIGHT-CUT library requires only two files `main.cpp` with a code and simple configuration `CMakeLists.txt` file.

**main.cpp**
```c++
#include <iostream>

#include "FaspTightCut/graph.h"
#include "FaspTightCut/graphFasp.h"

int main() {
    // Create graph by adding vertices and edges to the graph
    using VERTEX_TYPE = int;
    Graph::Graph<VERTEX_TYPE> g;
    for (int i = 0; i < 8; ++i) g.addVertex(i);
    g.addEdge({0, 1});
    g.addEdge({1, 2});
    g.addEdge({2, 3});
    g.addEdge({3, 4});
    g.addEdge({4, 5});
    g.addEdge({5, 0});

    g.addEdge({3, 6});
    g.addEdge({6, 2});

    g.addEdge({5, 7});
    g.addEdge({7, 4});

    // Add capacity to edges
    auto ep = Graph::Ext::getEdgeProperties(g, 1);
    ep[{2, 3}] = 3;
    ep[{3, 6}] = 1;
    ep[{6, 2}] = 1;
    ep[{4, 5}] = 3;
    ep[{5, 7}] = 1;
    ep[{7, 4}] = 1;

    // Run TIGHT-CUT heuristic
    auto [capacity, removedEdges, saEdgesCnt, saRndEdgesCnt, redRndEdgesCnt] = Graph::Fasp::tightCut<true, true, VERTEX_TYPE, int>(g, ep);

    // Print result
    std::cout << "Capacity of FASP edges: " << capacity << std::endl;

    return 0;
}
```

**CMakeLists.txt**
```cmake
cmake_minimum_required(VERSION 3.15)
project(TightCutExample LANGUAGES CXX)

find_package(FaspTightCut REQUIRED)

add_executable(main main.cpp)
target_link_libraries(main FaspTightCut::FaspTightCut)
```

