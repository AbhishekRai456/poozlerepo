#include "NfaBuilder.hpp"
#include "NfaMatcher.hpp"
#include "RegexPostfix.hpp"
#include "RegexTokenizer.hpp"
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

int total = 0, passed = 0, failed = 0;

void run(const string &desc, const string &pat, const string &text,
         bool expected) {
  total++;
  try {
    Tokenizer tok(pat);
    auto tokens = tok.tokenize();
    auto postfix = Postfix::convert(tokens);
    NfaBuilder b;
    State *s = b.build(postfix);
    NfaMatcher m(s);
    bool got = m.match(text).matched;
    if (got == expected) {
      passed++;
      cout << "[PASS] " << desc << "\n";
    } else {
      failed++;
      cout << "[FAIL] " << desc << " | pat=\"" << pat << "\" text=\"" << text
           << "\" expected=" << expected << " got=" << got << "\n";
    }
  } catch (const exception &e) {
    failed++;
    cout << "[FAIL] " << desc << " | EXCEPTION: " << e.what() << "\n";
  }
}

void run_error(const string &desc, const string &pat) {
  total++;
  try {
    Tokenizer tok(pat);
    auto tokens = tok.tokenize();
    Postfix::convert(tokens);
    failed++;
    cout << "[FAIL] " << desc << " | Expected error but none thrown\n";
  } catch (const exception &) {
    passed++;
    cout << "[PASS] " << desc << " (correctly rejected)\n";
  }
}

int main() {
  cout << "\n=== ADDITIONAL EDGE CASE TESTS ===\n\n";

  // Alternation precedence
  run("a|bc first branch", "a|bc", "a", true);
  run("a|bc second branch", "a|bc", "bc", true);
  run("ab|c first branch", "ab|c", "ab", true);
  run("ab|c second branch", "ab|c", "c", true);
  run("a(b|c)d middle b", "a(b|c)d", "abd", true);
  run("a(b|c)d middle c", "a(b|c)d", "acd", true);
  run("a(b|c)d no match", "a(b|c)d", "ad", false);
  run("a|b|c|d|e first", "a|b|c|d|e", "a", true);
  run("a|b|c|d|e last", "a|b|c|d|e", "e", true);
  run("a|b|c|d|e no match", "a|b|c|d|e", "f", false);

  // Overlapping alternation
  run("a|aa short match", "a|aa", "a", true);
  run("a|aa long match", "a|aa", "aa", true);
  run("(a|aa)* empty", "(a|aa)*", "", true);
  run("(a|aa)* one a", "(a|aa)*", "a", true);
  run("(a|ab)* empty", "(a|ab)*", "", true);
  run("(ab|a)* empty", "(ab|a)*", "", true);

  // Mixed quantifiers
  run("a+b* just a", "a+b*", "a", true);
  run("a+b* a and b", "a+b*", "aaabb", true);
  run("a+b* no a", "a+b*", "b", false);
  run("a?b+ just b", "a?b+", "b", true);
  run("a?b+ ab", "a?b+", "ab", true);
  run("a?b+ no b", "a?b+", "a", false);
  run("(a|b)+c basic", "(a|b)+c", "ac", true);
  run("(a|b)+c multi", "(a|b)+c", "abbc", true);
  run("(a|b)+c no c", "(a|b)+c", "ab", false);

  // Range quantifiers
  run("a{1,2} one", "a{1,2}", "a", true);
  run("a{1,2} two", "a{1,2}", "aa", true);
  run("a{1,2} zero", "a{1,2}", "", false);
  run("a{3,5} three", "a{3,5}", "aaa", true);
  run("a{3,5} five", "a{3,5}", "aaaaa", true);
  run("a{3,5} two", "a{3,5}", "aa", false);
  run("(a|b){2,4} two", "(a|b){2,4}", "ab", true);
  run("(a|b){2,4} four", "(a|b){2,4}", "abab", true);
  run("(a|b){2,4} one", "(a|b){2,4}", "a", false);
  run("(abc){2,3} two", "(abc){2,3}", "abcabc", true);
  run("(abc){2,3} one", "(abc){2,3}", "abc", false);
  run("[a-z]{2,5} two", "[a-z]{2,5}", "ab", true);
  run("[a-z]{2,5} five", "[a-z]{2,5}", "abcde", true);
  run("[a-z]{2,5} one", "[a-z]{2,5}", "a", false);

  // Char class edge cases
  run("[a] matches a", "[a]", "a", true);
  run("[a] no match b", "[a]", "b", false);
  run("[A-Z0-9] upper", "[A-Z0-9]", "B", true);
  run("[A-Z0-9] digit", "[A-Z0-9]", "5", true);
  run("[A-Z0-9] lower", "[A-Z0-9]", "b", false);
  run("[A-Fa-f0-9] hex upper", "[A-Fa-f0-9]", "A", true);
  run("[A-Fa-f0-9] hex lower", "[A-Fa-f0-9]", "f", true);
  run("[A-Fa-f0-9] digit", "[A-Fa-f0-9]", "9", true);
  run("[A-Fa-f0-9] no match", "[A-Fa-f0-9]", "g", false);

  // Dot quantifier
  run(".{2,4} two chars", ".{2,4}", "ab", true);
  run(".{2,4} four chars", ".{2,4}", "abcd", true);
  run(".{2,4} one char", ".{2,4}", "a", false);
  run("(.)* empty", "(.)*", "", true);
  run("(.)* some", "(.)*", "abc", true);

  // Anchor combinations
  run("^a$ exact", "^a$", "a", true);
  run("^a$ extra", "^a$", "ab", false);
  run("^(a|b)*$ empty", "^(a|b)*$", "", false);
  run("^(a|b)*$ valid", "^(a|b)*$", "abab", true);
  run("^.*$ no newline", "^.*$", "abc", true);

  // Nested captures
  run("((a)b) match", "((a)b)", "ab", true);
  run("((a)b) no match", "((a)b)", "b", false);
  run("(a(b(c))) full", "(a(b(c)))", "abc", true);
  run("(a|b)c(d|e) bd", "(a|b)c(d|e)", "bcd", true);
  run("(a|b)c(d|e) ae", "(a|b)c(d|e)", "ace", true);
  run("(a|b)c(d|e) no match", "(a|b)c(d|e)", "bc", false);

  // Large literal
  run("abcdefghij match", "abcdefghij", "abcdefghij", true);
  run("abcdefghij partial", "abcdefghij", "abcdefghi", false);

  // Error cases - these should throw
  run_error("invalid range a{2,1}", "a{2,1}");
  run_error("empty parens ()", "()");
  run_error("unclosed paren (a", "(a");

  cout << "\n=== SUMMARY ===\n";
  cout << "Total: " << total << " | Passed: " << passed
       << " | Failed: " << failed << "\n";
  return failed > 0 ? 1 : 0;
}