#pragma once
#include "AST_typecheck.hpp"
#include <iostream>

namespace myria {
namespace mtl {
namespace typecheck_phase {

template <typename l, typename y, typename v, typename e>
void
print_ast(std::ostream& o, const Binding<l, y, v, e>&)
{
  print_varname(o, v{});
  o << "@" << l{} << " = " << e{};
}

template <typename l, typename y, typename s, typename f>
void
print_ast(std::ostream& o, const Expression<l, y, FieldReference<s, f>>&)
{
  o << s{} << "." << f{} << "@" << l{};
}

template <typename l, typename y, typename v>
void
print_ast(std::ostream& o, const Expression<l, y, VarReference<v>>&)
{
  print_varname(o, v{});
  o << "@" << l{};
}

template <long long i>
void
print_ast(std::ostream& o, const Expression<Label<top>, long long, Constant<i>>&)
{
  o << i;
}

template <typename l, typename y, char op, typename L, typename R>
void
print_ast(std::ostream& o, const Expression<l, y, BinOp<op, L, R>>&)
{
  static const std::string opstr{ 1, op };
  o << L{} << " " << opstr << "@" << l{} << " " << R{};
}

template <typename l, typename b, typename body>
void
print_ast(std::ostream& o, const Statement<l, Let<b, body>>&, const std::string& tab)
{
  o << tab << "let@" << l{} << " " << b{} << " in "
    << "{";
  print_ast(o, body{}, tab);
  o << "}";
}

template <typename l, typename b, typename body>
void
print_ast(std::ostream& o, const Statement<l, LetRemote<b, body>>&, const std::string& tab)
{
  o << tab << "let remote@" << l{} << " " << b{} << " in "
    << "{";
  print_ast(o, body{}, tab);
  o << "}";
}

template <typename l, typename L, typename R>
void
print_ast(std::ostream& o, const Statement<l, Assignment<L, R>>&, const std::string& tab)
{
  o << L{} << " =@" << l{} << " " << R{};
}

template <typename l, typename c, typename t, typename e>
void
print_ast(std::ostream& o, const Statement<l, If<c, t, e>>&, const std::string& tab)
{
  o << "if@" << l{} << " (" << c{} << ") {";
  print_ast(o, t{}, tab);
  o << "} else {";
  print_ast(o, e{}, tab);
  o << "}";
}

template <typename l, typename c, typename t, char... name>
void
print_ast(std::ostream& o, const Statement<l, While<c, t, name...>>&, const std::string& tab)
{
  o << "while@" << l{} << " (" << c{} << ") {";
  print_ast(o, t{}, tab);
  o << "} ";
}

template <typename l, typename... Seq>
void
print_ast(std::ostream& o, const Statement<l, Sequence<Seq...>>&, const std::string& _tab)
{
  auto tab = _tab + std::string{ "  " };
  using namespace std;
  auto print = [&](const auto& e) {
    o << tab;
    print_ast(o, e, tab);
    o << ";" << endl;
    return nullptr;
  };
  o << "{ label::" << l{} << ";" << endl;
  auto ignore = { nullptr, nullptr, print(Seq{})... };
  o << _tab << "}";
}

template <typename l, typename s>
auto& operator<<(std::ostream& o, const Statement<l, s>& ast)
{
  print_ast(o, ast, "");
  return o;
}

template <typename l, typename y, typename e>
auto& operator<<(std::ostream& o, const Expression<l, y, e>& ast)
{
  print_ast(o, ast);
  return o;
}

template <typename l, typename y, typename v, typename e>
auto& operator<<(std::ostream& o, const Binding<l, y, v, e>& ast)
{
  print_ast(o, ast);
  return o;
}
}
}
}
