#ifndef HYPERGRAPH_HPP_
#define HYPERGRAPH_HPP_

#include <fstream>
#include <sstream>
#include "hyperedge.hpp"

class HyperGraph {
 public:
  std::vector<std::vector<eid_t>> V;
  std::vector<HyperEdge> E;

  vid_t sizeV() const;
  eid_t sizeE() const;
  bool readFileEdgeString(const std::string &path);
  bool readFileScHoLP(const std::string &folder, const std::string &name);
  void extract2Core();
  eid_t sortByDegree();
  eid_t sortByDegeneracy();
  vid_t sortV();
  void uniqueEdges();
};

vid_t HyperGraph::sizeV() const {
  return V.size();
}

eid_t HyperGraph::sizeE() const {
  return E.size();
}

bool HyperGraph::readFileEdgeString(const std::string &path) {
  std::ifstream file(path);
  if (!file.is_open()) return false;
  std::string line;
  while (std::getline(file, line)) {
    auto eid = E.size();
    E.emplace_back(line);
    for (const auto &vtx:E.back()) {
      if (vtx >= V.size()) V.resize(vtx + 1);
      if (V[vtx].empty() || V[vtx].back() != eid) V[vtx].emplace_back(eid);
    }
  }
  return true;
}
bool HyperGraph::readFileScHoLP(const std::string &folder, const std::string &name) {
  std::ifstream nverts(folder + "/" + name + "-nverts.txt");
  std::ifstream simplices(folder + "/" + name + "-simplices.txt");
  if (!nverts.is_open() || !simplices.is_open()) return false;
  int sz;
  while (nverts >> sz) {
    auto eid = E.size();
    E.emplace_back(simplices, sz);
    for (const auto &vtx:E.back()) {
      if (vtx >= V.size()) V.resize(vtx + 1);
      if (V[vtx].empty() || V[vtx].back() != eid) V[vtx].emplace_back(eid);
    }
  }
  return true;
}

eid_t HyperGraph::sortByDegree() {
  std::vector<bool> used(sizeE());
  std::vector<std::pair<eid_t, eid_t>> deg(sizeE());
  for (eid_t e = 0; e < sizeE(); e++) {
    deg[e] = {0, e};
    for (const auto &v:E[e]) deg[e].first += V[v].size() - 1;
  }
  std::sort(deg.begin(), deg.end(), std::greater<>());
  std::vector<eid_t> newId(sizeE());
  for (eid_t e = 0; e < sizeE(); e++) {
    newId[deg[e].second] = e;
  }
  for (eid_t e = 0; e < sizeE(); e++) {
    if (!used[e]) {
      auto te = std::move(E[e]);
      auto t = e;
      while (!used[t]) {
        used[t] = true;
        std::swap(te, E[newId[t]]);
        t = newId[t];
      }
    }
  }
  for (vid_t v = 0; v < sizeV(); v++) {
    for (auto &e:V[v]) e = newId[e];
    std::sort(V[v].begin(), V[v].end());
  }
  return deg.back().first;
}

