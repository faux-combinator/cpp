#include <gtest/gtest.h>
#include <faux_combinator.hpp>

enum class MyToken { TokenLparen, TokenRparen, TokenId };
using enum MyToken;
using MyTokenType = FauxCombinator::StringViewToken<MyToken>;
using Token = MyTokenType const*; // TODO rename PCToken (?)

TEST(FauxCombinatorTest, isEOF) {
  {
    FauxCombinator::Parser<MyTokenType> p { };
    EXPECT_TRUE(p.isEOF());
  }

  {
    FauxCombinator::Parser<MyTokenType> p {
      { TokenLparen, "(" },
    };
    EXPECT_FALSE(p.isEOF());
  }
}

TEST(FauxCombinatorTest, peek) {
  EXPECT_THROW({
    FauxCombinator::Parser<MyTokenType> p { };
    p.peek();
  }, FauxCombinator::ParserException);

  {
    FauxCombinator::Parser<MyTokenType> p {
      { TokenLparen, "(" },
    };
    EXPECT_EQ(p.peek()->tokenType, TokenLparen);
    EXPECT_EQ(p.peek()->tokenData, "(");
  }
}

TEST(FauxCombinatorTest, expect) {
  EXPECT_THROW({
    FauxCombinator::Parser<MyTokenType> p { };
    p.expect(TokenLparen);
  }, FauxCombinator::ParserException);

  EXPECT_THROW(({ // Parens so that the inner {}s don't mess everything up
    FauxCombinator::Parser<MyTokenType> p {
      { TokenLparen, "(" },
    };
    p.expect(TokenRparen);
  }), FauxCombinator::ParserException);

  {
    FauxCombinator::Parser<MyTokenType> p {
      { TokenLparen, "(" },
    };
    auto token = p.expect(TokenLparen);
    EXPECT_EQ(token->tokenType, TokenLparen);
    EXPECT_EQ(token->tokenData, "(");
  }

  {
    FauxCombinator::Parser<MyTokenType> p {
      { TokenLparen, "(" },
      { TokenRparen, ")" },
    };
    auto token1 = p.expect(TokenLparen);
    EXPECT_EQ(token1->tokenType, TokenLparen);
    EXPECT_EQ(token1->tokenData, "(");
    auto token2 = p.expect(TokenRparen);
    EXPECT_EQ(token2->tokenType, TokenRparen);
    EXPECT_EQ(token2->tokenData, ")");
  }
}

TEST(FauxCombinatorTest, attempt) {
  {
    FauxCombinator::Parser<MyTokenType> p { };
    ASSERT_FALSE(p.attempt<Token>([&p]() { return p.peek(); }));
  }

  {
    FauxCombinator::Parser<MyTokenType> p {
      { TokenLparen, "(" },
    };
    ASSERT_FALSE(p.attempt<Token>([&p]() { return p.expect(TokenRparen); }));
  }

  {
    FauxCombinator::Parser<MyTokenType> p {
      { TokenLparen, "(" },
    };
    ASSERT_FALSE(p.attempt<std::unique_ptr<Token>>([&p]() { return p.either<Token>(); }));
  }
}

