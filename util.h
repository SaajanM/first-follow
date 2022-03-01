#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
using namespace std;

class grammar
{
  private:
  map<string, vector<vector<string>>> rules;
  vector<string> ruleList;
  string sRule;
  multimap<string, set<string>> first;
  multimap<string, set<string>> follow;
  set<string> terminals;

  public:
  grammar(string startRule) : rules({}), ruleList({}), sRule(startRule), first({}), follow({}), terminals({}){
    setlocale( LC_ALL, "en_US.utf8" );
  };
  void add(string nterm, vector<string> ruleset)
  {
    if (!rules.count(nterm))
    {
      vector<vector<string>> temp = {};
      ruleList.push_back(nterm);
      rules.insert(make_pair(nterm, temp));
    }
    rules.find(nterm)->second.push_back(ruleset);
  };
  void setTerm(string term)
  {
    terminals.insert(term);
  };
  void calculateFirst(int cycles)
  {
    bool completeCalculation = cycles == -1;
    pair<string, set<string>> eps = make_pair<string, set<string>>("epsilon", {"epsilon"});
    first.insert(eps);

    for (auto i = terminals.begin(); i != terminals.end(); i++)
    {
      first.insert(make_pair(*i, set<string>({*i})));
    }

    for (auto i = rules.begin(); i != rules.end(); i++)
    {
      first.insert(make_pair(i->first, set<string>({})));
    }

    bool changed = true;
    int count = 0;
    while (completeCalculation ? changed : (count < cycles))
    {
      count++;
      changed = false;
      for (auto i = ruleList.begin(); i != ruleList.end(); i++)
      {
        string currentNTerm = *i;
        vector<vector<string>> currCRule = rules.find(*i)->second;
        for (auto j = currCRule.begin(); j != currCRule.end(); j++)
        {
          bool containsEpsilon = true;
          vector<string> currRule = *j;
          for (auto k = currRule.begin(); k != currRule.end() && containsEpsilon; k++)
          {
            set<string> firstK = first.find(*k)->second;
            if (!firstK.count("epsilon")) containsEpsilon = false;

            set<string> temp = set<string>(firstK.begin(), firstK.end());
            temp.erase("epsilon");
            size_t sizeB = first.find(currentNTerm)->second.size();
            first.find(currentNTerm)->second.insert(temp.begin(), temp.end());
            if (sizeB != first.find(currentNTerm)->second.size()) changed = true;
          }
          if (containsEpsilon)
          {
            size_t sizeB = first.find(currentNTerm)->second.size();
            first.find(currentNTerm)->second.insert("epsilon");
            if (sizeB != first.find(currentNTerm)->second.size()) changed = true;
          }
        }
      }
    }
  }
  void printNTermFirst()
  {
    cout << "FIRST SETS:" << endl;
    for (auto i = first.begin(); i != first.end(); i++)
    {
      if (terminals.count(i->first) || i->first == "epsilon") continue;
      cout << "\t" << i->first << " = { ";
      for (auto j = i->second.begin(); j != i->second.end(); j++)
      {
        cout << *j << (j == prev(i->second.end()) ? " " : ", ");
      }
      cout << "}" << endl;
    }
  }

  void calculateFollow()
  {
    follow.insert(make_pair(sRule, set<string>({"$"})));

    for (auto i = ruleList.begin(); i != ruleList.end(); i++)
    {
      if (*i == sRule) continue;
      follow.insert(make_pair(*i, set<string>()));
    }

    for (auto i = ruleList.begin(); i != ruleList.end(); i++)
    {
      vector<vector<string>> currRule = rules.find(*i)->second;
      for (auto j = currRule.begin(); j != currRule.end(); j++)
      {
        vector<string> currSubRule = *j;
        for (auto k = currSubRule.end() - 1; k != currSubRule.begin() - 1; k--)
        {
          if (k == currSubRule.end() - 1) continue;
          if (*k == "epsilon" || terminals.count(*k)) continue;
          vector<string>::iterator epsilonDepth = k + 1;
          for (auto l = k + 1; l != currSubRule.end(); l++)
          {
            // K is equivalent of B whereas L is A_i
            if (!first.find(*l)->second.count("epsilon")) break;
            epsilonDepth = l + 1;
          }
          for (auto a = k; a != epsilonDepth; a++)
          {
            if (a + 1 == currSubRule.end()) break;
            set<string> firstK = first.find(*(a + 1))->second;
            set<string> temp = set<string>(firstK.begin(), firstK.end());
            temp.erase("epsilon");
            follow.find(*k)->second.insert(temp.begin(), temp.end());
          }
        }
      }
    }

    bool changed = true;
    while (changed)
    {
      changed = false;
      for (auto i = ruleList.begin(); i != ruleList.end(); i++)
      {
        vector<vector<string>> currRule = rules.find(*i)->second;
        for (auto j = currRule.begin(); j != currRule.end(); j++)
        {
          vector<string> currSubRule = *j;
          bool hasEp = true;
          for (auto k = currSubRule.end() - 1; k != currSubRule.begin() - 1 && hasEp; k--)
          {

            if (!first.find(*k)->second.count("epsilon")) hasEp = false;
            if (terminals.count(*k) || (*k) == "epsilon") break;
            size_t sizeB = follow.find(*k)->second.size();
            follow.find(*k)->second.insert(follow.find(*i)->second.begin(), follow.find(*i)->second.end());
            if (sizeB != follow.find(*k)->second.size()) changed = true;
          }
        }
      }
    }
  }

  void printFollow()
  {
    cout << "FOLLOW SETS:" << endl;
    for (auto i = follow.begin(); i != follow.end(); i++)
    {
      cout << "\t" << i->first << " = { ";
      for (auto j = i->second.begin(); j != i->second.end(); j++)
      {
        cout << *j << (j == prev(i->second.end()) ? " " : ", ");
      }
      cout << "}" << endl;
    }
  }

