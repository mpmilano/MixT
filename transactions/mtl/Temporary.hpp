#pragma once
#include "ConExpr.hpp"
#include "CommonExprs.hpp"
#include "Temporary_common.hpp"
#include <string>

namespace myria { namespace mtl {

		template<unsigned long long ID, Level l, typename T>
		struct Temporary : public TemporaryCommon<ID, l,T> {

			static_assert(is_handle<run_result<T> >::value, "Error: this is a dereferencing construct. only valid on handles");
			
			Temporary(const std::string& name, const T& t):
				TemporaryCommon<ID,l,T>(name,t){}

			typedef typename std::integral_constant<Level,l>::type level;
			typedef T type;
			typedef std::true_type found;
			typedef typename std::integral_constant<unsigned long long, ID>::type key;
		};

		template<unsigned long long ID, unsigned long long ID2, Level l, typename T>
		std::enable_if_t<ID != ID2,std::nullptr_t> find_usage(const Temporary<ID2,l,T> &rt){
			return nullptr;
		}

	
		template<unsigned long long ID, Level l, typename T>
		auto find_usage(const Temporary<ID,l,T> &rt){
			return mutils::shared_copy(rt);
		}

		template<unsigned long long ID, unsigned long long ID2, Level l, typename T>
		struct contains_temporary<ID, Temporary<ID2,l,T> > : std::integral_constant<bool, ID == ID2> {};

	} }