TEST(FauxCombinatorTest, either) {
  EXPECT_THROW({
    FauxCombinator::Parser<MyTokenType> p { };
    p.either<Token>();
  }, FauxCombinator::ParserException);

  EXPECT_THROW(({
    FauxCombinator::Parser<MyTokenType> p {
      { TokenId, "a" },
    };
    //using Token = FauxCombinator::Token<MyTokenType> const*;
    p.either<std::string>(
      [&p]() { return std::make_unique<std::string>(p.expect(TokenLparen)->tokenData); },
      [&p]() { return std::make_unique<std::string>(p.expect(TokenRparen)->tokenData); }
    );
  }), FauxCombinator::ParserException);

  {
    FauxCombinator::Parser<MyTokenType> p {
      { TokenLparen, "(" },
    };
    auto res = p.either<std::string>(
      [&p]() { return std::make_unique<std::string>(p.expect(TokenLparen)->tokenData); },
      [&p]() { return std::make_unique<std::string>(p.expect(TokenRparen)->tokenData); }
    );
    ASSERT_EQ(*res, "(");
  }

  {
    FauxCombinator::Parser<MyTokenType> p {
      { TokenLparen, ")" },
    };
    auto res = p.either<std::string>(
      [&p]() { return std::make_unique<std::string>(p.expect(TokenLparen)->tokenData); },
      [&p]() { return std::make_unique<std::string>(p.expect(TokenRparen)->tokenData); }
    );
    ASSERT_EQ(*res, ")");
  }
}

TEST(FauxCombinatorTest, any) {
  {
    FauxCombinator::Parser<MyTokenType> p { };
    auto res = p.any<std::string>([&p]() {
      return std::make_unique<std::string>(p.expect(TokenId)->tokenData);
    });
    ASSERT_EQ(0, res.size());
  }

  {
    FauxCombinator::Parser<MyTokenType> p {
      { TokenId, "a" }
    };
    auto res = p.any<std::string>([&p]() {
      return std::make_unique<std::string>(p.expect(TokenId)->tokenData);
    });
    ASSERT_EQ(1, res.size());
    ASSERT_EQ("a", *res[0]);
  }


  {
    FauxCombinator::Parser<MyTokenType> p {
      { TokenId, "a" },
      { TokenId, "b" },
      { TokenId, "c" }
    };
    auto res = p.any<std::string>([&p]() {
      return std::make_unique<std::string>(p.expect(TokenId)->tokenData);
    });
    ASSERT_EQ(3, res.size());
    ASSERT_EQ("a", *res[0]);
    ASSERT_EQ("b", *res[1]);
    ASSERT_EQ("c", *res[2]);
  }
}

TEST(FauxCombinatorTest, many) {
  ASSERT_THROW(({
    FauxCombinator::Parser<MyTokenType> p { };
    p.many<std::string>([&p]() {
      return std::make_unique<std::string>(p.expect(TokenId)->tokenData);
    });
  }), FauxCombinator::ParserException);

  {
    FauxCombinator::Parser<MyTokenType> p {
      { TokenId, "a" }
    };
    auto res = p.many<std::string>([&p]() {
      return std::make_unique<std::string>(p.expect(TokenId)->tokenData);
    });
    ASSERT_EQ(1, res.size());
    ASSERT_EQ("a", *res[0]);
  }

  {
    FauxCombinator::Parser<MyTokenType> p {
      { TokenId, "a" },
      { TokenId, "b" },
      { TokenId, "c" }
    };
    auto res = p.many<std::string>([&p]() {
      return std::make_unique<std::string>(p.expect(TokenId)->tokenData);
    });
    ASSERT_EQ(3, res.size());
    ASSERT_EQ("a", *res[0]);
    ASSERT_EQ("b", *res[1]);
    ASSERT_EQ("c", *res[2]);
  }
}

struct Expr {
  virtual std::string format() const = 0;
  virtual ~Expr() = 0;
};
Expr::~Expr() = default;
struct Id : Expr {
  std::string_view const id;
  Id(std::string_view _id) : id(_id) {}
  
  std::string format() const override { return std::string{id}; }
};
struct Call : Expr {
  std::unique_ptr<Expr> callee; 
  std::vector<std::unique_ptr<Expr>> arguments;

  Call(std::unique_ptr<Expr> _callee, std::vector<std::unique_ptr<Expr>> _arguments)
    : callee(std::move(_callee))
    , arguments(std::move(_arguments)) {
  }

  std::string format() const override {
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

TEST(FauxCombinatorTest, tree) {
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
  EXPECT_EQ(tree->format(), "a(b()(c), d)");
}
