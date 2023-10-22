#include <bits/iterator_concepts.h>
#include <cstddef>
#include <span>
#include <concepts>
#include <cstdlib>
#include <array>
#include <iterator>
#include <vector>
#include <iostream>
inline constexpr std::ptrdiff_t dynamic_stride = -1;

template <
  std::size_t extent
  >
class SliceExtentBase {
public:
  SliceExtentBase(std::size_t) {}
  SliceExtentBase() = default;

  static constexpr std::size_t Size() {
    return extent;
  }
};

template<>
class SliceExtentBase<std::dynamic_extent> {
public:
  SliceExtentBase(std::size_t extent): extent_(extent) {}
  SliceExtentBase() = default;
  std::size_t Size() const {
    return extent_;
  }

protected:
  std::size_t extent_;
};

template <
  std::ptrdiff_t stride
  >
class SliceStrideBase {
public:
  SliceStrideBase(std::ptrdiff_t) {}
  SliceStrideBase() = default;
  static constexpr std::ptrdiff_t Stride() {
    return stride;
  }
};

template<>
class SliceStrideBase<dynamic_stride> {
public:
  SliceStrideBase(std::ptrdiff_t stride): stride_(stride) {}
  SliceStrideBase() = default;

  std::ptrdiff_t Stride() const {
    std::cout << "MAM" << std::endl;
    return stride_;
  }
protected:
  std::ptrdiff_t stride_;
};


template
  < class T
  , std::size_t extent = std::dynamic_extent
  , std::ptrdiff_t stride = 1u
  >
class Slice: public SliceExtentBase<extent>, public SliceStrideBase<stride> {
public:

  class Iterator {
  private:
    T* data_;
    std::ptrdiff_t stride_;
  public:
    using iterator_category = std::contiguous_iterator_tag;
    using value_type = std::remove_const_t<T>;
    using element_type = T;
    using size_type = std::size_t;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const value_type&;
    using difference_type = std::ptrdiff_t;

    Iterator() = default;
    Iterator(T* data, std::ptrdiff_t stride__): data_(data), stride_(stride__) {}

    reference operator*() const {
      std::cout << "LOL1" << *data_ << std::endl;
      return *data_;
    }
    pointer operator->() {
      std::cout << "LOL2" << *data_ << std::endl;
      return data_;
    }
    reference operator[](size_type idx) const {
      std::cout << "LOL3" << *data_ << std::endl;
      return data_[idx];
    }

    Iterator& operator++() {
      data_ += stride_;
      return *this;
    }
    Iterator& operator--() {
      data_ -= stride_;
      return *this;
    }
    Iterator operator++(int) {
      auto tmp = *this;
      ++(*this);
      return tmp;
    }
    Iterator operator--(int) {
      auto tmp = *this;
      --(*this);
      return tmp;
    }

    Iterator& operator+=(size_type plus) {
      data_ += stride_ * plus;
      return *this;
    }
    Iterator& operator-=(size_type minus) {
      data_ -= stride_ * minus;
      return *this;
    }
    Iterator operator+(size_type plus) const {
      Iterator tmp = *this;
      tmp += plus;
      return tmp;
    }
    Iterator operator-(size_type minus) const {
      Iterator tmp = *this;
      tmp -= minus;
      return tmp;
    }

    difference_type operator-(const Iterator& other) const {
      return (data_ - other.data_) / stride_;
    }
    friend Iterator operator+(size_type left, const Iterator& right) {
      return right + left;
    }

    inline bool operator==(const Iterator& other) const {
      return data_ == other.data_;
    }
    inline bool operator!=(const Iterator& other) const {
      return data_ != other.data_;
    }
    inline bool operator>(const Iterator& other) const {
      return data_ > other.data_;
    }
    inline bool operator<(const Iterator& other) const {
      return data_ < other.data_;
    }
    inline bool operator>=(const Iterator& other) const {
      return data_ >= other.data_;
    }
    inline bool operator<=(const Iterator& other) const {
      return data_ <= other.data_;
    }
  };

  using iterator = Iterator;
  using value_type = std::remove_const_t<T>;
  using element_type = T;
  using size_type = std::size_t;
  using pointer = T*;
  using const_pointer = const T*;
  using reference = T&;
  using const_reference = const T&;
  using difference_type = std::ptrdiff_t;

  Slice() = default;
  Slice(const Slice&) = default;
  explicit Slice(Slice&&) = default;
  Slice& operator=(const Slice&) = default;
  Slice& operator=(Slice&&) = default;

  template <class It>
  requires std::contiguous_iterator<It>
  Slice(It begin, size_type count, difference_type skip): SliceExtentBase<extent>(count), SliceStrideBase<stride>(skip), data_(&(*begin)) {}

