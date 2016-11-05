#pragma once
#include "TemporaryMutation.hpp"
#include "MutableTemporary.hpp"

namespace myria { namespace mtl {

		template<typename Temp>
                struct RefTemporaryCommon {
		private:
			RefTemporaryCommon(const Temp& t, const std::string& name, int id):t(t),name(name),id(id){}
		public:
	
			const Temp t;
			const std::string name;

			//Note: this ID will change
			//every time we copy this class.
			//every copy should have a unique ID.
			const int id = mutils::gensym();

			RefTemporaryCommon(const Temp &t):t(t),name(t.name) {}

			RefTemporaryCommon(const RefTemporaryCommon& rt):t(rt.t),name(rt.name){
			}
	
			RefTemporaryCommon(RefTemporaryCommon&& rt):t(rt.t),name(rt.name),id(rt.id){}

			RefTemporaryCommon clone() const {
				return RefTemporaryCommon(t,name,id);
			}

			auto environment_expressions() const {
				return t.environment_expressions();
			}
		};

		template<typename Temp>
		struct RefTemporary;

                //This will dereference a handle! needs to be annotated with the *handle's* level!
                template<unsigned long long ID, Level l, typename T>
                struct RefTemporary<Temporary<ID,l,T > > :
                        public RefTemporaryCommon<Temporary<ID,l,T > >,
                        public ConExpr<typename run_result<T>::stored_type,run_result<T>::level_t::value>  {

                    static_assert(can_flow(l,run_result<T>::level_t::value),
                                  "Error: cannot dereference this handle; it is stored at a causal level, but it points to a strong object");

                    using super_t = RefTemporaryCommon<Temporary<ID,l,T > >;
                    using Temp = Temporary<ID,l,T >;
		private:
			RefTemporary(const Temp& t, const std::string& name, int id):super_t(t,name,id){}
		public:
					using level = typename run_result<T>::level_t;
					
			RefTemporary(const Temp &t):super_t(t){}

			RefTemporary(const RefTemporary& rt):super_t(rt){}

			auto strongCall(TransactionContext* ctx, StrongCache& cache, const StrongStore &s) const {
				choose_strong<level::value > choice{nullptr};
				try {
					return strongCall(ctx,cache, s,choice);
				}
				catch (const StoreMiss&){
					std::cerr << "tried to reference variable " << this->name << std::endl;
					assert(false && "we don't have that in the store");
					struct dead_code{}; throw dead_code{};
				}
			}

			auto strongCall(TransactionContext* ctx, StrongCache& cache, const StrongStore &s, std::true_type*) const {
                            //This is a handle; we will call get on it, which returns a shared pointer, which we then dereference to store in the cache
				auto ret = *s.template get<run_result<T> >(this->t.store_id).get(ctx->trackingContext->trk,ctx);
				cache.insert(this->id,ret);
				return ret;
			}

			void strongCall(TransactionContext* , StrongCache& , const StrongStore &, std::false_type*) const {
				//we haven't even done the assignment yet. nothing to see here.
			}

			auto causalCall(TransactionContext* ctx, CausalCache& cache, const CausalStore &s) const {
		
				//typedef decltype(call<StoreType::CausalStore>(this->s,this->t)) R;
				using R = typename run_result<T>::stored_type;
				if (cache.contains(this->id)) {
					return cache.get<R>(this->id);
				}
				else {
                                    R ret = *s.template get<run_result<T> >(this->t.store_id).get(ctx->trackingContext->trk,ctx);
                                    cache.insert(this->id,ret);
                                    return ret;
				}
			}
	
			template<typename E2>
			auto operator=(const E2 &e) const {
				static_assert(is_ConExpr<std::decay_t<decltype(wrap_constants(e))> >::value,"Error: attempt to assign non-Expr");
				return *this << wrap_constants(e);
			}//*/

		};

