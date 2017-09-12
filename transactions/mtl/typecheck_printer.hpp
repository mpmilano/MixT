#pragma once
#include "mtl/AST_typecheck.hpp"
#include <iostream>

namespace myria {
namespace mtl {
namespace typecheck_phase {

template <typename l, typename y, typename v, typename e>
auto
print_ast(const Binding<l, y, v, e>&)
{
	using namespace mutils;
  return print_varname(v{}).append(String<'/','*','@'>{}).append(print_label(l{}).append(String<'*','/'>{})).append(String<'='>{}).append(print_ast(e{}));
}

template <typename l, typename y, typename s, typename f>
auto
print_ast(const Expression<l, y, FieldReference<s, f>>&)
{
	using namespace mutils;
	return print_ast(s{}).append(String<'.'>{}).append(f{}).append(String<'/','*','@'>{}).append(print_label(l{}).append(String<'*','/'>{}));
}

template <typename l, typename y, typename v>
auto
print_ast(const Expression<l, y, VarReference<v>>&)
{
	using namespace mutils;
	return print_varname(v{}).append(String<'/','*','@'>{}).append(print_label(l{}).append(String<'*','/'>{}));
}

template <int i>
auto
print_ast(const Expression<Label<top>, int, Constant<i>>&)
{
	using namespace mutils;
	return string_from_int<i>();
}

template <typename l, typename y, char op, typename L, typename R>
auto
print_ast(const Expression<l, y, BinOp<op, L, R>>&)
{
	using namespace mutils;
	return print_ast(L{}).append(String<' ',op,'/','*','@'>{}).append(print_label(l{}).append(String<'*','/'>{})).append(String<' '>{}).append(print_ast(R{}));
}

template <typename l, typename y, typename h>
auto print_ast(const Expression<l, y, IsValid<h>>&)
{
	using namespace mutils;
	return print_ast(h{}).append(String<'.','i','s','V','a','l','i','d','/','*','@'>{}).append(print_label(l{}).append(String<'*','/'>{})).template append<'(',')'>();
}

	template <typename l, typename lold, typename y, typename h>
	auto print_ast(const Expression<l, y, Endorse<lold,h>>&)
{
	using namespace mutils;
	return print_ast(h{}).append(MUTILS_STRING(.endorse()){}).append(print_label(l{}).append(String<'*','/'>{})).template append<')'>();
}


	template <typename l, typename b, typename body, typename tab>
	auto print_ast(const Statement<l, Let<b, body>>&, tab)
{
	using namespace mutils;
	return tab{}.append(String<'/','*','@'>{})
								 .append(print_label(l{}).append(String<'*','/'>{}))
								 .append(String<' '>{})
								 .append(print_ast(b{})).append(String<' ',';',' ','{'>{})
								 .append(print_ast(body{},tab{})).append(tab{}).append(String<'}'>{});
}

	template <typename l, typename b, typename body, typename tab>
auto
	print_ast(const Statement<l, LetRemote<b, body>>&, tab)
{
	using namespace mutils;
	return tab{}.append(String<'/','*',' ','r','e','m','o','t','e','@'>{}).
								 append(print_label(l{}).append(String<'*','/'>{})).append(String<' '>{}).append(print_ast(b{})).append(String<' ',';',' '>{})
								 .append(print_ast(body{},tab{}))
								 .append(tab::append(String<'}'>{}));
}


	template <typename l, typename y, typename oper_name, typename Hndl, typename... args>
auto
	print_ast(const Expression<l, y, Operation<oper_name,Hndl,args...>>&)
{
	using namespace mutils;

	return print_ast(Hndl{}).append(String<'.',' ','/','*','@'>{}).append(print_label(l{}).append(String<'*','/'>{})).append(String<' '>{})
		.append(oper_name{}).append(String<'('>{})
		.append(print_ast(args{})...).append(String<')'>{});
	
}
	template <typename l, typename oper_name, typename Hndl, typename tab, typename... args>
auto
	print_ast(const Statement<l, Operation<oper_name,Hndl,args...>>&, tab)
{
	using namespace mutils;
	return tab::append(print_ast(Expression<l, void, Operation<oper_name,Hndl,args...>>{}));
}

	
	template <typename l, typename L, typename R, typename tab>
auto
print_ast(const Statement<l, Assignment<L, R>>&, tab)
{
	using namespace mutils;
	return tab::append(print_ast(L{})).append(String<' ','=','/','*','@'>{}).append(print_label(l{}).append(String<'*','/'>{})).append(String<' '>{}).append(print_ast(R{}));
}

	template <typename l, typename R, typename tab>
auto
print_ast(const Statement<l, Return<R> >&, tab)
{
	using namespace mutils;
	return tab::append(String<'r','e','t','u','r','n',' '>{}).append(print_ast(R{}));

}


	template <typename l, typename c, typename t, typename e, typename tab>
auto
print_ast(const Statement<l, If<c, t, e>>&, tab)
{
	using namespace mutils;
	return tab::append(String<'i','f','/','*','@'>{}).append(print_label(l{}).append(String<'*','/'>{})).append(String<' ','('>{}).append(print_ast(c{}))
		.append(String<')',' ', '{'>{})
		.append(print_ast(t{},tab{})).append(String<'}',' ','e','l','s','e',' ','{'>{})
		.append(print_ast(e{},tab{})).append(String<'}'>{});
}

	template <typename l, typename c, typename t, typename tab, char... name>
auto
print_ast(const Statement<l, While<c, t, name...>>&, tab)
{
	using namespace mutils;
	return tab::append(String<'w','h','i','l','e',' ','/','*','@'>{}).append(print_label(l{}).append(String<'*','/'>{}))
		.append(String<' ','('>{}).append(print_ast(c{})).append(String<')',' ','{'>{})
		.append(print_ast(t{},tab{}))
		.append(tab::append(String<'}',' '>{}));
}

template <typename l, typename _tab>
auto
print_ast(const Statement<l, Sequence<>>&, _tab)
{
	return mutils::String<>{};
}

template<typename tab>
struct print_one{
	template<typename e>
	static constexpr auto f(e){
		return tab::append(print_ast(e{},tab{})).template append<';'>();
	}
};
	
template <typename l, typename _tab, typename... Seq>
auto
print_ast(const Statement<l, Sequence<Seq...>>&, _tab)
{
	using tab = DECT(_tab::append(String<' ',' '>{}));
	using namespace std;
	using namespace mutils;

	using label_str = String<'{',' ','/','*','l','a','b','e','l',':',':',')'>;
	return _tab::append(label_str{}).append(print_label(l{}).append(String<'*','/'>{})).append(String<';'>{}).append(String<'\n'>{})
		.append(print_one<tab>::f(Seq{})...)
		.append(_tab::template append<'}'>());
	
}

template <typename l, typename s>
auto& operator<<(std::ostream& o, const Statement<l, s>& ast)
{
	return o << print_ast(ast, String<>{});
}

template <typename l, typename y, typename e>
auto& operator<<(std::ostream& o, const Expression<l, y, e>& ast)
{
	return o << print_ast(ast);
}

template <typename l, typename y, typename v, typename e>
auto& operator<<(std::ostream& o, const Binding<l, y, v, e>& ast)
{
	return o << print_ast(ast);
}
}
}
}
