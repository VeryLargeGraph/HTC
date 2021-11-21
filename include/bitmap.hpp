#ifndef BITMAP_HPP_
#define BITMAP_HPP_

#include <vector>

class Bitmap {
 public:
  Bitmap() = default;
  Bitmap(unsigned int init);
  explicit operator bool() const;
  Bitmap operator-(unsigned int one) const;
  Bitmap operator<<(unsigned int shift) const;
  Bitmap operator&(const Bitmap &bm) const;
  bool operator>(const Bitmap &bm) const;
  bool operator==(unsigned int zero) const;
  bool operator!=(unsigned int zero) const;
  bool operator==(const Bitmap &bm) const;
  Bitmap &operator&=(const Bitmap &bm);
  Bitmap &operator|=(const Bitmap &bm);

 private:
  std::vector<uint64_t> arr{};
};
Bitmap::Bitmap(unsigned int init) {
  assert(init == 0 || init == 1);
  arr.emplace_back(init);
}
Bitmap::operator bool() const {
  for (const auto &a:arr) if (a) return true;
  return false;
}
Bitmap Bitmap::operator-(unsigned int one) const {
  assert(one == 1);
  Bitmap ans;
  for (const auto &a:arr) {
    ans.arr.emplace_back(a - 1);
  }
  if (ans.arr.back() == 0) ans.arr.pop_back();
  return ans;
}
Bitmap Bitmap::operator<<(unsigned int shift) const {
  Bitmap ans;
  uint64_t t = 0;
  for (const auto &a:arr) {
    auto b = a >> (64 - shift);
    ans.arr.emplace_back(a << shift | t);
    t = b;
  }
  if (t != 0) ans.arr.emplace_back(t);
  return ans;
}
Bitmap Bitmap::operator&(const Bitmap &bm) const {
  Bitmap ans;
  const auto minSize = std::min(arr.size(), bm.arr.size());
  ans.arr.resize(minSize);
  for (size_t i = 0; i < minSize; i++) ans.arr[i] = arr[i] & bm.arr[i];
  return ans;
}
bool Bitmap::operator>(const Bitmap &bm) const {
  const auto minSize = std::min(arr.size(), bm.arr.size());
  for (size_t i = arr.size() - 1; i >= minSize; i--) if (arr[i] != 0) return true;
  for (size_t i = bm.arr.size() - 1; i >= minSize; i--) if (bm.arr[i] != 0) return false;
  for (int i = minSize - 1; i >= 0; i--) {
    if (arr[i] > bm.arr[i]) return true;
    else if (bm.arr[i] > arr[i]) return false;
  }
  return false;
}
bool Bitmap::operator==(unsigned int zero) const {
  assert(zero == 0);
  for (const auto &a:arr) if (a != 0) return false;
  return true;
}
bool Bitmap::operator!=(unsigned int zero) const {
  assert(zero == 0);
  for (const auto &a:arr) if (a == 0) return false;
  return true;
}
bool Bitmap::operator==(const Bitmap &bm) const {
  const auto minSize = std::min(arr.size(), bm.arr.size());
  for (size_t i = 0; i < minSize; i++) if (arr[i] != bm.arr[i]) return false;
  for (size_t i = minSize; i < arr.size(); i++) if (arr[i] != 0) return false;
  for (size_t i = minSize; i < bm.arr.size(); i++) if (bm.arr[i] != 0) return false;
  return true;
}
Bitmap &Bitmap::operator&=(const Bitmap &bm) {
  const auto minSize = std::min(arr.size(), bm.arr.size());
  arr.resize(minSize);
  for (size_t i = 0; i < minSize; i++) arr[i] &= bm.arr[i];
  return *this;
}
Bitmap &Bitmap::operator|=(const Bitmap &bm) {
  const auto maxSize = std::max(arr.size(), bm.arr.size());
  arr.resize(maxSize);
  for (size_t i = 0; i < bm.arr.size(); i++) arr[i] |= bm.arr[i];
  return *this;
}

#endif //BITMAP_HPP_