		template<unsigned long long ID, Level l, typename T>
                struct RefTemporary<MutableTemporary<ID,l,T > > :
		public RefTemporaryCommon<MutableTemporary<ID,l,T > >,
		public ConExpr<run_result<T>, get_level<T>::value >
		{
                        using super_t = RefTemporaryCommon<MutableTemporary<ID,l,T > >;
			using Temp = MutableTemporary<ID,l,T >;
		private:
			RefTemporary(const Temp& t, const std::string& name, int id):super_t(t,name,id){}
		public:
			RefTemporary(const Temp &t):super_t(t){}

			RefTemporary(const RefTemporary& rt):super_t(rt){}

			using level = std::integral_constant<Level,l>;
			
			template<typename E>
			auto operator=(const E &e) const {
				static_assert(is_ConExpr<E>::value,"Error: attempt to assign non-Expr");
				auto wrapped = wrap_constants(e);
				TemporaryMutation<decltype(wrapped)> r{this->name,this->t.store_id,wrapped};
				return r;
			}

                        auto strongCall(TransactionContext* ctx, StrongCache& cache, const StrongStore &s) const {
				choose_strong<get_level<Temp>::value > choice{nullptr};
                                return strongCall(ctx,cache, s,choice);
			}

			auto strongCall(TransactionContext*, StrongCache& cache, const StrongStore &s, std::true_type*) const {
				//std::cout << "inserting RefTemp " << name << " (" << id<< ") into cache "
				//		  << &cache << std::endl;
                                auto ret = s.template get<run_result<T> >(this->t.store_id);
				cache.insert(this->id,ret);
				//std::cout << "RefTemp ID " << this->id << " inserting into Cache " << &cache << " value: " << ret << std::endl;
				return ret;
			}

			void strongCall(TransactionContext* , StrongCache& , const StrongStore &, std::false_type*) const {
				//we haven't even done the assignment yet. nothing to see here.
			}

			auto causalCall(TransactionContext* , CausalCache& cache, const CausalStore &s) const {
		
				using R = run_result<T>;
				if (cache.contains(this->id)) {
					return cache.get<R>(this->id);
				}
                                else {
                                    R ret = s.template get<run_result<T> >(this->t.store_id);
                                    cache.insert(this->id,ret);
                                    return ret;
                                }
			}
		};

                template<unsigned long long ID, Level l, typename T>
                auto find_usage(const RefTemporaryCommon<Temporary<ID,l,T> > &rt){
			return mutils::shared_copy(rt.t);
		}
                template<unsigned long long ID, Level l, typename T>
                auto find_usage(const RefTemporaryCommon<MutableTemporary<ID,l,T> > &rt){
                        return mutils::shared_copy(rt.t);
                }

                template<unsigned long long ID, unsigned long long ID2, Level l, typename T>
                std::enable_if_t<ID != ID2, std::nullptr_t> find_usage(const RefTemporaryCommon<Temporary<ID2,l,T> > & rtc){
					return find_usage<ID>(rtc.t);
				}

                template<unsigned long long ID, unsigned long long ID2, Level l, typename T>
                std::enable_if_t<ID != ID2, std::nullptr_t> find_usage(const RefTemporaryCommon<MutableTemporary<ID2,l,T> > & rtc){
					return find_usage<ID>(rtc.t);
                }

                template<unsigned long long ID, unsigned long long ID2, Level l, typename T>
                struct contains_temporary<ID, RefTemporary<Temporary<ID2,l,T> > > : std::integral_constant<bool, ID == ID2 || contains_temporary<ID,T>::value > {};

                template<unsigned long long ID, unsigned long long ID2, Level l, typename T>
                struct contains_temporary<ID, RefTemporary<MutableTemporary<ID2,l,T> > > : std::integral_constant<bool, ID == ID2 || contains_temporary<ID,T>::value> {};

		//TODO: figure out why this needs to be here
                template<typename temp>
                struct is_ConExpr<RefTemporary<temp> > : std::true_type {};

		struct nope{
			typedef std::false_type found;
		};

                template<typename E>
                constexpr bool is_reftemp(const RefTemporaryCommon<E> *){
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
