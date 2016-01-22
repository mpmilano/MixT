#pragma once
#include "CommonExprs.hpp"

/*

  The idea here:  Right now we're compiling in things from the environment directly, effectively as constants, which doesn't allow those parameters to vary.  Sometimes we want the parameters to vary.  

Solution sketch: 

 - change transactions to exist within a lambda which *does not* allow environment capture
 - give transaction blocks an extra parameter (like free expressions) which indicates the names of types we want to capture
 - capture those types as "EnvironmentExpressions" 
 - "EnvironmentExpressions" get their *actual* value from something passed into the transaction at execute time.  


Implementation Path:

 - restrict transactions to eliminate environment capture
 - add parameter to transaction execution which is the EnvironmentExpression
 - figure out how to pass this parameter around during execution
 - etc.

Ideas on passing around: 
 - known location in Cache; this requires figuring out where the location is, especially troubling given multiple environment expressions
 - queue of things in TransactionContext; evaluation order needs to be precise, queue would need to either know types ahead of time or be ready for some very unsafe casting.

Common problems with both approaches: 
 - EnvironmentExpression definitely needs to know underlying type
 - need to collect information from EnvironmentExpressions; either types + order for queue, or IDs for cache.

Point: if we're collecting the EnvironmentExpressions *anyway*, can't we just *actually* collect them and assign things to them in the beginning of the transaction?  Answer: Yes, but this requires an effort similar in extensiveness to the "handles" construct from before.  

Query: Are we even using handles anymore?  Could repurpose for this.  Answer: We are not using handles, let's repurpose. 

Summary: 
 - restrict transactions to eliminate environment capture
 - collect EnvironmentExpression via handles() mechanisms
 - assign parameters before beginning of transaction execution. ERROR: Const.
 - instead we'll go with queue option; assign parameters to Hetero queue (probably tuple-backed)
 - which is member of TransactionContext and is itself passed around.


Dependencies accumulating:
 - transaction's operator() needs to take a T* and assign it to the queue (just one T for now? yes. Macro limitations, seems easy to change later {famous last words})
//*/

namespace myria { namespace mtl{
		template<typename T>
		struct EnvironmentExpression : public ConExpr<T, Level::undef> { //local, so no info-flow restrictions should apply.

			static_assert(!std::is_lvalue_reference<T>::value && !std::is_rvalue_reference<T>::value, "Error: transaction capture only available by value");
			static_assert(!is_ConExpr<T>::value, "Error: environment capture for use on *external* resources.  Come on man.");
			
			const int id = mutils::gensym();
			T* v;
			EnvironmentExpression(){}
			EnvironmentExpression(const EnvironmentExpression& n):v(n.v){}

			auto handles() const {
				return v.handles();
			}

			auto causalCall(TransactionContext* ctx, CausalCache& cache, const CausalStore& s) const {
				//runs with strong, so we should already have populated by this point
				assert(cache.contains(this->id));
				return cache.get<T>(this->id);;
			}

			auto strongCall(TransactionContext* ctx, StrongCache& cache, const StrongStore& s) const {
				assert(v);
				auto ret = *v;
				cache.insert(this->id,ret);
				return ret;
			}

			template<typename i2>
			friend std::ostream & operator<<(std::ostream &os, const EnvironmentExpression<i2>&);
		};

		template<unsigned long long ID, typename T>
		struct contains_temporary<ID, EnvironmentExpression<T> > : std::false_type {};


		template<unsigned long long ID, typename T>
		auto find_usage(const EnvironmentExpression<T> &){
			return nullptr;
		}

		//TODO: figure out why this needs to be here
		template<typename T>
		struct is_ConExpr<EnvironmentExpression<T> > : std::true_type {};
 
		template<typename T>
		EnvironmentExpression<T> env_expr(T const * const){
			return EnvironmentExpression<T>{};
		}
	}
}
