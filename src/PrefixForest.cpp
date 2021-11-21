#include <bits/stdc++.h>
#include "hypergraph.hpp"
using namespace std;

//#define Debug
//#define DEBUG_V
//#define PREPROCESS_DEGENERACY_ORDERING
//#define PREPROCESS_ADJACENT_EDGE
//#define PROGRESSIVE_COUNTING
//#define USING_BITMAP
//#define PARALLEL
//#define PARALLEL_FINE_GRAINED_SIZE 64

uint64_t dbg = 0;
double dTime_WL = 0, dTime_BC = 0, dTime_BC0 = 0, dTime_BC1 = 0, dTime_BC00 = 0, dTime_BC01 = 0, dTime_jump = 0;

#ifdef USING_BITMAP
#define BITMAP_MAX_SIZE 64
using bitmap_t = uint64_t;
#endif

uint64_t triangle_count = 0;

struct TreeNode {
  vid_t vid;
  eid_t eid;
  eid_t minEid, maxEid;

#ifdef USING_BITMAP
  bitmap_t mask;
  vid_t height;
#endif
  vector<TreeNode *> next;
  unordered_map<vid_t, size_t> ump;
  TreeNode *father, *jump;

#ifdef PROGRESSIVE_COUNTING
  eid_t tc;
#endif
  size_t wl_begin, wl_end;

  TreeNode() = delete;
  TreeNode(vid_t v,
#ifdef USING_BITMAP
      const bitmap_t &mk,
#endif
           TreeNode *fa)
      : vid(v),
        eid(-1),
        minEid(-1),
        maxEid(0),
#ifdef USING_BITMAP
      mask(mk),
      height(0),
#endif
        father(fa),
        jump(nullptr),
#ifdef PROGRESSIVE_COUNTING
      tc(0),
#endif
        wl_begin(0),
        wl_end(0) {}
};
vector<TreeNode *> edge2node;

void buildTreeDFS(vector<eid_t> &edgeNid, TreeNode *node, TreeNode *ancestor) {
  static eid_t eNid = 0;
  sort(node->next.begin(), node->next.end(), [&](const auto &a, const auto &b) { return a->vid < b->vid; });
  if (node->eid != (eid_t) -1) {
    const auto t = node->eid;
    node->eid = edgeNid[t] = eNid++;
    edge2node[node->eid] = node;
    node->minEid = node->eid;
    node->maxEid = node->eid;
#ifdef USING_BITMAP
    node->height = 1;
#endif
  }
  node->jump = ancestor;
  if (node->next.empty()) return;
  for (size_t i = 0; i < node->next.size() - 1; i++)
    buildTreeDFS(edgeNid, node->next[i], node->next[i + 1]);
  buildTreeDFS(edgeNid, node->next[node->next.size() - 1], ancestor);
  node->minEid = min(node->minEid, node->next[0]->minEid);
  node->maxEid = max(node->maxEid, node->next[node->next.size() - 1]->maxEid);
#ifdef USING_BITMAP
  for (auto nxt:node->next) node->height = max(node->height, nxt->height + 1);
#endif
}

TreeNode *buildTree(HyperGraph &G) {
  auto root = new TreeNode(-1,
#ifdef USING_BITMAP
      1,
#endif
                           nullptr);
  root->father = root;
  for (eid_t eid = 0; eid < G.sizeE(); eid++) {
    const auto &e = G.E[eid];
    if (e.size() == 1) continue;
    auto node = root;
    for (const auto &v:e) {
      auto uit = node->ump.find(v);
      if (uit == node->ump.end()) {
        uit = (node->ump.emplace(v, node->next.size())).first;
        node->next.emplace_back(new TreeNode{v,
#ifdef USING_BITMAP
            (node == root) ? 1 : (node->mask << 1),
#endif
                                             node});
      }
      node = node->next[uit->second];
    }
    if (node->eid == (eid_t) -1) node->eid = eid;
    else G.E[node->eid].weight += G.E[eid].weight;
  }
  return root;
}

