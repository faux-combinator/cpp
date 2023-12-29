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

    bool isEOF() { return index >= tokens.size(); }
    Token<TT> const* peek() {
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

  private:
    std::vector<Token<TT>> tokens;
    size_t index;
  };

}

struct Expr {
  virtual std::string format() = 0;
  virtual ~Expr() = 0;
};
Expr::~Expr() {}
struct Id : Expr {
  std::string_view const id;
  Id(std::string_view _id) : id(_id) {}
  
  std::string format() override { return std::string{id}; }
};
struct Call : Expr {
  std::unique_ptr<Expr> callee; 
  std::vector<std::unique_ptr<Expr>> arguments;

  Call(std::unique_ptr<Expr> _callee, std::vector<std::unique_ptr<Expr>> _arguments)
    : callee(std::move(_callee))
    , arguments(std::move(_arguments)) {
  }

  std::string format() override {
    std::string fmt = callee->format();
    fmt += "(";
    if (!arguments.empty()) {
      for (size_t i = 0; i < arguments.size(); ++i) {
        if (i != 0) fmt += ", ";
        fmt += arguments[i]->format();
      }
    }
    fmt += ")";
    return fmt;
  }
};

template<typename T>
using EitherFn = std::function<std::unique_ptr<T>()>;

enum MyTokenType { TokenLparen, TokenRparen, TokenId };
int main() {
  FauxCombinator::Parser<MyTokenType> p {
    { TokenLparen, "(" },
    { TokenId, "a" },
    { TokenLparen, "(" },
    { TokenLparen, "(" },
    { TokenId, "b" },
    { TokenRparen, ")" },
    { TokenId, "c" },
    { TokenRparen, ")" },
    { TokenId, "d" },
    { TokenRparen, ")" }
  };

  EitherFn<Expr> expr = [&expr, &p]() {
    return p.either<Expr>(
      [&p]() {
        auto t = p.expect(TokenId);
        return std::make_unique<Id>(t->tokenData);
      },
      [&p, &expr]() {
        p.expect(TokenLparen);
        auto callee = expr();
        std::vector<std::unique_ptr<Expr>> args = p.any<Expr>(expr);
        p.expect(TokenRparen);
        return std::make_unique<Call>(std::move(callee), std::move(args));
      }
    );
  };
  auto tree = expr();
  std::cout << tree->format() << '\n';
}

