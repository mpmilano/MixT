#pragma once
#include "mtl/AST_parse.hpp"
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

template <typename s, typename f>
void
print_ast(std::ostream& o, const Expression<FieldPointerReference<s, f>>&)
{
  o << s{} << "->" << f{};
}

template <typename s>
void
print_ast(std::ostream& o, const Expression<Dereference<s>>&)
{
  o << "*" << s{};
}

template <typename v>
void
print_ast(std::ostream& o, const Expression<VarReference<v>>&)
{
  print_varname(o, v{});
}

template <int i>
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

  template <typename Name, typename Hndl>
void
  print_ast(std::ostream& o, const Expression<Operation<Name, Hndl, operation_args_exprs<>, operation_args_varrefs<> > >&)
{
  o << Hndl{} << "." << Name{} << "()";
}

	
	template <typename Name, typename Hndl, typename... vfs>
void
print_ast(std::ostream& o, const Expression<Operation<Name, Hndl, operation_args_exprs<>, operation_args_varrefs<vfs...> >>&)
{
  o << Hndl{} << "." << Name{} << "(";
  ((o <<  "," << vfs{} ), ...);
	o << ")";
}

  template <typename Name, typename Hndl, typename vfs, typename a1, typename... args>
void
  print_ast(std::ostream& o, const Expression<Operation<Name, Hndl, operation_args_exprs<a1, args...>, vfs>>&)
{
  o << Hndl{} << "." << Name{} << "(" << a1{};
  ((o <<  "," << args{} ), ...);
  o << ")";
}


  template<typename... args>
  void print_ast(std::ostream& o, const operation_args_exprs<args...>){
    o << "exprs: ";
    ((o << args{} << ","),...);
  }

  template<typename... args>
  void print_ast(std::ostream& o, const operation_args_varrefs<args...>){
    o << "vars: ";
    ((o << args{} << ","),...);
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

template <typename h>
void
print_ast(std::ostream& o, const Expression<IsValid<h>>&)
{
	o << h{} << ".isValid()";
}

	
	template <typename l, typename h>
void
	print_ast(std::ostream& o, const Expression<Endorse<l,h>>&)
{
	o << h{} << ".endorse(" << l{} << ")";
}

	template <typename l, typename h>
void
	print_ast(std::ostream& o, const Expression<Ensure<l,h>>&)
{
	o << h{} << ".ensure(" << l{} << ")";
}

	
template <typename L, typename R>
void
print_ast(std::ostream& o, const Statement<Assignment<L, R>>&)
{
  o << L{} << " = " << R{};
}

template <typename R>
void
print_ast(std::ostream& o, const Statement<Return<R>>&)
{
  o << "return " << R{};
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
  (print(Seq{}), ...);
  o << "}";
}

template <typename AST>
auto&
operator<<(std::ostream& o, const AST& ast)
{
  print_ast(o, ast);
  return o;
}
}
}
}
