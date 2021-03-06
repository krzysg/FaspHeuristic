#include "FaspTightCut/graphFasp.h"
#include "FaspTightCut/graph.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <iostream>

using ::testing::UnorderedElementsAreArray;
using ::testing::UnorderedElementsAre;
using ::testing::ElementsAre;


namespace {

    TEST(TestGraphFasp, testGraphSpeedUtils) {
	using VERTEX_TYPE = uint16_t;
        Graph::Fasp::GraphSpeedUtils<VERTEX_TYPE> u{7};
        // Test graph:
        //           /---> 2
        //          /---> 4
        //  0 ---> 1 ---> 3 ---> 5 ---> 6
        //                ^-------------

        using Edge = typename Graph::Graph<VERTEX_TYPE >::Edge;

        Graph::Graph<VERTEX_TYPE> g;
        for (int i = 0; i < 7; ++i) g.addVertex(i);
        g.addEdge({0, 1});
        g.addEdge({1, 2});
        g.addEdge({1, 3});
        g.addEdge({1, 4});
        g.addEdge({3, 5});
        g.addEdge({5, 6});
        g.addEdge({6, 3});

        {   // 'pathExistsDFS'
            ASSERT_TRUE(u.pathExistsDFS(g, 0, 6));
            ASSERT_TRUE(u.pathExistsDFS<false>(g, 6, 0));

            ASSERT_FALSE(u.pathExistsDFS(g, 6, 0));
            ASSERT_FALSE(u.pathExistsDFS<false>(g, 0, 6));

            ASSERT_FALSE(u.pathExistsDFS(g, 5, 4));
            ASSERT_FALSE(u.pathExistsDFS<false>(g, 4, 5));

            ASSERT_TRUE(u.pathExistsDFS(g, 3, 6));
            ASSERT_TRUE(u.pathExistsDFS(g, 6, 3));

            ASSERT_TRUE(u.pathExistsDFS(g, 3, 3));
        }

        {   // 'findPathDFS'
            auto [pathExist, path] = u.findPathDFS(g, 1, 6);
            ASSERT_TRUE(pathExist);
            ASSERT_THAT(path, ElementsAre(1, 3, 5, 6));

            auto [pathExist2, path2] = u.findPathDFS(g, 6, 1);
            ASSERT_FALSE(pathExist2);

            auto [pathExist3, path3] = u.findPathDFS(g, 2, 2);
            ASSERT_TRUE(pathExist3);
            ASSERT_THAT(path3, ElementsAre(2));
        }

        {   // 'isAcyclic'
            auto gg = g; // copy 'g'
            ASSERT_FALSE(u.isAcyclic(gg));
            gg.removeEdge({5, 6});
            ASSERT_TRUE(u.isAcyclic(gg));
        }

        {   // 'findEdgesWithCycles'
            auto edges = u.findEdgesWithCycles(g);

            ASSERT_EQ(edges.size(), 3);
            ASSERT_THAT(edges, UnorderedElementsAre(Edge{3, 5}, Edge{5, 6}, Edge{6, 3}));

            auto gg = g; // copy 'g'
            gg.removeEdge({5, 6});
            ASSERT_EQ(u.findEdgesWithCycles(gg).size(), 0);
        }

        { // 'stronglyConnectedComponents'
            auto scc = u.stronglyConnectedComponents(g);

            // We should get (order not important):
            // {6, 3, 5}, {4}, {2}, {1}, {0}
            std::vector<std::unordered_set<VERTEX_TYPE>> expected;
            expected.emplace_back(std::unordered_set<VERTEX_TYPE>{0});
            expected.emplace_back(std::unordered_set<VERTEX_TYPE>{1});
            expected.emplace_back(std::unordered_set<VERTEX_TYPE>{2});
            expected.emplace_back(std::unordered_set<VERTEX_TYPE>{4});
            expected.emplace_back(std::unordered_set<VERTEX_TYPE>{3, 5, 6});

            ASSERT_EQ(scc.size(), 5);
            ASSERT_THAT(scc, UnorderedElementsAreArray(expected));

            scc = u.stronglyConnectedComponents(g, true);
            ASSERT_EQ(scc.size(), 1);
            ASSERT_THAT(scc, UnorderedElementsAre(std::unordered_set<VERTEX_TYPE>{3, 5, 6}));

        }

        { // 'minStCut'
            Graph::Graph<VERTEX_TYPE> g;
            Graph::Fasp::GraphSpeedUtils<VERTEX_TYPE> u{6};
            for (int i = 0; i < 5; ++i) g.addVertex(i);
            // forward edge
            g.addEdge({0, 1});

            // backward edge
            g.addEdge({1, 0});

            // double-edge backward path
            g.addEdge({1, 2});
            g.addEdge({2, 0});

            // triple-edge backward path
            g.addEdge({1, 3});
            g.addEdge({3, 4});
            g.addEdge({4, 0});

            auto ep = Graph::Ext::getEdgeProperties(g, 1);

            // We need to cut one edge in each backward path so result should be 3
            ASSERT_EQ(u.minStCut(g, 1, 0, ep), 3);

            // We have only one edge in forward direction
            ASSERT_EQ(u.minStCut(g, 0, 1, ep), 1);

            // Add some weights to edges
            ep[{0, 1}] = 2;
            ep[{1, 0}] = 3;
            ep[{1, 2}] = 4;
            ep[{2, 0}] = 5;
            ep[{1, 3}] = 6;
            ep[{3, 4}] = 7;
            ep[{4, 0}] = 8;

            // We need to cut one edge in each backward path so result should be cutting smallest backward edges in each path
            ASSERT_EQ(u.minStCut(g, 1, 0, ep), 3 + 4 + 6);

            // We have only one edge in forward direction
            ASSERT_EQ(u.minStCut(g, 0, 1, ep), 2);
        }

        { // 'findIsolatedCycles'
            // Create test graph
            //       3 <-- 2
            //       |     ^
            //       |     |
            //       v     |
            //       0 --> 1
            Graph::Graph<VERTEX_TYPE> g;
            Graph::Fasp::GraphSpeedUtils<VERTEX_TYPE> u{5};
            for (int i = 0; i < 5; ++i) g.addVertex(i);
            g.addEdge({0, 1});
            g.addEdge({1, 2});
            g.addEdge({2, 3});
            g.addEdge({3, 0});
            Graph::Fasp::EdgesSet<VERTEX_TYPE> blueEdges;

            // Test function
            auto gg = g;
            auto hasIsoCycles = u.findIsolatedCycles(gg, {0, 1}, blueEdges, false);
            ASSERT_TRUE(hasIsoCycles);
            ASSERT_THAT(blueEdges, UnorderedElementsAre(Edge{1, 2}, Edge{2, 3}, Edge{3, 0}));

            // Add additional cycle to prevent finding isolated cycle for edge {0, 1}
            //          4          //
            //         ^ \         // Since {3, 2} has other cycle than only going through
            //        /   \        // {0, 1} then there is no isoleted cycle.
            //       /     v       //
            //       3 <-- 2
            //       |     ^
            //       |     |
            //       v     |
            //       0 --> 1
            g.addEdge({3, 4});
            g.addEdge({4, 2});

            // Test function
            gg = g;
            blueEdges.clear();
            hasIsoCycles = u.findIsolatedCycles(gg, {0, 1}, blueEdges, false);
            ASSERT_FALSE(hasIsoCycles);
            ASSERT_THAT(blueEdges, UnorderedElementsAre(Edge{1, 2}, Edge{3, 0}));

            // Check if blue edges are not updated when graph is weighted
            gg = g;
            blueEdges.clear();
            u.findIsolatedCycles(gg, {0, 1}, blueEdges, true);
            ASSERT_EQ(blueEdges.size(), 0);
        }

        { // 'getRandomSubgraph'

            // Generate complete graph
            Graph::Graph<VERTEX_TYPE> g;
            Graph::Fasp::GraphSpeedUtils<VERTEX_TYPE> u{5};
            for (int i = 0; i < 5; ++i) g.addVertex(i);
            for (VERTEX_TYPE s = 0; s < 5; ++s) {
                for (VERTEX_TYPE t = s + 1; t < 5; ++t) {
                    g.addEdge({s, t});
                    g.addEdge({t, s});
                }
            }
            Graph::Fasp::EdgesSet<VERTEX_TYPE> blueEdges;

            // ------------ First test - no blue edges provided
            // Complete graph with 5 vertices has 20 edges
            auto gg = g;
            ASSERT_EQ(gg.getNumOfEdges(), 20);

            // Remove 5 edges (should be always possible - FASP size for that complete graph is 10)
            u.getRandomSubgraph(gg, 5, blueEdges);
            ASSERT_EQ(gg.getNumOfEdges(), 15);

            // Try to remove more edges with cycles that actually are left in the graph
            // It should remove as many as possible but still one cycle should be there
            u.getRandomSubgraph(gg, 15, blueEdges);
            ASSERT_TRUE(gg.getNumOfEdges() > 0);
            ASSERT_FALSE(u.findEdgesWithCycles(gg).empty());

            // ------------ Second test - blue edges provided
            // Add to blueEdges all outgoing edges for vertices 0, 1, 2
            for (VERTEX_TYPE s = 0; s < 3; ++s) {
                for (VERTEX_TYPE t = 0; t < 5; ++t) {
                    if (s != t) blueEdges.emplace(Edge{s, t});
                }
            }
            // Complete graph with 5 vertices has 20 edges
            gg = g;
            ASSERT_EQ(gg.getNumOfEdges(), 20);

            // Remove 5 edges (should be always possible - FASP size for that complete graph is 10)
            u.getRandomSubgraph(gg, 5, blueEdges);
            ASSERT_EQ(gg.getNumOfEdges(), 15);

            // Try to remove more edges with cycles that actually are left in the graph
            // It should remove as many as possible but still one cycle should be there
            u.getRandomSubgraph(gg, 15, blueEdges);
            ASSERT_TRUE(gg.getNumOfEdges() > 0);
            ASSERT_FALSE(u.findEdgesWithCycles(gg).empty());
            // Check if all 'blue edges' are still in graph
            for (VERTEX_TYPE s = 0; s < 3; ++s) {
                for (VERTEX_TYPE t = 0; t < 5; ++t) {
                    if (s != t) {
                        ASSERT_TRUE(gg.hasEdge(Edge{s, t}));
                    }
                }
            }
        }

        { // 'getRedEdge'

            Graph::Graph<VERTEX_TYPE> g;
            Graph::Fasp::GraphSpeedUtils<VERTEX_TYPE> u{8};
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

            auto ep = Graph::Ext::getEdgeProperties(g, 1);
            ep[{5, 7}] = 2;
            ep[{7, 4}] = 3;
            ep[{3, 6}] = 4;
            ep[{6, 2}] = 5;

            typename Graph::Graph<VERTEX_TYPE>::Edges edges{};
            edges.emplace_back(Edge{0, 1});

            // From two candidates ({2, 3}, {4, 5}) the first one should be preferred since it has
            // higher min-cut
            auto [mc, e] = u.getRedEdge(g, ep, edges);
            ASSERT_EQ(mc, 5);
            ASSERT_EQ(e, (Edge{2, 3}));

            // Modified a little bit weights in graph to change preferred edge to {4, 5}
            ep[{5, 7}] = 6;
            ep[{7, 4}] = 7;

            auto [mc2, e2] = u.getRedEdge(g, ep, edges);
            ASSERT_EQ(mc2, 7);
            ASSERT_EQ(e2, Edge(4, 5) );
        }
    }

