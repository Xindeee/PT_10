#include "MyGrammarBaseListener.h"
#include "MyGrammarLexer.h"
#include "MyGrammarParser.h"
#include <ANTLRInputStream.h>
#include <antlr4-runtime.h>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>

std::unordered_map<std::string, int> hashtable;

// class MyGrammarListener : public antlr4::tree::ParseTreeListener {};
class Listener : public MyGrammarBaseListener {
  void exitExpr(MyGrammarParser::ExprContext *ctx) override {
    std::string var_name;

    if (ctx->term() != nullptr) {
      // expr ::= term = expr
      if (ctx->term()->tok.e == MyGrammarParser::TOK_TYPE_I)
        throw std::runtime_error("Can't assign to number");
      else if (ctx->term()->tok.e == MyGrammarParser::TOK_TYPE_S)
        var_name = std::get<std::string>(ctx->term()->tok.v);
      else
        var_name = std::get<std::pair<std::string,bool>>(ctx->term()->tok.v).first;
    } else if (ctx->add() != nullptr) {
      // expr ::= add
      ctx->i = ctx->add()->i;
      return;
    }

    hashtable[var_name] += ctx->expr()->i;

    if (ctx->term()->tok.e == MyGrammarParser::TOK_TYPE_INC) {
      if (std::get<std::pair<std::string, bool>>(ctx->term()->tok.v).second) {
        // ПОСТ инкремент
        ctx->i = hashtable[var_name]++;
        return;
      } else {
        // ПРЕД инкремент
        ctx->i = ++hashtable[var_name];
        return;
      }
    }

    ctx->i = hashtable[var_name];
  }

  void exitAdd(MyGrammarParser::AddContext *ctx) override {
    if (ctx->add() != nullptr) {
      // add ::= add + term
      ctx->i = ctx->add()->i;
    } else {
      // add ::= term
      ctx->i = 0;
    }

    switch (ctx->term()->tok.e) {
      case MyGrammarParser::TOK_TYPE_I:
        ctx->i += std::get<int>(ctx->term()->tok.v);
        break;
      case MyGrammarParser::TOK_TYPE_S:
        ensureinitted(std::get<std::string>(ctx->term()->tok.v));
        ctx->i += hashtable[std::get<std::string>(ctx->term()->tok.v)];
        break;
      case MyGrammarParser::TOK_TYPE_INC:
        ensureinitted(std::get<std::pair<std::string, bool>>(ctx->term()->tok.v).first);
        if (std::get<std::pair<std::string, bool>>(ctx->term()->tok.v).second)
          // ПОСТ инкремент
          ctx->i += hashtable[std::get<std::pair<std::string, bool>>(ctx->term()->tok.v).first]++;
        else
          // ПРЕД инкремент
          ctx->i += ++hashtable[std::get<std::pair<std::string, bool>>(ctx->term()->tok.v).first];
        break;
    }
  }

  void ensureinitted(std::string s) {
    if (hashtable.find(s) == hashtable.end()) {
      hashtable[s] = 0;
    }
  }
};

int main(int argc, const char *argv[]) {
  std::ifstream stream;
  if (argc > 1) {
    stream.open(argv[1]);
    std::cout << argv[1] << std::endl;
  } else {
    stream.open("../test");
  }
  antlr4::ANTLRInputStream input(stream);

  MyGrammarLexer lexer(&input);
  antlr4::CommonTokenStream tokens(&lexer);

  tokens.fill();
  for (auto token : tokens.getTokens()) {
    std::cout << token->toString() << std::endl;
  }

  MyGrammarParser parser(&tokens);
  Listener listener;
  parser.addParseListener(&listener);

  antlr4::tree::ParseTree *tree = parser.expr();

  std::cout << tree->toStringTree(&parser) << std::endl << std::endl;

  for (const auto &p : hashtable) {
    std::cout << p.first << " : " << p.second << std::endl;
  }

  return 0;
}