  bool checkPredictiveParse()
  {
    for (auto i = ruleList.begin(); i != ruleList.end(); i++)
    {
      if (!first.find(*i)->second.count("epsilon")) continue;
      set<string> intersect;
      set<string> s1 = first.find(*i)->second;
      set<string> s2 = follow.find(*i)->second;
      set_intersection(s1.begin(), s1.end(), s2.begin(), s2.end(), inserter(intersect, intersect.begin()));
      if (!intersect.empty()) return false;
    }

    for (auto i = ruleList.begin(); i != ruleList.end(); i++)
    {
      vector<vector<string>> compoundRule = rules.find(*i)->second;
      for (auto j = compoundRule.begin(); j != compoundRule.end() - 1; j++)
      {
        for (auto k = j + 1; k != compoundRule.end(); k++)
        {
          set<string> firstJ;
          set<string> firstK;
          // cout<<"COMP"<<endl;
          for (auto l = j->begin(); l != j->end(); l++)
          {
            firstJ.insert(first.find(*l)->second.begin(), first.find(*l)->second.end());
            if (!first.find(*l)->second.count("epsilon")) break;
          }
          for (auto l = k->begin(); l != k->end(); l++)
          {
            firstK.insert(first.find(*l)->second.begin(), first.find(*l)->second.end());
            if (!first.find(*l)->second.count("epsilon")) break;
          }
          set<string> intersect;
          set_intersection(firstJ.begin(), firstJ.end(), firstK.begin(), firstK.end(), inserter(intersect, intersect.begin()));
          if (!intersect.empty()) return false;
        }
      }
    }

    return true;
  }

  void generateParser(char *fName)
  {
    fstream parserFile;
    parserFile.open(fName, ios::out | ios::trunc);

    parserFile << "void Parser::parse_input()\n{\n    parse_" << sRule << "();\n    expect(END_OF_FILE);\n}" << endl;

    for (auto rIter = ruleList.begin(); rIter != ruleList.end(); rIter++)
    {
      string r = *rIter;
      parserFile << "void Parser::parse_" << r << "()\n{\n    Token t = lexer.peek(1);\n\n";
      bool hasEp = false;
      vector<vector<string>>::iterator epIter = rules.find(r)->second.begin() - 1;
      for (auto i = rules.find(r)->second.begin(); i != rules.find(r)->second.end(); i++)
      {
        // construct RHS_i
        set<string> firstRHS;
        bool genEpsilon = true;
        for (auto j = i->begin(); j != i->end(); j++)
        {
          if (genEpsilon)
          {
            firstRHS.insert(first.find(*j)->second.begin(), first.find(*j)->second.end());
          }
          if (!first.find(*j)->second.count("epsilon"))
          {
            genEpsilon = false;
          }
          else
          {
            firstRHS.erase("epsilon");
          }
        }

        // skip if produces epsilon - save for FOLLOW check
        if (genEpsilon)
        {
          hasEp = true;
          epIter = i;
          continue;
        }

        if (i == rules.find(r)->second.begin())
        {
          parserFile << "    if(";
        }
        else
        {
          parserFile << "    else if(";
        }

        for (auto j = firstRHS.begin(); j != firstRHS.end(); j++)
        {
          parserFile << "t.token_type == " << *j;
          if (j != prev(firstRHS.end()))
          {
            parserFile << " || ";
          }
        }
        parserFile << ")\n    {" << endl;
        for (auto j = i->begin(); j != i->end(); j++)
        {
          if (*j == "epsilon") continue;
          if (terminals.count(*j))
          {
            parserFile << "        expect(" << *j << ");" << endl;
          }
          else
          {
            parserFile << "        parse_" << *j << "();" << endl;
          }
        }
        parserFile << "    }" << endl;
      }
      if (hasEp)
      {
        auto ruleVec = rules.find(r)->second;
        if (ruleVec.size() - 1 == 0)
        {
          parserFile << "    if(";
        }
        else
        {
          parserFile << "    else if(";
        }

        for (auto j = follow.find(r)->second.begin(); j != follow.find(r)->second.end(); j++)
        {
          string token = (*j == "$") ? "EOF" : *j;
          parserFile << "t.token_type == " << token;
          if (j != prev(follow.find(r)->second.end()))
          {
            parserFile << " || ";
          }
        }
        parserFile << ")\n    {" << endl;
        bool onlyEpsilons = true;
        for (auto j = epIter->begin(); j != epIter->end(); j++)
        {
          if (*j == "epsilon") continue;
          onlyEpsilons = false;
          if (terminals.count(*j))
          {
            parserFile << "        expect(" << *j << ");" << endl;
          }
          else
          {
            parserFile << "        parse_" << *j << "();" << endl;
          }
        }
        if (onlyEpsilons) parserFile << "        return;" << endl;
        parserFile << "    }" << endl;
      }
      parserFile << "    else{\n        syntax_error();\n    }\n}" << endl;
    }
    parserFile.close();
  }
  void printGrammar()
  {
    cout << "GRAMMAR: " << endl;
    for (auto i = ruleList.begin(); i != ruleList.end(); i++)
    {
      vector<vector<string>> compoundRule = rules.find(*i)->second;
      cout << "\t" << rules.find(*i)->first << " ";
      cout << "\u2192";
      cout << " ";
      for (auto j = compoundRule.begin(); j != compoundRule.end(); j++)
      {
        for (auto k = j->begin(); k != j->end(); k++)
        {
          cout<<*k<<" ";
        }
        if(j!=compoundRule.end() - 1) cout<<" | ";
      }
      cout << endl;
    }
  }
};