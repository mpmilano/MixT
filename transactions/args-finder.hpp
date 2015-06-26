#include <type_traits>
#include <tuple>
#include <functional>

using namespace std;

template <typename T>
struct function_traits
    : public function_traits<decltype(&T::operator())>
{};
// For generic types, directly use the result of the signature of its 'operator()'

template <typename ClassType, typename ReturnType, typename... Args>
struct function_traits<ReturnType(ClassType::*)(Args...) const>
// we specialize for pointers to member function
{
	enum { arity = sizeof...(Args) };
	// arity is the number of arguments.

	typedef ReturnType result_type;

	template <size_t i>
	    struct arg
	{
		typedef typename std::tuple_element<i, std::tuple<Args...>>::type type;
		// the i-th argument is equivalent to the i-th tuple element of a tuple
		// composed of those arguments.
	};

	template<typename T>
	static std::function<ReturnType (Args...)> as_function(T t){
		return std::function<ReturnType (Args...)>(t);
	}
};

template <typename ClassType, typename ReturnType>
struct function_traits<ReturnType(ClassType::*)() const>
{
	enum { arity = 0 };
	typedef ReturnType result_type;
	template<typename T>
	static std::function<ReturnType ()> as_function(T t){
		return std::function<ReturnType ()>(t);
	}
};

template<typename F>
auto convert(F f) -> decltype(function_traits<F>::as_function(f))
{
	return function_traits<F>::as_function(f);
}
