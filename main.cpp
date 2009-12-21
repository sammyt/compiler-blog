#include <iostream>
#include <vector>

using namespace std;

//===----------------------------------------------------------------------===//
// Setup the base Compiler and CompilerStep classes
//===----------------------------------------------------------------------===//

// forward define CompilerStep
class CompilerStep;
class Token;

class Compiler {
  vector<CompilerStep *> _steps;
  
public:
  string code;
  vector<Token*> tokens;
  
  void add_step(CompilerStep *step){
    _steps.push_back(step);
  }
  void compile(void);
};

class CompilerStep{
public:
  virtual void process(Compiler *compiler) = 0;
};

void Compiler::compile(){
  for (int i=0; i<_steps.size(); i++) {
    _steps.at(i)->process(this);
  }  
};

//===----------------------------------------------------------------------===//
// Define the CompilerStep subclasses
//===----------------------------------------------------------------------===//


//===----------------------------------------------------------------------===//
// Tokenise the input source
//===----------------------------------------------------------------------===//

enum TokenType {
  token_identifier,
  token_integer_value,
  token_type,
  token_right_paren,
  token_left_paren,
  token_right_curly,
  token_left_curly,
  token_eol,
  token_ret,
  token_operation,
};

class Token {
  
  bool is_null;
  TokenType type;
  
public:
  
  Token(){
    is_null = true;
  };
  Token(TokenType t_type): type(t_type){
    is_null = false;
  };
  bool isNull(){
    return is_null;
  }
  string repr(){
    switch (type) {
      case token_integer_value:
        return "value ";
      case token_identifier:
        return "identifier ";
      case token_left_paren:
        return "( ";
      case token_right_paren:
        return ") ";
      case token_left_curly:
        return "{ ";
      case token_right_curly:
        return "} ";
      case token_eol:
        return "; ";
      case token_ret:
        return "return ";
      case token_operation:
        return "operation ";
      case token_type:
        return "type ";
      default:
        return "other ";
    }
  }
  
  // should really have subclasses
  // to store these, but hey, its a demo
  string identifer_value;
  int int_value;
};

class TokeniseStep : public CompilerStep {
  string *code;
  char last_char;
  int index;
  
  Token token_from_char(TokenType t){
    Token tk = Token(t);
    last_char = code->at(index);
    index ++;
    return tk;
  }
  
public:
  void process(Compiler *compiler){
    cout << "TokeniseStep" << endl;
    code = &compiler->code;
    index = 0;
    last_char = ' ' ;
    
    while (true) {
      Token token = find();
      
      cout << token.repr();
      
      if (token.isNull() || code->length() == index ) {
        break;
      }
      
      compiler->tokens.push_back(&token);
    }
  }
  
  Token find(){
    eat_white_space();
    
    if (isalpha(last_char)) return word();
    if (isdigit(last_char)) return number();
    if (last_char == '(') return token_from_char(token_left_paren);
    if (last_char == ')') return token_from_char(token_right_paren);
    if (last_char == '{') return token_from_char(token_left_curly);
    if (last_char == '}') return token_from_char(token_right_curly);
    if (last_char == ';') return token_from_char(token_eol);
    if (last_char == '+') return token_from_char(token_operation);
    Token a;
    return a;
  }
  
  void eat_white_space(){
    while (isspace(last_char)) {
      last_char = code->at(index);
      index++;
    }
  }
  
  Token word(){
    string current = "";
    while (isalpha(last_char)) {
      current += last_char;
      last_char = code->at(index);
      index++;
    }
    
    if(current == "int"){
      Token type = Token(token_type);
      return type; 
    }
    
    if(current == "return"){
      Token type = Token(token_ret);
      return type; 
    }
    
    Token word = Token(token_identifier);
    word.identifer_value = current;
    return word;
  }
  
  Token number(){
    string current = "";
    while (isdigit(last_char)) {
      current += last_char;
      last_char = code->at(index);
      index++;
    }
    Token num = Token(token_integer_value);
    num.int_value = atoi(current.c_str());
    return num;
  }
};


//===----------------------------------------------------------------------===//
// Build the abstract syntax tree from the tokens
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Define the nodes of the tree
//===----------------------------------------------------------------------===//
enum LanguageType {
  integer,
};

class ExpressionNode {
public:
  LanguageType type;
};

class IntegerExpressionNode : public ExpressionNode {
  int value;
public:
  IntegerExpressionNode(int val) : value(val){}
};

class VariableExpressionNode : public ExpressionNode {
  string name;
public:
  VariableExpressionNode(string identifier) : name(identifier){}
};

class AddExpressionNode : public ExpressionNode {
  ExpressionNode left;
  ExpressionNode right;
public:
  AddExpressionNode(ExpressionNode l, ExpressionNode r) : left(l), right(r){}
};

class InvokeExpression : public ExpressionNode {
  string name;
  vector<ExpressionNode*> arguments;
public:
  InvokeExpression(string callee, vector<ExpressionNode*> args): 
    name(callee), arguments(args){}
};

class FunctionNode {
  string name;
  vector<VariableExpressionNode*> params;
  ExpressionNode *body;
public:
  FunctionNode(string fname, 
    vector<VariableExpressionNode*> fparams, ExpressionNode *fbody) : 
      name(fname), params(fparams), body(fbody){}
};

class BuildAstStep : public CompilerStep {
  vector<Token*> *tokens;
  Token current_token;
  int index;
public:
  void process(Compiler *compiler){
    cout << endl << "BuildAstStep" << endl;
    index = 0;
    tokens = &compiler->tokens;
    
  }
  
  /*Token getNextToken(){
    
  }*/
  
  // a IntegerExpressionNode parser
  // e.g. ::= 4
  ExpressionNode *parseIntegerExpression(){
    ExpressionNode *node = new IntegerExpressionNode(current_token.int_value);
    return node;
  }
};

//===----------------------------------------------------------------------===//
// Entry point for compiler
//===----------------------------------------------------------------------===//
int main(string args[]){
  Compiler compiler;
  TokeniseStep tokenise_step;
  BuildAstStep build_ast_step;
  
  compiler.add_step(&tokenise_step);
  compiler.add_step(&build_ast_step);
  compiler.code = "int doIt(int a){return a + 1;} int main(){doIt(3);}";
  compiler.compile();
};