    TEST(TestGraphFasp, testIsoCut) {
        using VERTEX_TYPE = int;
        Graph::Graph<VERTEX_TYPE> g;
        Graph::Fasp::GraphSpeedUtils<VERTEX_TYPE> u{8};
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


        {   // easy case - no weights, 2 edges are exect solution
            // edges are not checked since different impl. of unordered_map can give different solutions
            auto gg{g};
            auto[edges, blueEdges, guessEdges] = Graph::Fasp::isoCut(gg, u);
            ASSERT_TRUE(guessEdges.empty());
            ASSERT_TRUE(blueEdges.empty());
            ASSERT_EQ(edges.size(), 2);
            ASSERT_TRUE(u.isAcyclic(gg));
        }
        {   // weights - not possible to use regular iso-cut and we expect guess (edge is not checked since
            // it may vary for different implementations of unordered_map).
            auto gg{g};
            auto ep = Graph::Ext::getEdgeProperties(g, 1);
            ep[{2, 3}] = 2;
            ep[{3, 6}] = 1;
            ep[{6, 2}] = 1;
            ep[{4, 5}] = 2;
            ep[{5, 7}] = 1;
            ep[{7, 4}] = 1;
            auto[edges, blueEdges, guessEdges] = Graph::Fasp::isoCut(gg, ep, u, true);
            ASSERT_FALSE(guessEdges.empty());
            ASSERT_TRUE(blueEdges.empty());
            ASSERT_TRUE(edges.empty());
            ASSERT_FALSE(u.isAcyclic(gg));
        }
    }

