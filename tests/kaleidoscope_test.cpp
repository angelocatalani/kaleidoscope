#include "../kaleidoscope/src/interpreter.h"
#include "../kaleidoscope/src/parser.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

// Fixture to parametrize a program's text into tokens types
class LexerMultiTokens : public testing::TestWithParam<
                             std::pair<std::string, std::vector<Token>>> {
public:
  void SetUp() override {}

  void TearDown() override {}
};

// Fixture to parametrize a program's text into token values
class LexerMultiValues : public testing::TestWithParam<
                             std::pair<std::string, std::vector<std::string>>> {
public:
  virtual void SetUp() {}

  virtual void TearDown() {}
};

INSTANTIATE_TEST_SUITE_P(
    LexerTest, LexerMultiTokens,
    ::testing::Values(
        std::pair<std::string, std::vector<Token>>(
            "def sum(x,y)",
            {tok_def, tok_identifier, tok_open_parenthesis, tok_identifier,
             tok_comma, tok_identifier, tok_close_parenthesis, tok_eof}),
        std::pair<std::string, std::vector<Token>>(
            "x+y-2  *3/4<>()  ",
            {tok_identifier, tok_plus, tok_identifier, tok_minus, tok_number,
             tok_multiply, tok_number, tok_divide, tok_number, tok_less,
             tok_greater, tok_open_parenthesis, tok_close_parenthesis,
             tok_eof}),
        std::pair<std::string, std::vector<Token>>(
            "sum(1,2);  ",
            {tok_identifier, tok_open_parenthesis, tok_number, tok_comma,
             tok_number, tok_close_parenthesis, tok_semicolon, tok_eof})));

INSTANTIATE_TEST_SUITE_P(
    LexerTest, LexerMultiValues,
    ::testing::Values(
        std::pair<std::string, std::vector<std::string>>(
            "def sum(x,y)", {"def", "sum", "(", "x", ",", "y", ")", "EOF"}),
        std::pair<std::string, std::vector<std::string>>(
            "def    123 abc sum(x,y) (",
            {"def", "123", "abc", "sum", "(", "x", ",", "y", ")", "(", "EOF"}),
        std::pair<std::string, std::vector<std::string>>(
            "   abc (,.<+-", {"abc", "(", ",", ".", "<", "+", "-", "EOF"})));

TEST(LexerTest, CorrectInitialization) {
  auto lexer = Lexer("123");
  SUCCEED();
  EXPECT_EQ(lexer.GetTokenType(), tok_number);
  EXPECT_EQ(lexer.GetTokenValue(), "123");
}
TEST(LexerTest, CorrectException) {
  auto lexer = Lexer("def =");
  EXPECT_EQ(lexer.GetTokenType(), tok_def);
  EXPECT_EQ(lexer.GetTokenValue(), "def");
  try {
    lexer.Next();
  } catch (std::exception &e) {
    EXPECT_EQ(std::string(e.what()), "unknown char: =");
  }
}

TEST_P(LexerMultiTokens, CorrectTokenizationTypes) {
  auto test_data = LexerMultiTokens::GetParam();
  auto text = test_data.first;
  auto tokens = test_data.second;

  auto lexer = Lexer(text);
  for (auto token : tokens) {
    EXPECT_EQ(lexer.GetTokenType(), token);
    lexer.Next();
  }
}

TEST_P(LexerMultiValues, CorrectTokenizationValues) {
  auto test_data = LexerMultiValues::GetParam();
  auto text = test_data.first;
  auto token_values = test_data.second;

  auto lexer = Lexer(text);
  for (const auto &token_value : token_values) {
    EXPECT_EQ(lexer.GetTokenValue(), token_value);
    lexer.Next();
  }
}

TEST(ParserTest, CorrectExternParsing) {
  auto parser = Parser("extern cos(x)");
  auto exp = parser.HandleExtern();
  std::unique_ptr<ExprAST> proto =
      std::make_unique<PrototypeAST>(PrototypeAST("cos", {"x"}));
  EXPECT_TRUE(exp->Equals(proto));
  std::unique_ptr<ExprAST> wrong_proto =
      std::make_unique<PrototypeAST>(PrototypeAST("cos", {"y"}));
  EXPECT_FALSE(exp->Equals(wrong_proto));
}

