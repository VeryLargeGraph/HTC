/*
 * From:
 * Lee, G., Ko, J., & Shin, K. (2020). Hypergraph motifs: Concepts, algorithms, and discoveries.
 * https://github.com/geonlee0325/MoCHy
 */

#include <fstream>
#include <iostream>
#include <random>
#include <vector>
#include <algorithm>
#include <set>
#include <map>
#include <ctime>
#include <unordered_map>
#include <unordered_set>
#include <chrono>

#include <omp.h>
#include <atomic>

using namespace std;

int id_to_index[128] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    21, 23, 22, 24, 23, 25, 24, 26,
    0, 0, 0, 0, 0, 0, 0, 0,
    21, 22, 23, 24, 23, 24, 25, 26,
    21, 23, 23, 25, 22, 24, 24, 26,
    27, 28, 28, 29, 28, 29, 29, 30,
    1, 2, 2, 3, 2, 3, 3, 4,
    5, 6, 6, 8, 7, 9, 9, 10,
    5, 7, 6, 9, 6, 9, 8, 10,
    11, 13, 12, 14, 13, 15, 14, 16,
    5, 6, 7, 9, 6, 8, 9, 10,
    11, 12, 13, 14, 13, 14, 15, 16,
    11, 13, 13, 15, 12, 14, 14, 16,
    17, 18, 18, 19, 18, 19, 19, 20
};

int get_motif_index_new(int deg_a, int deg_b, int deg_c, int C_ab, int C_bc, int C_ca, int g_abc) {
  int a = deg_a - (C_ab + C_ca) + g_abc;
  int b = deg_b - (C_bc + C_ab) + g_abc;
  int c = deg_c - (C_ca + C_bc) + g_abc;
  int d = C_ab - g_abc;
  int e = C_bc - g_abc;
  int f = C_ca - g_abc;
  int g = g_abc;
  int motif_id =
      (a > 0) + ((b > 0) << 1) + ((c > 0) << 2) + ((d > 0) << 3) + ((e > 0) << 4) + ((f > 0) << 5) + ((g > 0) << 6);
  return id_to_index[motif_id] - 1;
}

void read_data(string path,
               vector<vector<int> > &node2hyperedge,
               vector<vector<int> > &hyperedge2node,
               vector<unordered_set<int> > &hyperedge2node_set) {
  ifstream graphFile(path.c_str());
  string line;
  int num_hyperedges = 0;
  while (getline(graphFile, line)) {
    size_t pos = 0;
    unordered_set<int> tokens_set;
    vector<int> tokens;
    bool EOL = false;
    int idx;
    while (!EOL) {
      int pos = line.find(",");
      if (pos == string::npos) {
        pos = line.size();
        EOL = true;
        idx = stoi(line);
      } else {
        idx = stoi(line.substr(0, pos));
        line.erase(0, pos + 1);
      }
      while (idx >= node2hyperedge.size()) {
        node2hyperedge.push_back(vector<int>());
      }
      if (node2hyperedge[idx].empty() || node2hyperedge[idx].back() != num_hyperedges) {
        node2hyperedge[idx].push_back(num_hyperedges);
        tokens.push_back(idx);
        tokens_set.insert(idx);
      }
    }
    hyperedge2node.push_back(tokens);
    hyperedge2node_set.push_back(tokens_set);
    num_hyperedges++;
  }
  graphFile.close();
}

inline long long convert_id(int hyperedge_a, int hyperedge_b) {
  return hyperedge_a * (1LL << 31) + hyperedge_b;
}

