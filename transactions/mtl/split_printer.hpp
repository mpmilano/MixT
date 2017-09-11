#pragma once
#include "AST_split.hpp"
#include <iostream>
#include <sstream>
#include "split_context.hpp"

namespace myria {
namespace mtl {
namespace split_phase {

	BEGIN_SPLIT_CONTEXT(split_printer);
	
	template <typename l2, typename y, typename v, typename e>
	static void print_ast(std::ostream& o, const Binding<l2, y, v, e>&)
	{
		print_varname(o, v{});
		o << " : " << mutils::typename_str<y>::f() << " = ";
		print_ast(o, e{});
	}

	template <typename y, typename s, typename f>
	static void print_ast(std::ostream& o, const Expression<y, FieldReference<s, f>>&)
	{
		print_ast(o, s{});
		o << "." << f{};
	}
  
template < typename y, typename v>
static void print_ast(std::ostream& o, const Expression<y, VarReference<v>>&)
{
  o << "(";
  print_varname(o, v{});
  o << " : " << mutils::typename_str<y>::f() << ")";
}

template < int i>
static void print_ast(std::ostream& o, const Expression<int, Constant<i>>&)
{
  o << i;
}

static void print_ast(std::ostream& o, const Expression<tracker::Tombstone, GenerateTombstone>&)
{
  o << "random_tombstone()";
}


template < typename y, char op, typename L, typename R>
static void print_ast(std::ostream& o, const Expression<y, BinOp<op, L, R>>&)
{
  static const std::string opstr{{ op, (op == '=' ? '=' : 0) , 0 }};
  print_ast(o, L{});
  o << " " << opstr << " ";
  print_ast(o, R{});
}

	MATCH_CONTEXT(is_sequence) {
		template<typename... S>
			MATCHES(Statement<Sequence<S...>>) -> RETURNVAL(true);
		template<typename S>
			MATCHES(Statement<S>) -> RETURNVAL(false);
	};
	
template < typename b, typename _body>
static void print_ast(std::ostream& o, const Statement<Let<b, _body>>&, const std::string& tab)
{
	
  o << tab << "let [";
  print_ast(o, b{});
  o << "] in "
    << "{";
	using body = std::conditional_t<MATCH(is_sequence,_body)::value, _body , Statement<Sequence<_body> > >;
  print_ast(o, body{}, tab);
  o << "}";
}

template < typename b, typename _body>
static void print_ast(std::ostream& o, const Statement<LetRemote<b, _body>>&, const std::string& tab)
{
	using body = std::conditional_t<MATCH(is_sequence,_body)::value, _body , Statement<Sequence<_body> > >;
  o << tab << "let remote [";
  print_ast(o, b{});
  o << "] in "
    << "{";
  print_ast(o, body{}, tab);
  o << "}";
}

	template < typename y, typename h>
static void print_ast(std::ostream& o, const Expression<y,IsValid<h>>&)
{
	print_ast(o,h{});
	o << ".isValid()";
}

template < typename oper_name, typename Hndl, typename... args>
static void print_ast(std::ostream& o, const Statement<Operation<oper_name,Hndl,args...>>&, const std::string& tab)
{
	o << tab;
	print_ast(o,Hndl{});
	o << "." << oper_name{} << "(";
	((print_ast(o,args{}),o << ","),...);
	o << ")";
}

template < typename y, typename oper_name, typename Hndl, typename... args>
static void print_ast(std::ostream& o, const Expression<y,Operation<oper_name,Hndl,args...>>&)
{
	print_ast(o,Hndl{});
	o << "." << oper_name{} << "(";
	((print_ast(o,args{}),o << ","),...);
	o << ")";
}

	
template < typename L, typename R>
static void print_ast(std::ostream& o, const Statement<Assignment<L, R>>&, const std::string& tab)
{
	o << tab;
  print_ast(o, L{});
  o << " = ";
  print_ast(o, R{});
}

template < typename R>
static void print_ast(std::ostream& o, const Statement<Return<R>>&, const std::string& tab)
{
  o << tab << "return ";
  print_ast(o, R{});
}
  
template < typename R>
static void print_ast(std::ostream& o, const Statement<AccompanyWrite<R>>&, const std::string& tab)
{
  o << tab << "track ";
  print_ast(o, R{});
}

  template < typename e>
static void print_ast(std::ostream& o, const Statement<WriteTombstone<e>>&, const std::string& tab)
{
  o << tab << "write(";
  print_ast(o,e{});
  o << ")";
}

template < char... var>
static void print_ast(std::ostream& o, const Statement<IncrementOccurance<String<var...>>>&, const std::string& tab)
{
  o << tab << "use(";
  print_varname(o, String<var...>{});
  o << ")";
}

template < char... var>
static void print_ast(std::ostream& o, const Statement<IncrementRemoteOccurance<String<var...>>>&, const std::string& tab)
{
  o << tab << "use remote(";
  print_varname(o, String<var...>{});
  o << ")";
}

template < typename var>
static void print_ast(std::ostream& o, const Statement<RefreshRemoteOccurance<var>>&, const std::string& tab)
{
  o << tab << "refresh remote(";
  print_ast(o,var{});
  o << ")";
}

template < typename c, typename _t, typename _e>
static void print_ast(std::ostream& o, const Statement<If<c, _t, _e>>&, const std::string& tab)
{
	using t = std::conditional_t<MATCH(is_sequence,_t)::value, _t , Statement<Sequence<_t> > >;
	using e = std::conditional_t<MATCH(is_sequence,_e)::value, _e , Statement<Sequence<_e> > >;
  o << "if (";
  print_ast(o, c{});
  o << ") {";
  print_ast(o, t{}, tab);
  o << "} else {";
  print_ast(o, e{}, tab);
  o << "}";
}

template < typename c, typename _t, char... name>
static void print_ast(std::ostream& o, const Statement<While<c, _t, name...>>&, const std::string& tab)
{
	using t = std::conditional_t<MATCH(is_sequence,_t)::value, _t , Statement<Sequence<_t> > >;
  o << "while (";
  print_ast(o, c{});
  o << ") {";
  print_ast(o, t{}, tab);
  o << "} ";
}

template < typename _t, char... name>
static void print_ast(std::ostream& o, const Statement<ForEach<_t, name...>>&, const std::string& tab)
{
	using t = std::conditional_t<MATCH(is_sequence,_t)::value, _t , Statement<Sequence<_t> > >;
  o << "ForEach (";
  print_varname(o, String<name...>{});
  o << ") {";
  print_ast(o, t{}, tab);
  o << "} ";
}

template < typename... Seq>
static void print_ast(std::ostream& o, const Statement<Sequence<Seq...>>&, const std::string& _tab)
{
  auto tab = _tab + std::string{ "  " };
  using namespace std;
  auto print = [&](const auto& e) {
    o << tab;
    print_ast(o, e, tab);
    o << ";" << endl;
    return nullptr;
  };
  o << "{" << endl;
  auto ignore = { nullptr, nullptr, print(Seq{})... };
	(void)ignore;
  o << _tab << "}";
}

	END_SPLIT_CONTEXT;
		

}
}
}

