#include <optional>
#include <iostream>
#include <tuple>
#include <vector>
#include <string>
#include <string_view>

namespace FauxCombinator {

  struct ParserException : public std::runtime_error {
    ParserException(std::string s)
      : std::runtime_error{std::move(s)} {
      }
  };

  template<typename T>
  concept Token = requires (T const a, typename T::type const& b) {
    typename T::type;
    { a.accepts(b) } -> std::same_as<bool>;
  };

  // Simple, easy-to-use for a simple lexing library
  template<typename TT>
  struct StringViewToken {
    using type = TT;
    bool accepts(TT const& t) const {
      return tokenType == t;
    }

    TT const tokenType;
    std::string_view const tokenData;
  };

  static_assert(Token<StringViewToken<int>>);

  template<Token T>
  struct Parser {
    using TT = typename T::type;

    Parser(std::initializer_list<T> xs)
      : tokens(xs)
      , index(0) {
    }

    Parser(std::vector<T>&& xs)
      : tokens(std::move(xs))
      , index(0) {
    }

    bool isEOF() const { return index >= tokens.size(); }

    T const* peek() const {
      if (isEOF()) throw ParserException{"EOF"};
      return tokens.data() + index;
    }

    T const* expect(TT const& t) {
      if (isEOF()) throw ParserException{"EOF"};
      T const* next = peek();
      if (next->accepts(t)) {
        ++index;
        return next;
      }
      throw ParserException{"Wrong type"};
    }

    template<typename R, typename F>
    std::optional<R> attempt(F&& f) {
      size_t rollback = index;
      try {
        return std::optional { std::forward<F>(f)() };
      } catch (ParserException) {
        index = rollback;
        return std::nullopt;
      }
    }

    template<typename R>
    std::unique_ptr<R> either() {
      throw ParserException{"Either unmatched"};
    }

    template<typename R, typename F, typename... Fs>
    std::unique_ptr<R> either(F&& f, Fs... fs) {
      size_t rollback = index;
      try {
        return std::forward<F>(f)();
      } catch (ParserException) {
        index = rollback;
        return either<R, Fs...>(std::forward<Fs>(fs)...);
      }
    }
    
    template<typename R, typename F>
    std::vector<std::unique_ptr<R>> any(F&& f) {
      std::vector<std::unique_ptr<R>> xs;
      while (auto res = attempt<std::unique_ptr<R>>(f)) {
        xs.emplace_back(std::move(*res));
      }
      return xs;
    }

    template<typename R, typename F>
    std::vector<std::unique_ptr<R>> many(F&& f) {
      std::vector<std::unique_ptr<R>> xs;
      xs.emplace_back(f());
      while (auto res = attempt<std::unique_ptr<R>>(f)) {
        xs.emplace_back(std::move(*res));
      }
      return xs;
    }

  private:
    std::vector<T> tokens;
    size_t index;
  };

}

