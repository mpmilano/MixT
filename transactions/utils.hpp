#pragma once
#include "restrict.hpp"
#include "args-finder.hpp"
#include "Basics.hpp"
#include <type_traits>
#include <cassert>
#include <tuple>
#include <set>
#include <vector>
#include <map>
#include <dirent.h>
#include <string>
#include <iostream>
#include <sstream>
#include "../extras"
#include "macro_utils.hpp"

template<typename T, std::size_t size1, std::size_t size2>
auto prefix_array(const std::array<T,size1>& t,
				  const std::array<T,size2> &arr,
				  const T& remove){
	assert(remove == t[0]);
	assert(remove == arr[0]);
	std::array<T, size1 + size2 - 1 > ret;
	std::size_t i = 0;
	for (; i < size1; ++i){
		ret[i] = t[i];
	}
	std::size_t j = 1;
	for (; i < size1 + size2 -1 ; ++i){
		ret[i] = arr[j];
		++j;
	}
	return ret;
}

template<typename T>
constexpr T mke(){
	return *((typename std::decay<T>::type*) nullptr);
}

template<typename T>
constexpr typename std::enable_if<!std::is_pointer<T>::value,typename std::decay<T>::type*>::type
mke_p(){
	return (typename std::decay<T>::type*) nullptr;
}

template<typename T, restrict(std::is_pointer<T>::value)>
constexpr T mke_p(){
	return (T) nullptr;
}

template<typename T>
T cpy(const T& t){
	return T(t);
}

template<int ...>
struct seq { };

template<int N, int ...S>
struct gens : gens<N-1, N-1, S...> { };

template<int ...S>
struct gens<0, S...> {
	typedef seq<S...> type;
	static type build(){ return type();}
};


template<typename F, typename Tuple, int ...S>
auto __callFunc(const F& f, const Tuple &t, seq<S...>) {
	return f(std::get<S>(t)...);
}


template<typename F, typename Tuple, restrict(!std::is_function<F>::value)>
auto callFunc(const F &f, const Tuple &t) {
	return __callFunc(f,t,gens<std::tuple_size<Tuple>::value >::build() );
}

template<typename Ret, typename Tuple, typename Pack, typename... Args>
Ret callFunc(Ret (*f) (Args...), const Tuple &t, Pack p) {
	return __callFunc(convert(f),t,p);
}

template<typename Ret, typename Tuple, int ...S>
Ret callConstructor(const Tuple &t, seq<S...>) {
	return Ret(std::get<S>(t)...);
}

template<typename Arg>
constexpr Arg last_of_f(const std::tuple<Arg>*){
	return mke<Arg>();
}

template<typename Arg1, typename... Args>
constexpr decltype(last_of_f(mke_p<std::tuple<Args...> >()))
	last_of_f(const std::tuple<Arg1, Args...>*){
	return last_of_f(mke_p<std::tuple<Args...> >());
}

template<typename... T>
struct last_of {
	static_assert(sizeof...(T) > 0, "Error: cannot call last_of on empty packs");
	using type = decltype(last_of_f(mke_p<std::tuple<typename std::decay<T>::type...> >()));
};

template<typename T>
T* heap_copy(const T& t){
	return new T(t);
}

template<const int i,restrict(i <= 0)>
constexpr unsigned long long unique_id(const char*){
	return 0;
}
	

template<const int i>
constexpr typename std::enable_if<(i > 0), unsigned long long>::type
unique_id(const char str[i]){
	return (i == 0 ? 0 : (str[0] << sizeof(char)*i ) + unique_id<i-1>(str+1));
}



#include <type_traits>
#include <typeinfo>
#ifndef _MSC_VER
#   include <cxxabi.h>
#endif
#include <memory>
#include <string>
#include <cstdlib>

template <class T>
std::string
type_name()
{
	typedef typename std::remove_reference<T>::type TR;
	std::unique_ptr<char, void(*)(void*)> own
		(
			#ifndef _MSC_VER
			abi::__cxa_demangle(typeid(TR).name(), nullptr,
								nullptr, nullptr),
			#else
			nullptr,
			#endif
			std::free
			);
	std::string r = own != nullptr ? own.get() : typeid(TR).name();
	if (std::is_const<TR>::value)
		r += " const";
	if (std::is_volatile<TR>::value)
		r += " volatile";
	if (std::is_lvalue_reference<T>::value)
		r += "&";
	else if (std::is_rvalue_reference<T>::value)
		r += "&&";
	return r;
}


template<typename A, typename B>
constexpr auto conditional(std::true_type*, const A a, const B &){
	return a;
}

template<typename A, typename B>
constexpr auto conditional(std::false_type*, const A &, const B b2){
	return b2;
}

template<bool b, typename A, typename B>
constexpr auto conditional(const A &a, const B &b2){
	typedef typename std::integral_constant<bool,b>::type ptr;
	ptr* p = nullptr;
	return conditional(p,a,b2);
}


template<typename> struct _Left;
template<typename A, typename B> struct _Left<std::pair<A,B> >{
	typedef A type;
};
template<typename T>
using Left = typename _Left<T>::type;

template<typename> struct _Right;
template<typename A, typename B> struct _Right<std::pair<A,B> >{
	typedef B type;
};
template<typename T>
using Right = typename _Right<T>::type;



int gensym() {
	static int counter = 0;
	assert(counter < (std::numeric_limits<int>::max() - 1));
	return ++counter;
}

std::vector<std::string> read_dir(const std::string &name){

	std::vector<std::string> ret;
	
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir (name.c_str()))) {
		/* print all the files and directories within directory */
		while ((ent = readdir (dir))) {
			std::string maybe(ent->d_name);
			if (maybe == "." || maybe == "..") continue;
			ret.push_back(std::string(maybe));
		}
		closedir (dir);
	} else {
		/* could not open directory */
		perror ("");
		assert(false && "Could not open dir.");
	}

	return ret;
}

template<typename T>
std::unique_ptr<T> make_unique(T *t){
	return std::unique_ptr<T>(t);
}

template<typename T>
std::shared_ptr<const T> make_cnst_shared(T *t){
	return std::shared_ptr<const T>(t);
}

template<typename T>
std::shared_ptr<const T> make_cnst_shared(std::nullptr_t){
	return std::shared_ptr<const T>();
}


template<typename T>
T& realmke(){
	static T t;
	return t;
}

template<typename T>
const T& constify(const T& t){
	return t;
}


template<typename T>
T pick_useful(const T &t, std::nullptr_t){
	return t;
}

std::nullptr_t pick_useful(std::nullptr_t, std::nullptr_t){
	return nullptr;
}

template<typename T>
T pick_useful(std::nullptr_t, const T& t){
	return t;
}

template<typename T>
std::string to_string(const T &t){
	std::stringstream ss;
	ss << t;
	return ss.str();
}

/*
template<typename T, typename U>
typename std::enable_if<!(std::is_same<T,std::nullptr_t>::value || std::is_same<T,std::nullptr_t>::value), T>::type
	pick_useful(const U& u, const T& t){
	return t;
}
//*/

template<typename... T>
void ignore(const T & ...) {}

