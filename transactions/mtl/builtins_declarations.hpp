#pragma once

namespace myria { namespace mtl { namespace builtins {
			struct ListStub{
				using label = Label<bottom>;
				using type = ListStub;
				using name = mutils::String<'d','e','f','a','u','l','t',' ','l','i','s','t'>;
				using push_back_name = mutils::String<'p','u','s','h','_','b','a','c','k'>;
			};

			template<typename T> struct List {
				using label = typename ListStub::label;
				using type = List;
				using name = typename ListStub::name;
				using push_back_name = typename ListStub::push_back_name;
				std::list<T> t;

				//boilerplate; must find a way to eliminate it.
				bool is_struct{ true };
				auto& field(MUTILS_STRING(t)) { return t; }
				
			};

			struct NulledOp{
				using name = mutils::String<'n','u','l','l','e','d'>;
			};

			template<typename> struct is_builtin;

			template<> struct is_builtin<ListStub> : std::true_type{};
			template<typename T> struct is_builtin<List<T>> : std::true_type{};
			template<> struct is_builtin<typename ListStub::name> : std::true_type{};
			template<> struct is_builtin<typename ListStub::push_back_name> : std::true_type{};
			template<> struct is_builtin<typename NulledOp::name> : std::true_type{};
			template<typename> struct is_builtin : std::false_type{};
			
		}
	}}
namespace mutils{
	template<> struct typename_str<myria::mtl::builtins::ListStub>{
		static auto f(){return "List";}
	};

		template<typename T> struct typename_str<myria::mtl::builtins::List<T>>{
		static auto f(){
			std::stringstream ss;
			ss << "List<" << typename_str<T>::f() << ">";
			return ss.str();
		}
	};
}
