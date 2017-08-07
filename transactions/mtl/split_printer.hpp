#pragma once
#include "AST_split.hpp"
#include <iostream>
#include <sstream>

namespace myria {
namespace mtl {
namespace split_phase {

template <typename l, typename l2, typename y, typename v, typename e>
void
print_ast(std::ostream& o, const typename AST<l>::template Binding<l2, y, v, e>&)
{
  print_varname(o, v{});
  o << " : " << mutils::type_name<y>() << " = ";
  print_ast<l>(o, e{});
}

template <typename l, typename y, typename s, typename f>
void
print_ast(std::ostream& o, const typename AST<l>::template Expression<y, typename AST<l>::template FieldReference<s, f>>&)
{
  print_ast<l>(o, s{});
  o << "." << f{};
}

  template<typename> struct print_split_type_str;
  template<typename T>
  const std::string& print_split_type();
  
  template<typename l, typename y, typename... ops> struct print_split_type_str<Handle<l,y,ops...> >{
    static const auto& print(){
      using namespace std;
      static const std::string s{
	[](){
	  stringstream ss;
	  ss << "Handle<" << l{} << "," << print_split_type<y>() << ">";
	  return ss.str();
	}()};
      return s;
    }
  };

  template<typename T> struct print_split_type_str{
    static const auto& print(){
      static const auto ret = mutils::type_name<T>();
      return ret;
    }
  };
  
  template<typename T>
  const std::string& print_split_type(){
    return print_split_type_str<T>::print();
  };
  
template <typename l, typename y, typename v>
void
print_ast(std::ostream& o, const typename AST<l>::template Expression<y, typename AST<l>::template VarReference<v>>&)
{
  o << "(";
  print_varname(o, v{});
  o << " : " << print_split_type<y>() << ")";
}

template <typename l, int i>
void
print_ast(std::ostream& o, const typename AST<l>::template Expression<int, typename AST<l>::template Constant<i>>&)
{
  o << i;
}

template <typename l>
void
print_ast(std::ostream& o, const typename AST<l>::template Expression<tracker::Tombstone, typename AST<l>::template GenerateTombstone<>>&)
{
  o << "random_tombstone()";
}


template <typename l, typename y, char op, typename L, typename R>
void
print_ast(std::ostream& o, const typename AST<l>::template Expression<y, typename AST<l>::template BinOp<op, L, R>>&)
{
  static const std::string opstr{{ op, (op == '=' ? '=' : 0) , 0 }};
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

	template <typename l, typename n, typename h, typename body>
void
	print_ast(std::ostream& o, const typename AST<l>::template Statement<typename AST<l>::template LetIsValid<n,h, body>>&, const std::string& tab)
{
  o << tab << "let isValid" << " [" << n{} << " = " << h{} << "] in "
    << "{";
  print_ast<l>(o, body{}, tab);
  o << tab << "}";
}

template <typename l, typename oper_name, typename Hndl, typename... args>
void
print_ast(std::ostream& o, const typename AST<l>::template Statement<typename AST<l>::template StatementOperation<oper_name,Hndl,args...>>&, const std::string& tab)
{
	o << tab;
	print_ast<l>(o,Hndl{});
	o << "." << oper_name{} << "(";
	((print_ast<l>(o,args{}),o << ","),...);
	o << ")";
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

template <typename l, typename R>
void
print_ast(std::ostream& o, const typename AST<l>::template Statement<typename AST<l>::template Return<R>>&, const std::string& tab)
{
  o << tab << "return ";
  print_ast<l>(o, R{});
}
  
template <typename l, typename R>
void
print_ast(std::ostream& o, const typename AST<l>::template Statement<typename AST<l>::template AccompanyWrite<R>>&, const std::string& tab)
{
  o << tab << "track ";
  print_ast<l>(o, R{});
}

  template <typename l, typename e>
void
print_ast(std::ostream& o, const typename AST<l>::template Statement<typename AST<l>::template WriteTombstone<e>>&, const std::string& tab)
{
  o << tab << "write(";
  print_ast<l>(o,e{});
  o << ")";
}

template <typename l, char... var>
void
print_ast(std::ostream& o, const typename AST<l>::template Statement<typename AST<l>::template IncrementOccurance<String<var...>>>&, const std::string& tab)
{
  o << tab << "use(";
  print_varname(o, String<var...>{});
  o << ")";
}

template <typename l, char... var>
void
print_ast(std::ostream& o, const typename AST<l>::template Statement<typename AST<l>::template IncrementRemoteOccurance<String<var...>>>&, const std::string& tab)
{
  o << tab << "use remote(";
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
