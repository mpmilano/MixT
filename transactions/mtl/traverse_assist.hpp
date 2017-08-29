#pragma once

namespace myria {
namespace mtl {

	template<template<typename,typename> class F, class Accum, class... args>
	struct fold_all;

	template<template<typename,typename> class F, class Accum>
	struct fold_all<F,Accum>{
		using type = Accum;
	};
	
	template<template<typename,typename> class F, class Accum, class arg1, class... args>
	struct fold_all<F,Accum,arg1,args...>{
		using type = F<typename fold_all<F,Accum,args...>::type,arg1>;
	};
	
	template<template<typename,typename> class F, class Accum, class... args>
	using Fold_all = typename fold_all<F,Accum,args...>::type;

	template<template<typename,typename> class Combine, class Default, class... args>
	using Combine_all = Fold_all<Combine,Default,args...>;

}
}