    TEST(TestGraphFasp, testTightCut) {
        using VERTEX_TYPE = int;

        Graph::Graph<VERTEX_TYPE> g;
        Graph::Fasp::GraphSpeedUtils<VERTEX_TYPE> u{8};
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
        {   // no weighted graph - should give soluttion FASP size/capacity = 2
            auto gg{g};
            auto [capacity, removedEdges, saEdgesCnt, saRndEdgesCnt, redRndEdgesCnt] = Graph::Fasp::tightCut<false>(gg);
            ASSERT_EQ(capacity, 2);
            ASSERT_EQ(removedEdges.size(), 2);
            ASSERT_EQ(saEdgesCnt, 2);
            ASSERT_EQ(saRndEdgesCnt, 0);
            ASSERT_EQ(redRndEdgesCnt, 0);
        }
        {   // weighted graph - should give soluttion FASP size/capacity = 3
            auto gg{g};
            auto ep = Graph::Ext::getEdgeProperties(g, 1);
            ep[{2, 3}] = 3;
            ep[{3, 6}] = 1;
            ep[{6, 2}] = 1;
            ep[{4, 5}] = 3;
            ep[{5, 7}] = 1;
            ep[{7, 4}] = 1;
            auto [capacity, removedEdges, saEdgesCnt, saRndEdgesCnt, redRndEdgesCnt] = Graph::Fasp::tightCut<true, true, VERTEX_TYPE, int>(gg, ep);
            ASSERT_EQ(capacity, 3);
            ASSERT_EQ(removedEdges.size(), 3);
            ASSERT_EQ(saEdgesCnt, 1);
            ASSERT_EQ(saRndEdgesCnt, 2);
            ASSERT_EQ(redRndEdgesCnt, 0);
        }
    }
}


int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
