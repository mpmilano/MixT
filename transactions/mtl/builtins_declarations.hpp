#pragma once

namespace myria { namespace mtl { namespace builtins {
			struct ListStub{
				using label = Label<bottom>;
				using type = ListStub;
				using name = mutils::String<'d','e','f','a','u','l','t',' ','l','i','s','t'>;
				using push_back_name = mutils::String<'p','u','s','h','_','b','a','c','k'>;
				std::shared_ptr<void*> real_list{new void*{nullptr}};
				template<typename T> operator std::list<T>*(){
					return (std::list<T>*) *real_list;
				}
				template<typename T> operator std::list<T>() {
					return *(std::list<T>*) *real_list;
				}
			};

			struct NulledOp{
				using name = mutils::String<'n','u','l','l','e','d'>;
			};

			template<typename> struct is_builtin;

			template<> struct is_builtin<ListStub> : std::true_type{};
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
}
