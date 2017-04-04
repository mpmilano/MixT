#pragma once
#include "AST_split.hpp"
#include <iostream>

namespace myria {
namespace mtl {
namespace split_phase {

template <typename l, typename l2, typename y, typename v, typename e>
void
print_ast(std::ostream& o, const typename AST<l>::template Binding<l2, y, v, e>&)
{
  print_varname(o, v{});
  o << " = ";
  print_ast<l>(o, e{});
}

template <typename l, typename y, typename s, typename f>
void
print_ast(std::ostream& o, const typename AST<l>::template Expression<y, typename AST<l>::template FieldReference<s, f>>&)
{
  print_ast<l>(o, s{});
  o << "." << f{};
}

template <typename l, typename y, typename v>
void
print_ast(std::ostream& o, const typename AST<l>::template Expression<y, typename AST<l>::template VarReference<v>>&)
{
  print_varname(o, v{});
}

template <typename l, int i>
void
print_ast(std::ostream& o, const typename AST<l>::template Expression<int, typename AST<l>::template Constant<i>>&)
{
  o << i;
}

template <typename l, typename y, char op, typename L, typename R>
void
print_ast(std::ostream& o, const typename AST<l>::template Expression<y, typename AST<l>::template BinOp<op, L, R>>&)
{
  static const std::string opstr{ 1, op };
  print_ast<l>(o, L{});
  o << " " << opstr << " ";
  print_ast<l>(o, R{});
}

template <typename l, typename b, typename body>
void
print_ast(std::ostream& o, const typename AST<l>::template Statement<typename AST<l>::template Let<b, body>>&, const std::string& tab)
{
  o << tab << "let [";
  print_ast<l>(o, b{});
  o << "] in "
    << "{";
  print_ast<l>(o, body{}, tab);
  o << "}";
}

template <typename l, typename b, typename body>
void
print_ast(std::ostream& o, const typename AST<l>::template Statement<typename AST<l>::template LetRemote<b, body>>&, const std::string& tab)
{
  o << tab << "let remote [";
  print_ast<l>(o, b{});
  o << "] in "
    << "{";
  print_ast<l>(o, body{}, tab);
  o << "}";
}

template <typename l, typename L, typename R>
void
print_ast(std::ostream& o, const typename AST<l>::template Statement<typename AST<l>::template Assignment<L, R>>&, const std::string& tab)
{
	o << tab;
  print_ast<l>(o, L{});
  o << " = ";
  print_ast<l>(o, R{});
}

template <typename l, char... var>
void
print_ast(std::ostream& o, const typename AST<l>::template Statement<typename AST<l>::template IncrementOccurance<String<var...>>>&, const std::string& tab)
{
  o << tab << "use(";
  print_varname(o, String<var...>{});
  o << ")";
}

template <typename l, typename c, typename t, typename e>
void
print_ast(std::ostream& o, const typename AST<l>::template Statement<typename AST<l>::template If<c, t, e>>&, const std::string& tab)
{
  o << "if (";
  print_ast<l>(o, c{});
  o << ") {";
  print_ast<l>(o, t{}, tab);
  o << "} else {";
  print_ast<l>(o, e{}, tab);
  o << "}";
}

template <typename l, typename c, typename t, char... name>
void
print_ast(std::ostream& o, const typename AST<l>::template Statement<typename AST<l>::template While<c, t, name...>>&, const std::string& tab)
{
  o << "while (";
  print_ast<l>(o, c{});
  o << ") {";
  print_ast<l>(o, t{}, tab);
  o << "} ";
}

template <typename l, typename t, char... name>
void
print_ast(std::ostream& o, const typename AST<l>::template Statement<typename AST<l>::template ForEach<t, name...>>&, const std::string& tab)
{
  o << "ForEach (";
  print_varname(o, String<name...>{});
  o << ") {";
  print_ast<l>(o, t{}, tab);
  o << "} ";
}

template <typename l, typename... Seq>
void
print_ast(std::ostream& o, const typename AST<l>::template Statement<typename AST<l>::template Sequence<Seq...>>&, const std::string& _tab)
{
  auto tab = _tab + std::string{ "  " };
  using namespace std;
  auto print = [&](const auto& e) {
    o << tab;
    print_ast<l>(o, e, tab);
    o << ";" << endl;
    return nullptr;
  };
  o << "{" << endl;
  auto ignore = { nullptr, nullptr, print(Seq{})... };
	(void)ignore;
  o << _tab << "}";
}

template <typename s>
auto& operator<<(std::ostream& o, const typename AST<Label<bottom>>::template Statement<s>& ast)
{
  print_ast(o, ast, "");
  return o;
}

template <typename y, typename e>
auto& operator<<(std::ostream& o, const typename AST<Label<bottom>>::template Expression<y, e>& ast)
{
  print_ast<Label<bottom>>(o, ast);
  return o;
}

template <typename l2, typename y, typename v, typename e>
auto& operator<<(std::ostream& o, const typename AST<Label<bottom>>::template Binding<l2, y, v, e>& ast)
{
  print_ast(o, ast);
  return o;
}
template <typename s>
auto& operator<<(std::ostream& o, const typename AST<Label<top>>::template Statement<s>& ast)
{
  print_ast(o, ast, "");
  return o;
}

template <typename y, typename e>
auto& operator<<(std::ostream& o, const typename AST<Label<top>>::template Expression<y, e>& ast)
{
  print_ast<Label<top>>(o, ast);
  return o;
}

template <typename l2, typename y, typename v, typename e>
auto& operator<<(std::ostream& o, const typename AST<Label<top>>::template Binding<l2, y, v, e>& ast)
{
  print_ast(o, ast);
  return o;
}
}
}
}
namespace mutils {
}