TEST(ParserTest, CorrectFunctionParsing) {
  auto parser = Parser("def sum(x y z) x+y*z");
  auto def = parser.HandleDefinition();

  auto proto =
      std::make_unique<PrototypeAST>(PrototypeAST("sum", {"x", "y", "z"}));
  std::unique_ptr<ExprAST> lhs =
      std::make_unique<VariableExprAST>(VariableExprAST("x"));

  std::unique_ptr<ExprAST> lhs_1 =
      std::make_unique<VariableExprAST>(VariableExprAST("y"));
  std::unique_ptr<ExprAST> rhs_1 =
      std::make_unique<VariableExprAST>(VariableExprAST("z"));

  std::unique_ptr<ExprAST> rhs = std::make_unique<BinaryExprAST>(
      tok_multiply, std::move(lhs_1), std::move(rhs_1));
  std::unique_ptr<ExprAST> body = std::make_unique<BinaryExprAST>(
      BinaryExprAST(tok_plus, std::move(lhs), std::move(rhs)));

  std::unique_ptr<ExprAST> correct_def = std::make_unique<FunctionAST>(
      FunctionAST(std::move(proto), std::move(body)));
  EXPECT_TRUE(def->Equals(correct_def));

  std::unique_ptr<ExprAST> wrong_body = std::make_unique<BinaryExprAST>(
      BinaryExprAST(tok_minus, std::move(lhs), std::move(rhs)));
  std::unique_ptr<ExprAST> wrong_def = std::make_unique<FunctionAST>(
      FunctionAST(std::move(proto), std::move(body)));
  EXPECT_FALSE(def->Equals(wrong_def));
}

TEST(ParserTest, CorrectCallFunctionParsing) {
  auto parser = Parser("sum(1 2 val prod(x))");
  auto def = parser.HandleTopLevelExpression();
  auto anon_proto =
      std::make_unique<PrototypeAST>(PrototypeAST("__anon_expr", {}));

  std::unique_ptr<ExprAST> arg1 =
      std::make_unique<NumberExprAST>(std::move(NumberExprAST(1)));
  std::unique_ptr<ExprAST> arg2 =
      std::make_unique<NumberExprAST>(std::move(NumberExprAST(2)));
  std::unique_ptr<ExprAST> arg3 =
      std::make_unique<VariableExprAST>(std::move(VariableExprAST("val")));
  std::vector<std::unique_ptr<ExprAST>> nested_args;
  std::unique_ptr<ExprAST> nested_arg1 =
      std::make_unique<VariableExprAST>(std::move(VariableExprAST("x")));

  nested_args.push_back(std::move(nested_arg1));
  std::unique_ptr<ExprAST> arg4 = std::make_unique<CallExprAST>(
      CallExprAST("prod", std::move(nested_args)));

  std::vector<std::unique_ptr<ExprAST>> args;
  args.push_back(std::move(arg1));
  args.push_back(std::move(arg2));
  args.push_back(std::move(arg3));
  args.push_back(std::move(arg4));
  std::unique_ptr<ExprAST> body =
      std::make_unique<CallExprAST>(CallExprAST("sum", std::move(args)));
  std::unique_ptr<ExprAST> correct_def = std::make_unique<FunctionAST>(
      FunctionAST(std::move(anon_proto), std::move(body)));
  EXPECT_TRUE(def->Equals(correct_def));

  std::vector<std::unique_ptr<ExprAST>> wrong_args;
  std::unique_ptr<ExprAST> wrong_arg1 =
      std::make_unique<NumberExprAST>(std::move(NumberExprAST(1)));
  wrong_args.push_back(std::move(wrong_arg1));
  std::unique_ptr<ExprAST> wrong_body =
      std::make_unique<CallExprAST>(CallExprAST("sum", std::move(wrong_args)));
  std::unique_ptr<ExprAST> wrong_def = std::make_unique<FunctionAST>(
      FunctionAST(std::move(anon_proto), std::move(wrong_body)));
  EXPECT_FALSE(def->Equals(wrong_def));
}

TEST(InterpreterTest, CorrectCodeGeneration) {
  auto parser = Parser("def sum(x) x");
  auto interpreter = Interpreter(parser);
  auto generated_code = interpreter.Interpret();
  std::string expected_code = "; ModuleID = 'my cool jit'\n"
                              "source_filename = \"my cool jit\"\n"
                              "\n"
                              "define double @sum(double %x) {\n"
                              "entry:\n"
                              "  ret double %x\n"
                              "}\n";

  EXPECT_TRUE(generated_code == expected_code);
}

TEST(InterpreterTest, CorrectDuplicateDefHandling) {

  auto parser = Parser("extern foo(a);foo(1);def foo(b c) b+c;foo(1 2)");
  auto interpreter = Interpreter(parser);
  auto generated_code = interpreter.Interpret();
  std::string expected_code = "; ModuleID = 'my cool jit'\n"
                              "source_filename = \"my cool jit\"\n"
                              "\n"
                              "define double @foo(double %b, double %c) {\n"
                              "entry:\n"
                              "  %addtmp = fadd double %b, %b\n"
                              "  ret double %addtmp\n"
                              "}\n"
                              "\n"
                              "define double @__anon_expr() {\n"
                              "entry:\n"
                              "  %calltmp = call double @foo(double "
                              "1.000000e+00, double 2.000000e+00)\n"
                              "  ret double %calltmp\n"
                              "}\n";

  EXPECT_TRUE(generated_code == expected_code);
}