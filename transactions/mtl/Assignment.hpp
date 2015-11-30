#pragma once
#include "Handle.hpp"
#include "Temporary.hpp"
#include "RefTemporary.hpp"
#include "HandleCaching.hpp"

namespace myria { namespace mtl {

    template<Level l, HandleAccess ha, typename T, typename Expr>
    struct Assignment : public ConStatement<get_level<Expr>::value>{
    private:

      template<Level l2, HandleAccess ha2>
      static auto hndle_get(Handle<l2,ha2,T> h){
	return h.get();
      }

      template<typename T2, restrict(!is_handle<T2>::value)>
      static auto hndle_get(const T2 &t2){
	return t2;
      }

      template<typename T2>
      struct HDref_i {using t = T2;};
      template<Level l2, HandleAccess ha2>
      struct HDref_i<Handle<l2,ha2,T> > {using t = T;};
      template<typename T2>
      using HDref = typename HDref_i<run_result<T2> >::t;
	
    public:
      const Handle<l,ha,T> t;
      const Expr e;
      const int id = mutils::gensym();
      Assignment(const Handle<l,ha,T> &t, const Expr & e)
	:t(t),e(e)
      {
	static_assert(
		      can_flow(get_level<Expr>::value, l),
		      "Error: assignment to strong member from causal expression!"
		      );
	static_assert(std::is_same<HDref<Expr>, T>::value, "Error: Assignment of incompatible types (no subtyping applies here)" );
      }

      auto handles() const{
	return mtl::handles(e);
      }

      bool strongCall(StrongCache& c, const StrongStore &s) const {
	choose_strong<l> choice1{nullptr};
	choose_strong<get_level<Expr>::value> choice2{nullptr};
	strongCall(c,s,choice1,choice2);
	return true;
      }

      void strongCall(StrongCache &c, const StrongStore &s, std::false_type*, std::false_type*) const {
	run_ast_strong(c,s,e);
      }
      void strongCall(StrongCache &c, const StrongStore &s, std::false_type*, std::true_type*) const {
	c.insert(id,Assignment::hndle_get(run_ast_strong(c,s,e)));
      }

      template<typename T2>
      auto strongCall(const StrongCache &c, const StrongStore &s, std::true_type*, T2*) const {
	static_assert(runs_with_strong(get_level<Expr>::value),"error: flow violation in assignment");
	t.clone().put(Assignment::hndle_get(run_ast_strong(c,s,e)));
      }

      bool causalCall(const CausalCache &c, const CausalStore &s) const {
	choose_strong<l> choice{nullptr};
	causalCall(c,s,choice);
	return true;
      }

      auto causalCall(const CausalCache &c, const CausalStore &s, std::false_type*) const {
	if (runs_with_strong(get_level<Expr>::value) ){
	  t.clone().put(c.get<HDref<Expr> >(id));
	}
	else {
	  t.clone().put(Assignment::hndle_get(run_ast_causal(c,s,e)));
	}
      }

      auto causalCall(const CausalCache &c, const CausalStore &s, std::true_type*) const {
	static_assert(l == get_level<Expr>::value && l == Level::strong,"static assert failed");
      }

    };

    template<unsigned long long ID, Level l, typename T, HandleAccess HA, typename E>
    auto operator<<(const RefTemporary<ID,l,Handle<l,HA,T>,Temporary<ID,l,Handle<l,HA,T> > >& rt, const E &e){
      return Assignment<l,HA,T,E>{rt.t.t,e};
    }

    template<unsigned long long ID, Level l, HandleAccess ha, typename T, typename Expr>
    auto find_usage(const Assignment<l,ha,T,Expr> &rt){
      return find_usage<ID>(rt.e);
    }

  } }
