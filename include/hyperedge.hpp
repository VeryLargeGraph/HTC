#ifndef HYPEREDGE_HPP_
#define HYPEREDGE_HPP_

#include <vector>
#include <algorithm>
#include "basic.hpp"

class HyperEdge {
 public:
  eid_t weight = 1;

  HyperEdge() = default;
  explicit HyperEdge(const std::string &data);
  HyperEdge(std::ifstream &is, const int &sz);
  bool operator<(const HyperEdge &b) const;
  bool operator==(const HyperEdge &b) const;
  friend std::ostream &operator<<(std::ostream &os, const HyperEdge &e);
  friend bool hyperedge_intersection(const HyperEdge &a, const HyperEdge &b);
  std::string to_string() const;
  vid_t size() const;
  void resize(size_t new_size);
  void shrink_to_fit();
  vid_t &operator[](size_t index);
  const vid_t &operator[](size_t index) const;
  auto begin() { return vtx.begin(); }
  auto begin() const { return vtx.begin(); }
  auto end() { return vtx.end(); }
  auto end() const { return vtx.end(); }

 private:
  std::vector<vid_t> vtx;
};

HyperEdge::HyperEdge(const std::string &data) {
  vid_t v = 0;
  for (const char &c : data) {
    if (!isdigit(c)) {
      vtx.emplace_back(v);
      v = 0;
    } else v = v * 10 + (c - '0');
  }
  if (isdigit(data.back())) vtx.emplace_back(v);
  std::sort(vtx.begin(), vtx.end());
  vtx.shrink_to_fit();
}

HyperEdge::HyperEdge(std::ifstream &is, const int &sz) {
  vtx.resize(sz);
  for (int i = 0; i < sz; i++) is >> vtx[i];
  std::sort(vtx.begin(), vtx.end());
  vtx.erase(std::unique(vtx.begin(), vtx.end()), vtx.end());
  vtx.shrink_to_fit();
}

std::ostream &operator<<(std::ostream &os, const HyperEdge &e) {
  os << "{";
  for (size_t i = 0; i < e.vtx.size() - 1; i++) os << e.vtx[i] << ",";
  os << e.vtx[e.vtx.size() - 1] << "}";
  return os;
}

std::string HyperEdge::to_string() const {
  std::string res;
  for (size_t i = 0; i < vtx.size() - 1; i++) res += std::to_string(vtx[i]) + ",";
  res += std::to_string(vtx[vtx.size() - 1]);
  return res;
}

vid_t HyperEdge::size() const {
  return vtx.size();
}

vid_t &HyperEdge::operator[](size_t index) {
  return vtx[index];
}

const vid_t &HyperEdge::operator[](size_t index) const {
  return vtx[index];
}

void HyperEdge::resize(size_t new_size) {
  vtx.resize(new_size);
}

void HyperEdge::shrink_to_fit() {
  vtx.shrink_to_fit();
}

bool hyperedge_intersection(const HyperEdge &a, const HyperEdge &b) {
  for (auto ait = a.begin(), bit = b.begin();;) {
    while (ait != a.end() && *ait < *bit) ait++;
    if (ait == a.end()) break;
    while (bit != b.end() && *bit < *ait) bit++;
    if (bit == b.end()) break;
    if (*ait == *bit) return true;
  }
  return false;
}

bool HyperEdge::operator<(const HyperEdge &b) const {
  if (size() != b.size()) return size() < b.size();
  for (auto it = begin(), bit = b.begin(); it != end(); it++, bit++) {
    if (*it != *bit) return *it < *bit;
  }
  return false;
}
bool HyperEdge::operator==(const HyperEdge &b) const {
  if (size() != b.size()) return false;
  for (auto it = begin(), bit = b.begin(); it != end(); it++, bit++) {
    if (*it != *bit) return false;
  }
  return true;
}

#endif //HYPEREDGE_HPP_