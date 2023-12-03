#pragma once
#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>
#include <memory>
#include <iostream>

template<typename T>
concept DefaultConstructible = requires { T(); };

template<typename T>
concept CopyConstructible = requires (const T& t) {
    T(t);
};

template<typename T>
concept MoveConstructible = requires (T&& t) {
    T(std::move(t));
};

template<typename T>
concept Copyable = requires (T a, const T& t) {
    a = t;
};

template<typename T>
concept Movable = requires (T a, T&& t) {
    a = std::move(t);
};

template<typename T>
concept Destructible = requires (T t) {
    { std::destroy_at(&t) } noexcept;
};

template<typename T>
concept EqualityComparable = requires (T a, T b) {
    a == b;
};

template<typename T, typename Y>
concept SameConcepts = 
  (!std::copyable<Y> || std::copyable<T>) &&
  (!std::movable<Y> || std::movable<T>) &&
  // (!std::semiregular<Y> || std::semiregular<T>) &&
  (!std::regular<Y> || std::regular<T>);

class LoggerConcept {
public:

    virtual ~LoggerConcept() = default;
    virtual void operator()(int a) = 0;
    // virtual LoggerConcept& copy() const = 0;

    virtual LoggerConcept* copy() {
      return nullptr;
    }
    virtual LoggerConcept* move() {
      return nullptr;
    }
};

template <typename LoggerType>
class LoggerModel : public LoggerConcept {
private:
  LoggerType logger;

public:
  
  LoggerModel() requires(std::is_default_constructible_v<LoggerType>): logger() {}

  LoggerModel(const LoggerType& x) requires(std::is_copy_constructible_v<LoggerType>): logger(x) {}

  LoggerModel(LoggerType&& x) requires(std::is_move_constructible_v<LoggerType>): logger(std::move(x)) {}

  // LoggerModel(const LoggerType& logger_) : logger(logger_) {}
  void operator()(int a) override {
      logger(a);
  } 

  constexpr LoggerConcept* copy() override {
    if constexpr (std::is_copy_constructible_v<LoggerType>) {
      return new LoggerModel(logger);
    }
    return nullptr;
  }

  constexpr LoggerConcept* move() override {
    if constexpr (std::is_move_constructible_v<LoggerType>) {
      return new LoggerModel(std::move(logger));
    }
    return nullptr;
  }
  // template<CopyConstructible T = LoggerType>
  // LoggerModel<LoggerType>& copy() const override {
  //   LoggerModel<LoggerType> a = LoggerModel(*this);
  //   return a;
  // }
};


// Allocator is only used in bonus SBO tests,
// ignore if you don't need bonus points.
template <class T /*, class Allocator = std::allocator<std::byte>*/ >
class Spy {
public:
  // explicit Spy(T&& val): value_(val) {}
  
  // template<class Y = T> requires std::is_default_constructible<T>
  Spy() requires std::default_initializable<T> : value_(), logger_(nullptr){}
  T& operator *() {
    return &value_;
  }

  Spy(const T& x) requires(std::is_copy_constructible_v<T>) : value_(x), logger_(nullptr) {}

  Spy(T&& x) requires(std::is_move_constructible_v<T>) : value_(std::move(x)), logger_(nullptr) {}

  Spy(const Spy& x) requires(std::is_copy_constructible_v<T>) : value_(x.value_) {
    // std::cerr << "WWEWE\n";
    if (logger_) delete logger_;
    if (x.logger_) {
      logger_ = x.logger_->copy();
    } else {
      logger_ = nullptr;
    }
  }
  
  Spy(Spy&& x) requires(std::is_move_constructible_v<T>) : value_(std::move(x.value_)) {
    // std::cerr << "WWEeeeWE\n";
    if (logger_) delete logger_;
    if (x.logger_) {
      logger_ = x.logger_->move();
    } else {
      logger_ = nullptr;
    }
  }

  Spy& operator=(const Spy& x) requires(std::is_copy_assignable_v<T>) {
    if (this == &x) return *this;
    // if (kek) delete kek;
    value_ = x.value_;
    // std::cerr << "WWEWE\n";
    if (logger_) delete logger_;
    if (x.logger_) {
      logger_ = x.logger_->copy();
    } else {
      logger_ = nullptr;
    }
    return *this;
  }
  
  Spy& operator=(Spy&& x) requires(std::is_move_assignable_v<T>) {
    if (this == &x) return *this;
    // if (kek) delete kek;
    value_ = std::move(x.value_);
    // std::cerr << "WWEeeeWE\n";
    if (logger_) delete logger_;
    if (x.logger_) {
      logger_ = x.logger_->move();
      // if (!logger_) {
      //   std::cerr << "SSSSSS" << std::endl;
      // }
    } else {
      logger_ = nullptr;
    }
    return *this;
  }


  bool operator==(const Spy& b) const requires(std::equality_comparable<T>) {
    return value_ == *b;
  }

  const T& operator *() const {
    return &value_;
  }
  
  struct helper {
    T* operator->() {
      return value_;
    }
    ~helper() {
      // std::cerr << (*wtf) << '\n';
      (*wtf)--;
      if (*wtf == 0 && logger_) {
        // std::cerr << *cnt << '\n';
        // *cnt = 0;
        (*logger_)(*cnt);
      }
      // *cnt = 0;
      // std::cerr << "deleted\n";
      value_ = nullptr;
      logger_ = nullptr;
      cnt = nullptr;
      wtf = nullptr;
    }
    T* value_ = nullptr;
    LoggerConcept* logger_ = nullptr;
    int* cnt = nullptr;
    int* wtf = nullptr;
  };

  
  // helper* kek = nullptr;
  int wtf = 0;
  int cnt = 0;
  helper operator ->() {
    // std::cerr << wtf << ' ' << cnt << "AAAAA" << std::endl;
    if (wtf) {
      // std::cerr << "entered" << std::endl;
      cnt++;
      wtf++;
      return helper{&value_, logger_, &cnt, &wtf};
    }
    wtf = 1;
    cnt = 1;
    // if (kek) delete kek;
    // std::cerr << "created" << std::endl;
    return helper{&value_, logger_, &cnt, &wtf};

  }
  

  // Resets logger
  void setLogger() {
    delete logger_;
    logger_ = nullptr;
  }

  template <std::invocable<unsigned int> Logger> requires SameConcepts<Logger, T>
  void setLogger(Logger&& logger) {
    if (logger_) {
      delete logger_;
    }
    // std::cerr << "WTFFF" << std::endl;
    logger_ = new LoggerModel<Logger>(std::move(logger));
  }

  template <std::invocable<unsigned int> Logger> requires SameConcepts<Logger, T>
    void setLogger(const Logger& logger) {
      if (logger_) {
        delete logger_;
      }
    // std::cerr << "WTF2FF" << std::endl;
      logger_ = new LoggerModel<Logger>(logger);
  }

  ~Spy() {
    if (logger_) delete logger_;
    // if (kek) delete kek;
  }
private:
  T value_;
  LoggerConcept* logger_ = nullptr;
};
// int Spy::helper::wtf = 0;