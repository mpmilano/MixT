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

template <int i>
void
print_ast(std::ostream& o, const Expression<Label<top>, int, Constant<i>>&)
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

template <typename l, typename y, typename h>
void print_ast(std::ostream& o, const Expression<l, y, IsValid<h>>&)
{
	o << h{} << ".isValid@" << l{} << "()";
}


template <typename l, typename b, typename body>
void
print_ast(std::ostream& o, const Statement<l, Let<b, body>>&, const std::string& tab)
{
  o << tab << "let@" << l{} << " " << b{} << " in "
    << "{";
  print_ast(o, body{}, tab);
  o << tab << "}";
}

template <typename l, typename b, typename body>
void
print_ast(std::ostream& o, const Statement<l, LetRemote<b, body>>&, const std::string& tab)
{
  o << tab << "let remote@" << l{} << " " << b{} << " in "
    << "{";
  print_ast(o, body{}, tab);
  o << tab << "}";
}


	template <typename l, typename y, typename oper_name, typename Hndl, typename... args>
void
	print_ast(std::ostream& o, const Expression<l, y, Operation<oper_name,Hndl,args...>>&, const std::string& tab)
{
	o << tab;
	print_ast(o,Hndl{});
	o << ". @" << l{} << " " << oper_name{} << "(";
	(print_ast(o,args{}),...);
	o << ")";
}
	template <typename l, typename oper_name, typename Hndl, typename... args>
void
	print_ast(std::ostream& o, const Statement<l, Operation<oper_name,Hndl,args...>>&, const std::string& tab)
{
	o << tab;
	print_ast(o,Hndl{});
	o << ". @" << l{} << " " << oper_name{} << "(";
	(print_ast(o,args{}),...);
	o << ")";
}

	
template <typename l, typename L, typename R>
void
print_ast(std::ostream& o, const Statement<l, Assignment<L, R>>&, const std::string& tab)
{
  o << tab << L{} << " =@" << l{} << " " << R{};
}

template <typename l, typename R>
void
print_ast(std::ostream& o, const Statement<l, Return<R> >&, const std::string& tab)
{
  o << tab << "return ";
  print_ast(o, R{});
}


template <typename l, typename c, typename t, typename e>
void
print_ast(std::ostream& o, const Statement<l, If<c, t, e>>&, const std::string& tab)
{
  o << tab << "if@" << l{} << " (" << c{} << ") {";
  print_ast(o, t{}, tab);
  o << tab << "} else {";
  print_ast(o, e{}, tab);
  o << tab << "}";
}

template <typename l, typename c, typename t, char... name>
void
print_ast(std::ostream& o, const Statement<l, While<c, t, name...>>&, const std::string& tab)
{
  o << tab << "while@" << l{} << " (" << c{} << ") {";
  print_ast(o, t{}, tab);
  o << tab << "} ";
}

template <typename l, typename... Seq>
void
print_ast(std::ostream& o, const Statement<l, Sequence<Seq...>>&, const std::string& _tab)
{
  auto tab = _tab + std::string{ "  " };
  using namespace std;
  const auto print = [tab](auto& o, const auto& e) {
    o << tab;
    print_ast(o, e, tab);
    o << ";" << endl;
    return nullptr;
  };
  o << "{ label::" << l{} << ";" << endl;
  (print(o,Seq{}),...);
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
