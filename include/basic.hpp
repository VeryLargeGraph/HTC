#ifndef BASIC_HPP_
#define BASIC_HPP_

#define vid_t uint32_t
#define eid_t uint64_t

template<typename T, typename ST>
size_t vectorReorder(std::vector<T> &vec, const std::vector<ST> &new_pos) {
  assert(vec.size() == new_pos.size());
  std::vector<T> res(vec.size());
  size_t pos_count = 0;
  for (size_t i = 0; i < vec.size(); i++)
    if (new_pos[i] != (ST) -1) {
      res[new_pos[i]] = std::move(vec[i]);
      pos_count++;
    }
  std::swap(vec, res);
  return pos_count;
}

template<typename T>
inline bool vectorIntersect(const std::vector<T> &a, const std::vector<T> &b) {
  for (auto ita = a.begin(), itb = b.begin();;) {
    while (ita != a.end() && *ita < *itb) ita++;
    if (ita == a.end()) return false;
    while (itb != b.end() && *itb < *ita) itb++;
    if (itb == b.end()) return false;
    if (*ita == *itb) return true;
  }
}

template<typename T>
inline T vectorIntersectFirst(const std::vector<T> &a, const std::vector<T> &b) {
  for (auto ita = a.begin(), itb = b.begin();;) {
    while (ita != a.end() && *ita < *itb) ita++;
    if (ita == a.end()) return -1;
    while (itb != b.end() && *itb < *ita) itb++;
    if (itb == b.end()) return -1;
    if (*ita == *itb) return *ita;
  }
}

#endif //BASIC_HPP_