eid_t HyperGraph::sortByDegeneracy() {
  std::vector<bool> used(sizeE());
  std::vector<eid_t> deg(sizeE());
  eid_t maxDegree = 0;
  for (eid_t e = 0; e < sizeE(); e++) {
    used[e] = false;
    deg[e] = 0;
    for (const auto &v:E[e]) deg[e] += V[v].size() - 1;
    maxDegree = std::max(maxDegree, deg[e]);
  }
  std::vector<std::list<vid_t>> degList(maxDegree + 1);
  std::vector<std::list<vid_t>::iterator> degListIter(sizeE());
  for (eid_t e = 0; e < sizeE(); e++) {
    auto &l = degList[deg[e]];
    l.emplace_front(e);
    degListIter[e] = l.begin();
  }
  eid_t degeneracy = 0;
  std::vector<eid_t> newId(sizeE());
  for (eid_t i = 0, id = 0; i <= maxDegree;) {
    auto &l = degList[i];
    if (l.empty()) {
      i++;
      continue;
    }
    auto e = l.front();
    l.pop_front();
    newId[e] = id++;
    used[e] = true;
    degeneracy = std::max(degeneracy, deg[e]);
    for (const auto &v:E[e]) {
      for (const auto &ee:V[v]) {
        if (ee != e && !used[ee]) {
          auto &d = deg[ee];
          degList[d - 1].splice(degList[d - 1].begin(), degList[d], degListIter[ee]);
          d--;
          i = std::min(i, d);
        }
      }
    }
  }
  for (eid_t e = 0; e < sizeE(); e++) {
    if (used[e]) {
      auto te = std::move(E[e]);
      auto t = e;
      while (used[t]) {
        used[t] = false;
        std::swap(te, E[newId[t]]);
        t = newId[t];
      }
    }
  }
  for (vid_t v = 0; v < sizeV(); v++) {
    for (auto &e:V[v]) e = newId[e];
    std::sort(V[v].begin(), V[v].end());
  }
  return degeneracy;
}
void HyperGraph::extract2Core() {
  std::vector<bool> removedE(sizeE(), false), removedV(sizeV(), false);
  std::vector<vid_t> degE(sizeE(), 0);
  std::vector<eid_t> degV(sizeV(), 0);
  std::queue<vid_t> qV;
  std::queue<eid_t> qE;
  for (vid_t v = 0; v < sizeV(); v++) {
    degV[v] = V[v].size();
    if (degV[v] == 1) qV.emplace(v);
  }
  for (eid_t e = 0; e < sizeE(); e++) {
    degE[e] = E[e].size();
    if (degE[e] == 1) qE.emplace(e);
  }
  while (!qV.empty() || !qE.empty()) {
    if (!qV.empty()) {
      auto v = qV.front();
      qV.pop();
      removedV[v] = true;
      for (const auto &e:V[v]) {
        degE[e]--;
        if (degE[e] == 1) qE.emplace(e);
      }
    }
    if (!qE.empty()) {
      auto e = qE.front();
      qE.pop();
      removedE[e] = true;
      for (const auto &v:E[e]) {
        degV[v]--;
        if (degV[v] == 1) qV.emplace(v);
      }
    }
  }
  size_t sze = 0;
  for (eid_t e = 0; e < sizeE(); e++)
    if (!removedE[e]) {
      if (sze != e) E[sze] = std::move(E[e]);
      sze++;
    }
  E.resize(sze);
  std::vector<vid_t> idv(sizeV());
  size_t szv = 0;
  for (vid_t v = 0; v < sizeV(); v++) idv[v] = (removedV[v]) ? -1 : (szv++);
  V.resize(szv);
  for (auto &e:E) {
    size_t sz = 0;
    for (size_t i = 0; i < e.size(); i++)if (!removedV[e[i]]) e[sz++] = idv[e[i]];
    e.resize(sz);
    e.shrink_to_fit();
    std::sort(e.begin(), e.end());
  }
  for (auto &v:V) v.clear();
  for (eid_t e = 0; e < sizeE(); e++) for (auto &v:E[e]) V[v].emplace_back(e);
  for (auto &v:V) std::sort(v.begin(), v.end());
}
vid_t HyperGraph::sortV() {
  std::vector<eid_t> vid(sizeV()), nid(sizeV());
  std::iota(vid.begin(), vid.end(), 0);
  std::sort(vid.begin(), vid.end(), [&](const vid_t &a, const vid_t &b) { return V[a].size() > V[b].size(); });
  for (vid_t vi = 0; vi < sizeV(); vi++) nid[vid[vi]] = vi;
  for (vid_t v = 0; v < sizeV(); v++) {
    if (vid[v] != sizeV()) {
      auto tv = std::move(V[v]);
      auto t = v;
      while (vid[t] != sizeV()) {
        vid[t] = sizeV();
        std::swap(tv, V[nid[t]]);
        t = nid[t];
      }
    }
  }
  for (auto &e:E) {
    for (auto &v:e) v = nid[v];
    std::sort(e.begin(), e.end());
  }
  return V[0].size();
}

void HyperGraph::uniqueEdges() {
  for (auto &v:V) sort(v.begin(), v.end());
  for (auto &e:E) sort(e.begin(), e.end());
  std::sort(E.begin(), E.end());
  size_t nsz = 0;
  for (size_t i = 1; i < sizeE(); i++) {
    if (E[i] == E[nsz]) {
      E[nsz].weight += E[i].weight;
    } else {
      E[++nsz] = E[i];
    }
  }
  E.resize(nsz + 1);
  E.shrink_to_fit();
  for (size_t s = 1; s < E.size(); s++) assert(!(E[s - 1] == E[s]));
  for (auto &v:V) v.clear();
  for (eid_t e = 0; e < sizeE(); e++) {
    for (const auto &v:E[e]) V[v].emplace_back(e);
  }
  for (auto &v:V) {
    std::sort(v.begin(), v.end());
    v.shrink_to_fit();
  }
}

#endif //HYPERGRAPH_HPP_