  template<class V>
    Slice(std::vector<V>& container): 
      Slice(container.data(), container.size(), 1u) {}

  template<class V, size_type sz>
    Slice(std::array<V, sz>& container): 
      Slice(container.data(), sz, 1u) {}

  template<class U, size_type otherExtent, difference_type otherStride>
  operator Slice<U, otherExtent, otherStride>() const {
    return Slice<U, otherExtent, otherStride>(data_, this->Size(), this->Stride());
  }

  iterator begin() const {
    std::cout << "LOL" << *data_ << std::endl;
    return Iterator(data_, this->Stride());
  }

  iterator end() const {
    return Iterator(data_ + this->Size() * this->Stride(), this->Stride());
  }

  auto rbegin() const {
    return std::reverse_iterator(end());
  }

  auto rend() const {
    return std::reverse_iterator(begin());
  }

  reference operator[](size_type idx) const {
    return data_[idx * this->Stride()];
  }

  T* Data() const {
    return data_;
  }

  template<class U, size_type other_extent, difference_type other_stride>
  bool operator==(const Slice<U, other_extent, other_stride>& other_slice) const {
    auto other_it = other_slice.begin();
    for (auto it = begin(); it != end(); ++it) {
      if (other_it == other_slice.end()) {
        return false;
      }
      if (*it != *other_it) {
        return false;
      }
      ++other_it;
    }
    if (other_it != other_slice.end()) {
      return false;
    }
    return true;
  }

  Slice<T, std::dynamic_extent, stride>
    First(std::size_t count) const {
      return Slice<T, std::dynamic_extent, stride>(data_, count, this->Stride());
    }

  template <std::size_t count>
  Slice<T, count, stride>
    First() const {
      return Slice<T, count, stride>(data_, count, this->Stride());
    }

  Slice<T, std::dynamic_extent, stride>
    Last(std::size_t count) const {
      return Slice<T, std::dynamic_extent, stride>(data_ + this->Size() * this->Stride() - count * this->Stride(), count, this->Stride());
    }

  template <std::size_t count>
  Slice<T, count, stride>
    Last() const {
      return Slice<T, count, stride>(data_ + this->Size() * this->Stride() - count * this->Stride(), count, this->Stride());
    }

private:
  static constexpr size_type DroppedExtent(const difference_type drop) {
    if constexpr (extent == std::dynamic_extent) {
      return std::dynamic_extent;
    }
    return extent - drop;
  }
public:
  Slice<T, std::dynamic_extent, stride>
    DropFirst(std::size_t count) const {
      return Slice<T, std::dynamic_extent, stride>(data_ + count * stride, this->Size() - count, this->Stride());
    }

  template <std::size_t count>
  Slice<T, DroppedExtent(count), stride>
    DropFirst() const {
      return Slice(data_ + count * stride, this->Size() - count, this->Stride());
    }
  Slice<T, std::dynamic_extent, stride>
    DropLast(std::size_t count) const {
      return Slice<T, std::dynamic_extent, stride>(data_, this->Size() - count, this->Stride());
    }

  template <std::size_t count>
  Slice<T, DroppedExtent(count), stride>
    DropLast() const {
      return Slice(data_, this->Size() - count, this->Stride());
    }

  
private:

  static constexpr inline size_type RoudingUpDiv(const size_type a, const difference_type b) {
    return (a + b - 1) / b;
  }

  static constexpr size_type SkippedExtent(const difference_type skip) {
    if constexpr (extent == std::dynamic_extent) {
      return std::dynamic_extent;
    }
    return RoudingUpDiv(extent, skip);
  }

  static constexpr difference_type SkippedStride(const difference_type skip) {
    if constexpr (stride == dynamic_stride) {
      return dynamic_stride;
    }
    return stride * skip;
  }

public:

  Slice<T, std::dynamic_extent, dynamic_stride> 
    Skip(std::ptrdiff_t skip) const {
      return Slice<T, std::dynamic_extent, dynamic_stride>(
        data_, RoudingUpDiv(this->Size(), skip), this->Stride() * skip);
    }
  
  template <difference_type skip>
    Slice<T, SkippedExtent(skip), SkippedStride(skip)> Skip() const {
      return Slice(data_, RoudingUpDiv(this->Size(), skip), this->Stride() * skip);
    }
private:
  T* data_;
};
template<class T>
Slice(const std::vector<T>& container) -> Slice<T>;

template<class T, std::size_t Size>
Slice(const std::array<T, Size>& container) -> Slice<T, Size>;

template<class It>
Slice(It begin, std::size_t count, std::ptrdiff_t skip) -> Slice<typename std::iterator_traits<It>::value_type, std::dynamic_extent, dynamic_stride>;