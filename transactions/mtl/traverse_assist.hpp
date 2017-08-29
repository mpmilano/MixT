#pragma once

namespace myria {
namespace mtl {

	template<template<typename,typename> class Combine, class Default, class... args>
	struct fold_all;

	template<template<typename,typename> class Combine, class Default>
	struct fold_all<Combine,Default>{
		using type = Default;
	};
	
	template<template<typename,typename> class Combine, class Default, class arg1, class... args>
	struct fold_all<Combine,Default,arg1,args...>{
		using type = Combine<arg1,typename fold_all<Combine,Default,args...>::type>;
	};
	
	template<template<typename,typename> class Combine, class Default, class... args>
	using Fold_all = typename fold_all<Combine,Default,args...>::type;

	template<template<typename,typename> class Combine, class Default, class... args>
	using Combine_all = Fold_all<Combine,Default,args...>;

}
}
