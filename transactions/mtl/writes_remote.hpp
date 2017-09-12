#pragma once
#include "mtl/mtlutils.hpp"
#include "mtl/AST_typecheck.hpp"

namespace myria { namespace mtl  { namespace typecheck_phase {

			template<typename remote_name_set, typename AST>
			constexpr bool writes_remote(AST);


			template<typename remote_name_set, typename l, typename y, typename v>
			constexpr bool bottoms_out_at_remote(Expression<l,y,VarReference<v> >);
			
			template<typename remote_name_set, typename l, typename y, typename s, typename f>
			constexpr bool bottoms_out_at_remote(Expression<l,y,FieldReference<s,f> >){
				return bottoms_out_at_remote<remote_name_set>(s{});
			}
		
			template<typename remote_name_set, typename l, typename y, typename v>
			constexpr bool bottoms_out_at_remote(Expression<l,y,VarReference<v> >){
				return remote_name_set::template contains<v>();
			}

template<typename remote_name_set, typename l, typename bl, typename by, typename bv, typename be, typename s>
constexpr bool _writes_remote(Statement<l,Let<Binding<bl,by,bv,be>,s> >){
	return writes_remote<remote_name_set>(s{});
}

template<typename remote_name_set, typename l, typename bl, typename by, typename bv, typename be, typename s>
constexpr bool _writes_remote(Statement<l,LetRemote<Binding<bl,by,bv,be>,s> >){
	return writes_remote<DECT(remote_name_set::template add<bv>()) >(s{});
}

template<typename remote_name_set, typename l, typename oper_name, typename Hndl, typename... args>
constexpr bool _writes_remote(Statement<l,Operation<oper_name, Hndl, args...> >){
	//TODO: should operations count as a remote write? 
	return false;
}

template<typename remote_name_set, typename l, typename c, typename t, typename e>
constexpr bool _writes_remote(Statement<l,If<c,t,e> >){
	return writes_remote<remote_name_set>(t{}) || writes_remote<remote_name_set>(e{});
}

template<typename remote_name_set, typename l, typename c, typename e>
constexpr bool _writes_remote(Statement<l,While<c,e> >){
	return writes_remote<remote_name_set>(e{});
}

template<typename remote_name_set, typename l, typename c, typename e>
constexpr bool _writes_remote(Statement<l,Assignment<c,e> >){
	return bottoms_out_at_remote<remote_name_set>(c{});
}

template<typename remote_name_set, typename l, typename e>
constexpr bool _writes_remote(Statement<l,Return<e> >){
	return false;
}

template<typename remote_name_set, typename l, typename... seq>
constexpr bool _writes_remote(Statement<l,Sequence<seq...> >){
	return (false || ... || writes_remote<remote_name_set>(seq{}));
}

template<typename remote_name_set, typename AST>
constexpr bool writes_remote(AST){
	return _writes_remote<remote_name_set>(AST{});
}

template<typename AST>
constexpr bool begin_writes_remote(AST){
	return _writes_remote<mutils::typeset<> >(AST{});
}

}}}
