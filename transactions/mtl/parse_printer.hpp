#pragma once
#include "AST_parse.hpp"
#include <iostream>

namespace myria {
namespace mtl {
namespace parse_phase {

template <typename v, typename e>
void
print_ast(std::ostream& o, const Binding<v, e>&)
{
  print_varname(o, v{});
  o << "=" << e{};
}

template <typename s, typename f>
void
print_ast(std::ostream& o, const Expression<FieldReference<s, f>>&)
{
  o << s{} << "." << f{};
}

template <typename v>
void
print_ast(std::ostream& o, const Expression<VarReference<v>>&)
{
  print_varname(o, v{});
}

template <long long i>
void
print_ast(std::ostream& o, const Expression<Constant<i>>&)
{
  o << i;
}

template <char op, typename L, typename R>
void
print_ast(std::ostream& o, const Expression<BinOp<op, L, R>>&)
{
  static const std::string opstr{ 1, op };
  o << L{} << " " << opstr << " " << R{};
}

template <typename b, typename body>
void
print_ast(std::ostream& o, const Statement<Let<b, body>>&)
{
  o << "let " << b{} << " in "
    << "{" << body{} << "}";
}

template <typename b, typename body>
void
print_ast(std::ostream& o, const Statement<LetRemote<b, body>>&)
{
  o << "let remote " << b{} << " in "
    << "{" << body{} << "}";
}

template <typename L, typename R>
void
print_ast(std::ostream& o, const Statement<Assignment<L, R>>&)
{
  o << L{} << " = " << R{};
}

template <typename c, typename t, typename e>
void
print_ast(std::ostream& o, const Statement<If<c, t, e>>)
{
  o << "if"
    << " (" << c{} << ") {" << t{} << "} else {" << e{} << "}";
}

template <typename c, typename t>
void
print_ast(std::ostream& o, const Statement<While<c, t>>)
{
  o << "while"
    << " (" << c{} << ") {" << t{} << "} ";
}

template <typename... Seq>
void
print_ast(std::ostream& o, Statement<Sequence<Seq...>>)
{
  using namespace std;
  auto print = [&](const auto& e) {
    o << e << ";" << endl;
    return nullptr;
  };
  o << "{" << endl;
  auto ignore = { nullptr, nullptr, print(Seq{})... };
  o << "}";
}

template <typename AST>
auto& operator<<(std::ostream& o, const AST& ast)
{
  print_ast(o, ast);
  return o;
}
}
}
}
