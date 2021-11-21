#include <bits/stdc++.h>
#include "../include/hypergraph.hpp"
using namespace std;

struct TreeNode {
  vid_t vid;
  eid_t eid;

  vector<TreeNode *> next;
  TreeNode *father, *jump;

  TreeNode() = delete;
  TreeNode(vid_t v, TreeNode *fa)
      : vid(v),
        eid(-1),
        father(fa),
        jump(nullptr) {}
};

TreeNode *buildTree(HyperGraph &G) {
  unordered_map<TreeNode *, unordered_map<vid_t, size_t>> ump;
  auto root = new TreeNode(-1, nullptr);
  root->father = root;
  for (eid_t eid = 0; eid < G.sizeE(); eid++) {
    const auto &e = G.E[eid];
    if (e.size() == 1) continue;
    auto node = root;
    for (const auto &v:e) {
      auto uit = ump[node].find(v);
      if (uit == ump[node].end()) {
        uit = (ump[node].emplace(v, node->next.size())).first;
        node->next.emplace_back(new TreeNode{v, node});
      }
      node = node->next[uit->second];
    }
    if (node->eid == (eid_t) -1) node->eid = eid;
    else G.E[node->eid].weight += G.E[eid].weight;
  }
  return root;
}

void debugDFS(size_t &cnt, size_t &cnt_e, size_t &cnt_e_n, const TreeNode *node) {
  cnt++;
  if (node->eid != (eid_t) -1) {
    cnt_e++;
    cnt_e_n++;
  } else if (node->next.size() > 1) {
    cnt_e_n++;
  }
  for (const auto &nxt:node->next) debugDFS(cnt, cnt_e, cnt_e_n, nxt);
}
void debugInfo(const HyperGraph &G, const TreeNode *root) {
  size_t sumDegree = 0;
  for (const auto &e:G.E) sumDegree += e.size();
  size_t nodeCount = 0, nodeCount_eid = 0, nodeCount_eid_nxt = 0;
  for (auto node:root->next) debugDFS(nodeCount, nodeCount_eid, nodeCount_eid_nxt, node);
  cout << endl << "DEBUG_INFO: |sumDegree|=" << sumDegree << " -> |TreeNode|=" << nodeCount << endl
       << "DEBUG_INFO: |Forest|=" << root->next.size() << " |Node_E|=" << nodeCount_eid << " |Node_E+N|="
       << nodeCount_eid_nxt << endl;
}

int main(int argc, char *argv[]) {
  auto time_start = chrono::steady_clock::now();

  HyperGraph G;
  if (argc == 1) G.readFileEdgeString("dblp_graph.txt");
  else if (argc == 2) G.readFileEdgeString(argv[1]);
  else if (!G.readFileScHoLP(argv[1], argv[2])) {
    return -1;
  }
  cout << "Loading time: " << chrono::duration<double>{chrono::steady_clock::now() - time_start}.count() << " ; ";
  cout << "Original Graph: |V|=" << G.sizeV() << " |E|=" << G.sizeE() << endl;

  eid_t esz0 = 0, esz32 = 0, esz64 = 0;
  vid_t esm = 0;
  for (const auto &e:G.E) {
    esm = max(esm, e.size());
    if (e.size() > 64) esz64++;
    else if (e.size() > 32) esz32++;
    else if (e.size() > 0) esz0++;
  }
  cout << " HyperEdge size: (0,32]=" << esz0 << " | (32,64]=" << esz32 << " | (64," << esm << "]=" << esz64 << endl;

  if (argc > 2) {
    ofstream outFile(string(argv[2]) + ".dat.txt");
    for (const auto &e:G.E) outFile << e.to_string() << endl;
    outFile.close();
  }

  for (size_t tim = 1;; tim++) {
    auto cntE = G.sizeE();
    auto cntV = G.sizeV();
    G.uniqueEdges();
    cout << "> uniq-" << tim << " : |V|=" << G.sizeV() << " |E|=" << G.sizeE() << endl;
    G.extract2Core();
    cout << "> core-" << tim << " : |V|=" << G.sizeV() << " |E|=" << G.sizeE() << endl;
    if (G.sizeV() == cntV && G.sizeE() == cntE) break;
  }
  cout << "2-2 Core Graph: |V|=" << G.sizeV() << " |E|=" << G.sizeE() << endl;
  G.sortV();
  cout << "> Top5DegreeV=" << G.V[0].size() << "=" << G.V[1].size() << "=" << G.V[2].size() << "=" << G.V[3].size()
       << "=" << G.V[4].size() << endl;
  eid_t maxAppAdj = 0;
  for (eid_t e = 0; e < G.sizeE(); e++) {
    eid_t appAdj = 0;
    for (const auto &v:G.E[e]) appAdj += G.V[v].size();
    maxAppAdj = max(maxAppAdj, appAdj);
  }
  cout << "> maxApproximateAdjacentE=" << maxAppAdj << endl;
  auto root = buildTree(G);
  debugInfo(G, root);

  cout << endl << "Total runtime: " << chrono::duration<double>{chrono::steady_clock::now() - time_start}.count()
       << endl;

  return 0;
}