#pragma once
#include "TemporaryMutation.hpp"

namespace myria { namespace mtl {

		template<unsigned long long ID, Level l, typename T, typename Temp>
		struct RefTemporary : public ConExpr<run_result<T>,l> {
		private:
			RefTemporary(const Temp& t, const std::string& name, int id):t(t),name(name),id(id){}
		public:
	
			const Temp t;
			const std::string name;

			//Note: this ID will change
			//every time we copy this class.
			//every copy should have a unique ID.
			const int id = mutils::gensym();

			RefTemporary(const Temp &t):t(t),name(t.name) {}

			RefTemporary(const RefTemporary& rt):t(rt.t),name(rt.name){
				assert(!debug_forbid_copy);
			}
	
			RefTemporary(RefTemporary&& rt):t(rt.t),name(rt.name),id(rt.id){}

			RefTemporary clone() const {
				return RefTemporary(t,name,id);
			}

			auto environment_expressions() const {
				return t.environment_expressions();
			}

			auto strongCall(TransactionContext* ctx, StrongCache& cache, const StrongStore &s) const {
				//TODO - endorsements should happen somewhere around here, right?
				//todo: dID I want the level of the expression which assigned the temporary?
		
				choose_strong<get_level<Temp>::value > choice{nullptr};
				try {
					return strongCall(ctx,cache, s,choice);
				}
				catch (const StoreMiss&){
					std::cerr << "tried to reference variable " << name << std::endl;
					assert(false && "we don't have that in the store");
				}
			}

			auto strongCall(TransactionContext* ctx, StrongCache& cache, const StrongStore &s, std::true_type*) const {
				//std::cout << "inserting RefTemp " << name << " (" << id<< ") into cache "
				//		  << &cache << std::endl;
				auto ret = call<StoreType::StrongStore>(s, t);
				cache.insert(id,ret);
				//std::cout << "RefTemp ID " << this->id << " inserting into Cache " << &cache << " value: " << ret << std::endl;
				return ret;
			}

			void strongCall(TransactionContext* ctx, StrongCache& cache, const StrongStore &s, std::false_type*) const {
				//we haven't even done the assignment yet. nothing to see here.
			}

			auto causalCall(TransactionContext* ctx, const CausalCache& cache, const CausalStore &s) const {
		
				typedef decltype(call<StoreType::CausalStore>(s,t)) R;
				if (cache.contains(this->id)) {
					return cache.get<R>(this->id);
				}
				else {
					try {
						return call<StoreType::CausalStore>(s,t);
					}
					catch (const StoreMiss&){
						std::cerr << "Couldn't find this in the store: " << name << std::endl;
						assert(false && "Not in the store");
					}
				}
			}

			template<typename E>
			std::enable_if_t<!std::is_same<Temporary<ID,l,T>, Temp>::value, TemporaryMutation<decltype(wrap_constants(*mutils::mke_p<E>()))> >
			operator=(const E &e) const {
				static_assert(is_ConExpr<E>::value,"Error: attempt to assign non-Expr");
				auto wrapped = wrap_constants(e);
				TemporaryMutation<decltype(wrapped)> r{name,t.store_id,wrapped};
				return r;
			}

			template<typename E>
			std::enable_if_t<std::is_same<Temporary<ID,l,T>, Temp>::value,
							 decltype(std::declval<RefTemporary>() << std::declval<E>())>
			operator=(const E &e) const {
				static_assert(is_ConExpr<E>::value,"Error: attempt to assign non-Expr");
				return *this << e;
			}//*/



		private:
			template<StoreType st, restrict(is_store(st))>
			static auto call(const StoreMap<st> &s, const Temporary<ID,l,T> &t) ->
				run_result<decltype(t.t)>
				{
					typedef run_result<decltype(t.t)> R;
					static_assert(mutils::neg_error_helper<is_ConStatement,R>::value,"Static assert failed");
					return s. template get<R>(t.store_id);
				}
		};

		template<unsigned long long ID, Level l, typename T, typename Temp>
		auto find_usage(const RefTemporary<ID,l,T,Temp> &rt){
			return mutils::shared_copy(rt.t);
		}

		template<unsigned long long ID, unsigned long long ID2, Level l, typename T, typename Temp>
		std::enable_if_t<ID != ID2, std::nullptr_t> find_usage(const RefTemporary<ID2,l,T,Temp> &rt){
			return nullptr;
		}

		template<unsigned long long ID, unsigned long long ID2, Level l, typename T, typename Temp>
		struct contains_temporary<ID, RefTemporary<ID2,l,T,Temp> > : std::integral_constant<bool, ID == ID2> {};

		//TODO: figure out why this needs to be here
		template<Level l, typename T, typename E, unsigned long long id>
		struct is_ConExpr<RefTemporary<id,l,T, E> > : std::true_type {};

		struct nope{
			typedef std::false_type found;
		};

		template<Level l, typename T, typename E, unsigned long long id>
		constexpr bool is_reftemp(const RefTemporary<id,l,T, E> *){
			return true;
		}

		template<typename T>
		constexpr bool is_reftemp(const T*){
			return false;
		}

		template<typename T>
		struct is_RefTemporary : std::integral_constant<bool,is_reftemp(mutils::mke_p<T>())>::type
		{};

	} }