void edgeReorder(HyperGraph &G, TreeNode *root) {
  edge2node.resize(G.sizeE());
  vector<eid_t> edgeNid(G.sizeE(), -1);
  sort(root->next.begin(), root->next.end(), [&](const auto &a, const auto &b) { return a->vid < b->vid; });
  for (auto node:root->next) buildTreeDFS(edgeNid, node, root);
  root->minEid = 0;
  root->maxEid = root->next.back()->maxEid;
  vectorReorder(G.E, edgeNid);
  for (auto &v:G.V) {
    for (auto &e:v) e = edgeNid[e];
    sort(v.begin(), v.end());
  }
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
#ifdef Debug
  eid_t esz0 = 0, esz32 = 0, esz64 = 0;
  vid_t esm = 0;
  for (const auto &e:G.E) {
    esm = max(esm, e.size());
    if (e.size() > 64) esz64++;
    else if (e.size() > 32) esz32++;
    else if (e.size() > 0) esz0++;
  }
  cout << " HyperEdge size: (0,32]=" << esz0 << " | (32,64]=" << esz32 << " | (64," << esm << "]=" << esz64 << endl;
#endif

  auto time_preprocess = chrono::steady_clock::now();
  auto time_core = chrono::steady_clock::now();
  while (true) {
    auto cntE = G.sizeE();
    auto cntV = G.sizeV();
    G.uniqueEdges();
    G.extract2Core();
    if (G.sizeV() == cntV && G.sizeE() == cntE) break;
  }
  cout << "Core graph time: " << chrono::duration<double>{chrono::steady_clock::now() - time_core}.count() << " ; ";
  cout << "Core Graph: |V|=" << G.sizeV() << " |E|=" << G.sizeE() << endl;
  auto time_sort = chrono::steady_clock::now();
#ifdef PREPROCESS_DEGENERACY_ORDERING
  G.sortByDegeneracy();
#endif
  G.sortV();
  cout << "Sort graph time: " << chrono::duration<double>{chrono::steady_clock::now() - time_sort}.count() << endl;
  auto time_tree = chrono::steady_clock::now();
  auto root = buildTree(G);
  edgeReorder(G, root);
  cout << "Build tree time: " << chrono::duration<double>{chrono::steady_clock::now() - time_tree}.count() << endl;
#ifdef PREPROCESS_ADJACENT_EDGE
  auto time_adj = chrono::steady_clock::now();
  vector<unordered_set<eid_t>> adjacent(G.sizeE());
  for (eid_t e = 0; e < G.sizeE(); e++) {
    for (const auto &v:G.E[e]) {
      for (const auto &ee:G.V[v]) adjacent[e].emplace(ee);
    }
  }
  cout << "Calc adj time: " << chrono::duration<double>{chrono::steady_clock::now() - time_adj}.count() << endl;
#endif

#ifdef PROGRESSIVE_COUNTING
  auto check_intersection = [&](const eid_t &a, const eid_t &b) {
#ifdef PREPROCESS_ADJACENT_EDGE
    return adjacent[a].find(b) != adjacent[a].end();
#else
    return hyperedge_intersection(G.E[a], G.E[b]);
#endif
  };
#endif

  auto time_run = chrono::steady_clock::now();

#ifdef PARALLEL
#pragma omp parallel reduction(+:triangle_count)
#endif
  {
    vector<eid_t> worklist;
    worklist.reserve(G.sizeE());
#ifdef USING_BITMAP
    vector<bitmap_t> sourceBit(G.sizeE());
#endif
    vector<vector<vid_t>> source(G.sizeE());
#ifndef PROGRESSIVE_COUNTING
    vector<eid_t> timestamp(G.sizeE(), 0);
    eid_t stamp = 0;
#endif

#ifdef PARALLEL
#pragma omp barrier
#pragma omp single
#endif
    {
      cout << endl << "Preprocessing time: "
           << chrono::duration<double>{chrono::steady_clock::now() - time_preprocess}.count()
           << endl;
      time_run = chrono::steady_clock::now();
    }

#ifdef PARALLEL
#pragma omp for schedule(dynamic)
#endif
    for (auto node:root->next) {
      auto treeId = node->minEid;
      worklist.clear();
#ifdef USING_BITMAP
      if (node->height <= BITMAP_MAX_SIZE) {
        while (node != root) {
#ifdef Debug
          auto time_WL = chrono::steady_clock::now();
#endif
          node->wl_begin = worklist.size();
          for (const auto &e:G.V[node->vid]) {
            if (e > node->maxEid) break;
            if (sourceBit[e] == 0 && e < treeId) {
              worklist.emplace_back(e);
            }
            sourceBit[e] |= node->mask;
          }
          node->wl_end = worklist.size();
          const auto nxt = node->next.empty() ? node->jump : node->next[0];
#ifdef Debug
          dTime_WL += chrono::duration<double>{chrono::steady_clock::now() - time_WL}.count();
          auto time_BC = chrono::steady_clock::now();
          auto time_BC0 = chrono::steady_clock::now();
#endif
#ifdef PROGRESSIVE_COUNTING
          auto tc = node->father->tc;
          for (auto nb = node->father; nb != root; nb = nb->father) {
#if defined(PARALLEL) && defined(PARALLEL_FINE_GRAINED_SIZE)
#pragma omp taskloop default(shared) grainsize(PARALLEL_FINE_GRAINED_SIZE)
#endif
            for (auto wb = nb->wl_begin; wb != nb->wl_end; wb++) {
              const auto b = worklist[wb];
              decltype(tc) ttc = 0;
              if ((sourceBit[b] & node->mask) != 0) {
#ifdef DEBUG_V
                auto time_BC00 = chrono::steady_clock::now();
#endif
                for (auto nc = node->father; nc != nb; nc = nc->father) {
                  if ((sourceBit[b] & nc->mask) != 0) continue;
                  for (auto wc = nc->wl_begin; wc != nc->wl_end; wc++) {
                    const auto c = worklist[wc];
                    if ((sourceBit[b] & sourceBit[c]) == node->mask)
                      ttc -= G.E[b].weight * G.E[c].weight;
                  }
                }
#ifdef DEBUG_V
                dTime_BC00 += chrono::duration<double>{chrono::steady_clock::now() - time_BC00}.count();
#endif
              } else {
#ifdef DEBUG_V
                auto time_BC01 = chrono::steady_clock::now();
#endif
                for (auto wc = node->wl_begin; wc != node->wl_end; wc++) {
                  const auto c = worklist[wc];
                  if (check_intersection(b, c)) ttc += G.E[b].weight * G.E[c].weight;
                }
#ifdef DEBUG_V
                dTime_BC01 += chrono::duration<double>{chrono::steady_clock::now() - time_BC01}.count();
#endif
              }
#if defined(PARALLEL) && defined(PARALLEL_FINE_GRAINED_SIZE)
#pragma omp atomic
#endif
              tc += ttc;
            }
          }
          node->tc = tc;
#endif
#ifdef Debug
          dTime_BC0 += chrono::duration<double>{chrono::steady_clock::now() - time_BC0}.count();
          auto time_BC1 = chrono::steady_clock::now();
#endif
          if (node->eid != (eid_t) -1) {
#ifdef PROGRESSIVE_COUNTING
            triangle_count += G.E[node->eid].weight * node->tc;
#else
            for (auto nb = node; nb != root; nb = nb->father) {
#if defined(PARALLEL) && defined(PARALLEL_FINE_GRAINED_SIZE)
#pragma omp taskloop default(shared) grainsize(PARALLEL_FINE_GRAINED_SIZE)
#endif
              for (auto wb = nb->wl_begin; wb != nb->wl_end; wb++) {
                const auto b = worklist[wb];
                decltype(triangle_count) ttc = 0;
                stamp++;
                auto apt = node;
                for (auto bpt = edge2node[b]; bpt != root; bpt = bpt->father) {
                  while (apt != root && apt->vid > bpt->vid) apt = apt->father;
                  if (apt != root && apt->vid == bpt->vid) continue;
                  for (const auto &c:G.V[bpt->vid]) {
                    if (c >= b) break;
                    if (sourceBit[c] != 0 && timestamp[c] != stamp) {
                      timestamp[c] = stamp;
                      if ((sourceBit[b] & sourceBit[c]) == 0) {
                        ttc += G.E[node->eid].weight * G.E[b].weight * G.E[c].weight;
                      }
                    }
                  }
                }
#if defined(PARALLEL) && defined(PARALLEL_FINE_GRAINED_SIZE)
#pragma omp atomic
#endif
                triangle_count += ttc;
              }
            }
#endif
#if defined(PARALLEL) && defined(PARALLEL_FINE_GRAINED_SIZE)
#pragma omp taskloop default(shared) grainsize(PARALLEL_FINE_GRAINED_SIZE)
#endif
            for (auto b = treeId; b < node->eid; b++) {
              decltype(triangle_count) ttc = 0;
              auto apt = node;
              for (auto bpt = edge2node[b]; bpt != root; bpt = bpt->father) {
                while (apt->vid > bpt->vid) apt = apt->father;
                if (apt == bpt) break;
                if (apt->vid == bpt->vid) continue;
                for (auto wi = bpt->wl_begin; wi != bpt->wl_end; wi++) {
                  const auto c = worklist[wi];
                  if (sourceBit[c] != 0 && (sourceBit[b] & sourceBit[c]) == 0) {
                    ttc += G.E[node->eid].weight * G.E[b].weight * G.E[c].weight;
                  }
                }
              }
#if defined(PARALLEL) && defined(PARALLEL_FINE_GRAINED_SIZE)
#pragma omp atomic
#endif
              triangle_count += ttc;
            }
          }
#ifdef Debug
          dTime_BC1 += chrono::duration<double>{chrono::steady_clock::now() - time_BC1}.count();
        dTime_BC += chrono::duration<double>{chrono::steady_clock::now() - time_BC}.count();
        auto time_jump = chrono::steady_clock::now();
#endif
          if (node->next.empty()) {
#ifdef USING_BITMAP
            bitmap_t clr = 0;
            for (auto n = nxt->father; n != root; n = n->father) clr |= n->mask;
#endif
            for (auto n = node; n != nxt->father; n = n->father) {
              for (auto wi = n->wl_begin; wi != n->wl_end; wi++) {
                sourceBit[worklist[wi]] = 0;
              }
            }
            for (auto n = nxt->father; n != root; n = n->father)
              for (auto wi = n->wl_begin; wi != n->wl_end; wi++) {
                sourceBit[worklist[wi]] &= clr;
              }
            if (nxt == root)
              for (auto e = treeId; e <= node->maxEid; e++) {
                sourceBit[e] = 0;
              }
            else
              for (auto e = treeId; e < nxt->minEid; e++) {
                sourceBit[e] &= clr;
              }
          }
          node = nxt;
#ifdef Debug
          dTime_jump += chrono::duration<double>{chrono::steady_clock::now() - time_jump}.count();
#endif
        }
        continue;
      }
#endif
      while (node != root) {
#ifdef Debug
        auto time_WL = chrono::steady_clock::now();
#endif
        node->wl_begin = worklist.size();
        for (const auto &e:G.V[node->vid]) {
          if (e > node->maxEid) break;
          if (source[e].empty() && e < treeId) {
            worklist.emplace_back(e);
          }
          source[e].emplace_back(node->vid);
        }
        node->wl_end = worklist.size();
        const auto nxt = node->next.empty() ? node->jump : node->next[0];
#ifdef Debug
        dTime_WL += chrono::duration<double>{chrono::steady_clock::now() - time_WL}.count();
        auto time_BC = chrono::steady_clock::now();
        auto time_BC0 = chrono::steady_clock::now();
#endif
#ifdef PROGRESSIVE_COUNTING
        auto tc = node->father->tc;
        for (auto nb = node->father; nb != root; nb = nb->father) {
#if defined(PARALLEL) && defined(PARALLEL_FINE_GRAINED_SIZE)
#pragma omp taskloop default(shared) grainsize(PARALLEL_FINE_GRAINED_SIZE)
#endif
          for (auto wb = nb->wl_begin; wb != nb->wl_end; wb++) {
            const auto b = worklist[wb];
            decltype(tc) ttc = 0;
            if (source[b].back() == node->vid) {
#ifdef DEBUG_V
              auto time_BC00 = chrono::steady_clock::now();
#endif
              auto sbi = source[b].rbegin();
              for (auto nc = node->father; nc != nb; nc = nc->father) {
                while (*sbi > nc->vid) sbi++;
                if (*sbi == nc->vid) continue;
                for (auto wc = nc->wl_begin; wc != nc->wl_end; wc++) {
                  const auto c = worklist[wc];
                  if (vectorIntersectFirst(source[b], source[c]) == node->vid)
                    ttc -= G.E[b].weight * G.E[c].weight;
                }
              }
#ifdef DEBUG_V
              dTime_BC00 += chrono::duration<double>{chrono::steady_clock::now() - time_BC00}.count();
#endif
            } else {
#ifdef DEBUG_V
              auto time_BC01 = chrono::steady_clock::now();
#endif
              for (auto wc = node->wl_begin; wc != node->wl_end; wc++) {
                const auto c = worklist[wc];
                if (check_intersection(b, c)) ttc += G.E[b].weight * G.E[c].weight;
              }
#ifdef DEBUG_V
              dTime_BC01 += chrono::duration<double>{chrono::steady_clock::now() - time_BC01}.count();
#endif
            }
#if defined(PARALLEL) && defined(PARALLEL_FINE_GRAINED_SIZE)
#pragma omp atomic
#endif
            tc += ttc;
          }
        }
        node->tc = tc;
#endif
#ifdef Debug
        dTime_BC0 += chrono::duration<double>{chrono::steady_clock::now() - time_BC0}.count();
        auto time_BC1 = chrono::steady_clock::now();
#endif
        if (node->eid != (eid_t) -1) {
#ifdef PROGRESSIVE_COUNTING
          triangle_count += G.E[node->eid].weight * node->tc;
#else
          for (auto nb = node; nb != root; nb = nb->father) {
#if defined(PARALLEL) && defined(PARALLEL_FINE_GRAINED_SIZE)
#pragma omp taskloop default(shared) grainsize(PARALLEL_FINE_GRAINED_SIZE)
#endif
            for (auto wb = nb->wl_begin; wb != nb->wl_end; wb++) {
              const auto b = worklist[wb];
              decltype(triangle_count) ttc = 0;
              stamp++;
              auto apt = node;
              for (auto bpt = edge2node[b]; bpt != root; bpt = bpt->father) {
                while (apt != root && apt->vid > bpt->vid) apt = apt->father;
                if (apt != root && apt->vid == bpt->vid) continue;
                for (const auto &c:G.V[bpt->vid]) {
                  if (c >= b) break;
                  if (!source[c].empty() && timestamp[c] != stamp) {
                    timestamp[c] = stamp;
                    if (!vectorIntersect(source[b], source[c])) {
                      ttc += G.E[node->eid].weight * G.E[b].weight * G.E[c].weight;
                    }
                  }
                }
              }
#if defined(PARALLEL) && defined(PARALLEL_FINE_GRAINED_SIZE)
#pragma omp atomic
#endif
              triangle_count += ttc;
            }
          }
#endif
#if defined(PARALLEL) && defined(PARALLEL_FINE_GRAINED_SIZE)
#pragma omp taskloop default(shared) grainsize(PARALLEL_FINE_GRAINED_SIZE)
#endif
          for (auto b = treeId; b < node->eid; b++) {
            decltype(triangle_count) ttc = 0;
            auto apt = node;
            for (auto bpt = edge2node[b]; bpt != root; bpt = bpt->father) {
              while (apt->vid > bpt->vid) apt = apt->father;
              if (apt == bpt) break;
              if (apt->vid == bpt->vid) continue;
              for (auto wi = bpt->wl_begin; wi != bpt->wl_end; wi++) {
                const auto c = worklist[wi];
                if (!source[c].empty() && !vectorIntersect(source[b], source[c])) {
                  ttc += G.E[node->eid].weight * G.E[b].weight * G.E[c].weight;
                }
              }
            }
#if defined(PARALLEL) && defined(PARALLEL_FINE_GRAINED_SIZE)
#pragma omp atomic
#endif
            triangle_count += ttc;
          }
        }
#ifdef Debug
        dTime_BC1 += chrono::duration<double>{chrono::steady_clock::now() - time_BC1}.count();
        dTime_BC += chrono::duration<double>{chrono::steady_clock::now() - time_BC}.count();
        auto time_jump = chrono::steady_clock::now();
#endif
        if (node->next.empty()) {
#ifdef USING_BITMAP
          bitmap_t clr = 0;
          for (auto n = nxt->father; n != root; n = n->father) clr |= n->mask;
#endif
          for (auto n = node; n != nxt->father; n = n->father) {
            for (auto wi = n->wl_begin; wi != n->wl_end; wi++) {
              source[worklist[wi]].clear();
            }
          }
          for (auto n = nxt->father; n != root; n = n->father)
            for (auto wi = n->wl_begin; wi != n->wl_end; wi++) {
              while (source[worklist[wi]].back() > nxt->father->vid) source[worklist[wi]].pop_back();
            }
          if (nxt == root)
            for (auto e = treeId; e <= node->maxEid; e++) {
              source[e].clear();
            }
          else
            for (auto e = treeId; e < nxt->minEid; e++) {
              while (source[e].back() > nxt->father->vid) source[e].pop_back();
            }
        }
        node = nxt;
#ifdef Debug
        dTime_jump += chrono::duration<double>{chrono::steady_clock::now() - time_jump}.count();
#endif
      }
    }
  }

#ifdef Debug
  cout << endl;
  cout << "DBG = " << dbg << endl;
  cout << "dTime_WL = " << dTime_WL << endl;
  cout << "dTime_BC = " << dTime_BC << endl;
  cout << "  dTime_BC0 = " << dTime_BC0 << "  " << dTime_BC00 << " " << dTime_BC01 << endl;
  cout << "  dTime_BC1 = " << dTime_BC1 << endl;
  cout << "dTime_jump = " << dTime_jump << endl;
#endif

  cout << endl << "Found Triangle Count = " << triangle_count << endl;

  cout << endl << "Counting time: " << chrono::duration<double>{chrono::steady_clock::now() - time_run}.count() << " ; "
       << "Total runtime: " << chrono::duration<double>{chrono::steady_clock::now() - time_start}.count() << endl;

  return 0;
}