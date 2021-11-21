#include <bits/stdc++.h>
#include "hypergraph.hpp"
using namespace std;

//#define Debug
//#define PREPROCESS_DEGENERACY_ORDERING
//#define SKIP_INTERSECT_VERTEX
//#define USING_BITMAP
//#define PARALLEL

uint64_t dbg = 0;
double dTime_WL = 0, dTime_BC = 0;

#ifdef USING_BITMAP
using bitmap_t = uint32_t;
#endif

uint64_t triangle_count = 0;

int main(int argc, char *argv[]) {
  auto time_start = chrono::steady_clock::now();

  HyperGraph G;
  if (argc == 1) G.readFileEdgeString("dblp_graph.txt");
  else if (argc == 2) G.readFileEdgeString(argv[1]);
  else if (!G.readFileScHoLP(argv[1], argv[2])) {
    return -1;
  }

#ifdef Debug
  cout << "Read graph time: " << chrono::duration<double>{chrono::steady_clock::now() - time_start}.count() << " ; ";
#endif
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
#ifdef Debug
  auto time_core = chrono::steady_clock::now();
#endif
  while (true) {
    auto cntE = G.sizeE();
    auto cntV = G.sizeV();
    G.uniqueEdges();
    G.extract2Core();
    if (G.sizeV() == cntV && G.sizeE() == cntE) break;
  }
#ifdef Debug
  cout << "Core Graph time: " << chrono::duration<double>{chrono::steady_clock::now() - time_core}.count() << " ; ";
  cout << "Tri Core Graph: |V|=" << G.sizeV() << " |E|=" << G.sizeE() << endl;
  auto time_sort = chrono::steady_clock::now();
#endif
#ifdef PREPROCESS_DEGENERACY_ORDERING
  G.sortByDegeneracy();
#endif
  G.sortV();
#ifdef Debug
  cout << "Sort graph time: " << chrono::duration<double>{chrono::steady_clock::now() - time_sort}.count() << endl;
#endif

  auto time_run = chrono::steady_clock::now();

#ifdef PARALLEL
#pragma omp parallel reduction(+:triangle_count)
#endif
  {
#ifdef USING_BITMAP
    vector<bitmap_t> source(G.sizeE());
#else
    vector<vector<vid_t>> source(G.sizeE());
#endif
    vector<eid_t> worklist;
    vector<eid_t> timestamp(G.sizeE(), 0);
    eid_t stamp = 0;

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

#pragma omp for schedule(dynamic)
    for (eid_t a = 0; a < G.sizeE(); a++) {
#ifdef USING_BITMAP
      bitmap_t uip = 1;
#endif
#ifdef Debug
      auto time_WL = chrono::steady_clock::now();
#endif
      for (const auto &v:G.E[a]) {
        for (const auto &b:G.V[v]) {
          if (b >= a) break;
#ifdef USING_BITMAP
          if (source[b] == 0) worklist.emplace_back(b);
          source[b] |= uip;
#else
          if (source[b].empty()) worklist.emplace_back(b);
          source[b].emplace_back(v);
#endif
        }
#ifdef USING_BITMAP
        uip <<= 1;
#endif
      }
#ifdef Debug
      dTime_WL += chrono::duration<double>{chrono::steady_clock::now() - time_WL}.count();
      auto time_BC = chrono::steady_clock::now();
#endif
      for (const auto &b:worklist) {
        stamp++;
#ifdef SKIP_INTERSECT_VERTEX
        auto ait = G.E[a].begin();
#endif
        for (const auto &v:G.E[b]) {
#ifdef SKIP_INTERSECT_VERTEX
          while (ait != G.E[a].end() && *ait < v) ait++;
          if (ait != G.E[a].end() && *ait == v) continue;
#endif
          for (const auto &c:G.V[v]) {
            if (c >= b) break;
            if (
#ifdef USING_BITMAP
                    source[c] == 0
#else
                    source[c].empty()
#endif
                    || timestamp[c] == stamp)
              continue;
            timestamp[c] = stamp;
            if (
#ifdef USING_BITMAP
                (source[b] & source[c]) == 0
#else
                !vectorIntersect(source[b], source[c])
#endif
                )
              triangle_count += G.E[a].weight * G.E[b].weight * G.E[c].weight;
          }
        }
      }
#ifdef Debug
      dTime_BC += chrono::duration<double>{chrono::steady_clock::now() - time_BC}.count();
#endif
      for (const auto &w:worklist) {
#ifdef USING_BITMAP
        source[w] = 0;
#else
        source[w].clear();
#endif
      }
      worklist.clear();
    }
  }

#ifdef Debug
  cout << endl;
  cout << "DBG = " << dbg << endl;
  cout << "dTime_WL = " << dTime_WL << endl;
  cout << "dTime_BC = " << dTime_BC << endl;
#endif

  cout << endl << "Found Triangle Count = " << triangle_count << endl;

  cout << endl << "Counting time: " << chrono::duration<double>{chrono::steady_clock::now() - time_run}.count() << " ; "
       << "Total runtime: " << chrono::duration<double>{chrono::steady_clock::now() - time_start}.count() << endl;

  return 0;
}