int main(int argc, char *argv[]) {
  chrono::system_clock::time_point start;
  chrono::system_clock::time_point run_start;
  chrono::duration<double> dur;
  int progress;

  int num_threads = stoi(argv[1]);

  omp_set_num_threads(num_threads);

  string graphFile = (argc == 2) ? "dblp_graph.txt" : argv[2];

  // Read data
  start = chrono::system_clock::now();
  vector<vector<int> > node2hyperedge;
  vector<vector<int> > hyperedge2node;
  vector<unordered_set<int> > hyperedge2node_set;
  read_data(graphFile, node2hyperedge, hyperedge2node, hyperedge2node_set);

  int V = node2hyperedge.size(), E = hyperedge2node.size();
  cout << "# of nodes: " << V << '\n';
  cout << "# of hyperedges: " << E << '\n';
  dur = std::chrono::system_clock::now() - start;
  cout << "Reading data done: "
       << dur.count() << " sec" << endl;
  cout << "------------------------------------------" << endl << endl;


  // Make adjacency list
  start = chrono::system_clock::now();
  run_start = chrono::system_clock::now();
  hyperedge2node.resize(E);
  hyperedge2node_set.resize(E);
  vector<vector<pair<int, int> > > hyperedge_adj;
  vector<unordered_map<int, int> > hyperedge_inter;
  hyperedge_adj.resize(E);
  hyperedge_inter.resize(E);
  vector<vector<long long> > upd_time(num_threads);
  for (int i = 0; i < num_threads; i++)
    upd_time[i].resize(E, -1LL);

#pragma omp parallel for
  for (int hyperedge_a = 0; hyperedge_a < E; hyperedge_a++) {
    int tid = omp_get_thread_num();
    int deg_a = 0;
    long long l_hyperedge_a = (long long) hyperedge_a;
    for (const int &node: hyperedge2node[hyperedge_a]) {
      for (const int &hyperedge_b: node2hyperedge[node]) {
        if (hyperedge_b == hyperedge_a) continue;
        if ((upd_time[tid][hyperedge_b] >> 31) ^ hyperedge_a) {
          upd_time[tid][hyperedge_b] = (l_hyperedge_a << 31) + deg_a;
          deg_a++;
          hyperedge_adj[hyperedge_a].push_back({hyperedge_b, 0});
        }
        hyperedge_adj[hyperedge_a][(int) (upd_time[tid][hyperedge_b] & 0x7FFFFFFFLL)].second++;
      }
    }
  }

#pragma omp parallel for
  for (int hyperedge_a = 0; hyperedge_a < E; hyperedge_a++) {
    sort(hyperedge_adj[hyperedge_a].begin(), hyperedge_adj[hyperedge_a].end());
    int deg_a = hyperedge_adj[hyperedge_a].size();
    hyperedge_inter[hyperedge_a].rehash(deg_a);
    for (int i = 0; i < deg_a; i++) {
      int hyperedge_b = hyperedge_adj[hyperedge_a][i].first;
      int C_ab = hyperedge_adj[hyperedge_a][i].second;
      hyperedge_inter[hyperedge_a].insert({hyperedge_b, C_ab});
    }
  }

  dur = std::chrono::system_clock::now() - start;
  cout << "Adjacency list construction done: "
       << dur.count() << " sec" << endl;
  cout << "------------------------------------------" << endl << endl;


  // Exact hypergraph motif counting
  start = chrono::system_clock::now();
  vector<vector<long long> > h_motif(num_threads);
  vector<vector<int> > intersection(num_threads);

  for (int i = 0; i < num_threads; i++) {
    std::fill(upd_time[i].begin(), upd_time[i].end(), -1LL);
    intersection[i].resize(V);
    h_motif[i].resize(30, 0);
  }

  vector<std::atomic_flag> mutex(E);

#pragma omp parallel for
  for (int hyperedge_a = 0; hyperedge_a < E; hyperedge_a++) {
    int tid = omp_get_thread_num();

    long long l_hyperedge_a = (long long) hyperedge_a;
    int size_a = (int) hyperedge2node[hyperedge_a].size();
    int deg_a = (int) hyperedge_adj[hyperedge_a].size();

    for (int i = 0; i < deg_a; i++) {
      int hyperedge_b = hyperedge_adj[hyperedge_a][i].first, C_ab = hyperedge_adj[hyperedge_a][i].second;
      int size_b = (int) hyperedge2node[hyperedge_b].size();
      int deg_b = (int) hyperedge_adj[hyperedge_b].size();

      const auto &nodes = hyperedge2node_set[hyperedge_b];
      auto it_end = nodes.end();
      int cnt = 0;
      for (const int &node: hyperedge2node[hyperedge_a]) {
        if (nodes.find(node) != it_end)
          intersection[tid][cnt++] = node;
      }

      for (int j = i + 1; j < deg_a; j++) {
        int hyperedge_c = hyperedge_adj[hyperedge_a][j].first, C_ca = hyperedge_adj[hyperedge_a][j].second;
        int size_c = (int) hyperedge2node[hyperedge_c].size();
        int deg_c = (int) hyperedge_adj[hyperedge_c].size();

        int C_bc = 0;
        while (mutex[hyperedge_b].test_and_set(std::memory_order_acquire));
//        C_bc = hyperedge_inter[hyperedge_b][hyperedge_c];
        auto it = hyperedge_inter[hyperedge_b].find(hyperedge_c);
        if (it == hyperedge_inter[hyperedge_b].end()) C_bc = 0;
        else C_bc = (*it).second;
        mutex[hyperedge_b].clear(std::memory_order_release);
        if (C_bc) {
          if (hyperedge_a < hyperedge_b) {
            int g_abc = 0;
            const auto &nodes = hyperedge2node_set[hyperedge_c];
            auto it_end = nodes.end();
            for (int k = 0; k < C_ab; k++) { if (nodes.find(intersection[tid][k]) != it_end) g_abc++; }
            int motif_index = get_motif_index_new(size_a, size_b, size_c, C_ab, C_bc, C_ca, g_abc);
            h_motif[tid][motif_index]++;
          }
        } else {
          int motif_index = get_motif_index_new(size_a, size_b, size_c, C_ab, 0, C_ca, 0);
          h_motif[tid][motif_index]++;
        }
      }

    }
  }

  int index = 0;
  vector<long long> h_motif_final(30, 0);
  for (int i = 0; i < 30; i++) {
    for (int j = 0; j < num_threads; j++) h_motif_final[i] += h_motif[j][i];
    if (i == 0 || i == 1 || i == 4 || i == 6) continue;
    cout << fixed << "motif " << ++index << ": " << fixed << h_motif_final[i] << endl;
  }

  dur = std::chrono::system_clock::now() - run_start;
  double runtime = (double) dur.count();

  dur = std::chrono::system_clock::now() - start;
  cout << "\nMotif counting: "
       << dur.count() << " sec" << endl;
  cout << "Total runtime: " << runtime << endl;
  cout << "------------------------------------------" << endl << endl;

  node2hyperedge.clear();
  hyperedge2node.clear();
  hyperedge_adj.clear();
  hyperedge_inter.clear();

  return 0;
}