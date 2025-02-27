#include <optional>
#include <iostream>
#include <tuple>
#include <vector>
#include <string>
#include <string_view>

namespace FauxCombinator {

  struct ParserException : public std::runtime_error {
    ParserException(std::string);
  };

  ParserException::ParserException(std::string s)
    : std::runtime_error{std::move(s)} {
  }

  template<typename TT>
  struct Token {
    TT const tokenType;
    std::string_view const tokenData;
  };

  template<typename TT>
  struct Parser {
    Parser(std::initializer_list<Token<TT>> xs)
      : tokens(xs)
      , index(0) {
    }

    Parser(std::vector<Token<TT>>&& xs)
      : tokens(std::move(xs))
      , index(0) {
    }

    bool isEOF() const { return index >= tokens.size(); }

    Token<TT> const* peek() const {
      if (isEOF()) throw ParserException{"EOF"};
      return tokens.data() + index;
    }

    Token<TT> const* expect(TT tt) {
      if (isEOF()) throw ParserException{"EOF"};
      Token<TT> const* next = peek();
      if (next->tokenType == tt) {
        ++index;
        return next;
      }
      throw ParserException{"Wrong type"};
    }

    template<typename T, typename F>
    std::optional<T> attempt(F&& f) {
      size_t rollback = index;
      try {
        return std::optional { f() };
      } catch (ParserException) {
        index = rollback;
        return std::nullopt;
      }
    }

    template<typename T>
    std::unique_ptr<T> either() {
      throw ParserException{"Either unmatched"};
    }

    template<typename T, typename F, typename... Fs>
    std::unique_ptr<T> either(F f, Fs... fs) {
      size_t rollback = index;
      try {
        return f();
      } catch (ParserException) {
        index = rollback;
        return either<T, Fs...>(fs...);
      }
    }
    
    template<typename T, typename F>
    std::vector<std::unique_ptr<T>> any(F f) {
      std::vector<std::unique_ptr<T>> xs;
      while (auto res = attempt<std::unique_ptr<T>>(f)) {
        xs.emplace_back(std::move(*res));
      }
      return xs;
    }

    template<typename T, typename F>
    std::vector<std::unique_ptr<T>> many(F f) {
      std::vector<std::unique_ptr<T>> xs;
      xs.emplace_back(f());
      while (auto res = attempt<std::unique_ptr<T>>(f)) {
        xs.emplace_back(std::move(*res));
      }
      return xs;
    }

  private:
    std::vector<Token<TT>> tokens;
    size_t index;
  };

}

