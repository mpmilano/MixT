#pragma once
#include "ConStatement.hpp"

namespace myria { namespace mtl {

		template<unsigned long long ID, typename CS, Level l, typename Temp>
		struct DeclarationScope : public ConStatement<l>{
			const std::string name;
			const std::shared_ptr<const Temp > gt;
			const CS cs;
			const int id = mutils::gensym();
	
			template<typename Ptr>
			DeclarationScope(const std::string &name, const Ptr &gt, const CS &cs)
				:name(name),gt(gt),cs(cs){
			}

			DeclarationScope(const DeclarationScope& ds):
				name(ds.name),gt(ds.gt),cs(ds.cs){}

			auto environment_expressions() const {
				assert(gt);
				return std::tuple_cat(
					gt_environment_expressions(gt.get()),
					stmt_environment_expressions(cs));
			}

			bool strongCall(TransactionContext* ctx, StrongCache& c, StrongStore &s) const {
				assert(gt);
				return strongc_helper(ctx,c,s,gt) && call_all_strong(ctx,c,s,cs);
			}
	
			bool causalCall(TransactionContext* ctx, CausalCache& c, CausalStore &s) const {
				assert(gt);
				return causalc_helper(ctx,c,s,gt) && call_all_causal(ctx,c,s,cs);
			}

			virtual bool isVirtual() const = 0;

			virtual ~DeclarationScope(){}
		};

	} }
