#include "util.h"
int main(int argc, char **argv)
{
  if (argc > 2)
  {
    cout << "INVALID ARGS" << endl;
    return 1;
  }
  grammar g("in");
  g.setTerm("INPUT_TEXT");
  g.setTerm("HASH");
  g.setTerm("COMMA");
  g.setTerm("ID");
  g.setTerm("CHAR");
  g.setTerm("LPAREN");
  g.setTerm("RPAREN");
  g.setTerm("DOT");
  g.setTerm("OR");
  g.setTerm("STAR");
  g.setTerm("UNDERSCORE");

  g.add("in", {"tokens_section", "INPUT_TEXT"});
  g.add("tokens_section", {"token_list", "HASH"});
  g.add("token_list", {"token", "token_list_continue"});
  g.add("token_list_continue", {"COMMA", "token_list"});
  g.add("token_list_continue", {"epsilon"});
  g.add("token", {"ID", "expr"});
  g.add("expr", {"CHAR"});
  g.add("expr", {"LPAREN", "expr", "RPAREN", "expr_modifier"});
  g.add("expr_modifier", {"DOT", "LPAREN", "expr", "RPAREN"});
  g.add("expr_modifier", {"OR", "LPAREN", "expr", "RPAREN"});
  g.add("expr_modifier", {"STAR"});
  g.add("expr", {"UNDERSCORE"});

  g.printGrammar();
  cout << endl;

  g.calculateFirst(-1);
  g.printNTermFirst();
  cout << endl;
  g.calculateFollow();
  g.printFollow();
  cout << endl;
  if (g.checkPredictiveParse() && argc == 2)
  {
    g.generateParser(argv[1]);
    cout << "PREDICTIVE PARSER CREATED IN FILE: " << argv[1] << endl;
  }
  else
  {
    cout << "NO LL1 PREDICTIVE PARSER POSSIBLE" << endl;
  }

  return 0;
}
