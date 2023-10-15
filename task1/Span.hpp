#include <span>
#include <concepts>
#include <cstdlib>
#include <iterator>
#include <vector>
#include <array>


template <class T, std::size_t extent>
class SpanBase {
public:
  using size_type = std::size_t;

  static constexpr size_type Size() {
    return extent;
  }
  size_type GetExtent() const {
    return extent;
  }

  SpanBase(T* data): data_(data) {}

  SpanBase(T* data, size_type): data_(data) {}
protected:
  T* data_;
};

template<class T>
class SpanBase<T, std::dynamic_extent> {
public:
  using size_type = std::size_t;

  size_type Size() const {
    return extent_;
  }
  size_type GetExtent() const {
    return extent_;
  }

  SpanBase(T* data, size_type extent): data_(data), extent_(extent) {}
protected:
  T* data_;
private:
  size_type extent_;

};

template <std::size_t extent>
concept is_number = (extent == std::dynamic_extent);

template
  < class T
  , std::size_t extent = std::dynamic_extent  
  >
class Span: public SpanBase<T, extent> {
  using base = SpanBase<T, extent>;
public:
  // Reimplement the standard span interface here
  // (some more exotic methods are not checked by the tests and can be sipped)
  // Note that unliike std, the methods name should be Capitalized!
  // E.g. instead of subspan, do Subspan.
  // Note that this does not apply to iterator methods like begin/end/etc.
  
  template<typename U> requires ( !std::is_same<base, SpanBase<T, std::dynamic_extent>>::value )
  Span(T *data) : base(data) {}
  // Span(const Span<T>& other): base(other.Data()) {}
  Span(std::array<T, extent>& arr) : base(arr.data()) {}

  Span(T *data, std::size_t ex) : SpanBase<T, extent>(data, ex) {}
  template<typename U>
  Span(std::vector<U>& vec) : SpanBase<T, extent>(vec.data(), vec.size()) {}

  template<typename It>
  requires std::random_access_iterator<It>
  Span(It begin, std::size_t size) : Span(&(*begin), size) {}
  using iterator = T*;
  using const_iterator = const T*;
  using value_type = std::remove_const_t<T>;
  using element_type = T;
  using size_type = std::size_t;
  using pointer = T*;
  using const_pointer = const T*;
  using reference = T&;
  using const_reference = const T&;
  using difference_type = std::ptrdiff_t;

    iterator begin() const {
    return base::data_;
  }

  iterator end() const {
    return base::data_ + base::GetExtent();
  }

  auto rbegin() const {
    return std::reverse_iterator(end());
  }

  auto rend() const {
    return std::reverse_iterator(begin());
  }

  reference operator[](size_type idx) const {
    return base::data_[idx];
  }

  reference Front() const {
    return base::data_[0];
  }

  reference Back() const {
    return base::data_[base::GetExtent() - 1];
  }

  Span<T> First(size_type count) const {
    return Span<T>(base::data_, count);
  }

  Span<T> Last(size_type count) const {
    return Span<T>(base::data_ + base::GetExtent() - count, count);
  }

  template<std::size_t Count>
  Span<T, Count> First() const {
    return Span<T, Count>(base::data_, Count);
  }

  template<std::size_t Count>
  Span<T, Count> Last() const {
    return Span<T, Count>(base::data_ + base::GetExtent() - Count, Count);
  }
  T *Data() const {
    return base::data_;
  }

};

template<typename It>
  Span(It begin, std::size_t size) -> Span<typename std::iterator_traits<It>::value_type>;

template<typename T>
  Span(const std::vector<T>& vec) -> Span<T>;