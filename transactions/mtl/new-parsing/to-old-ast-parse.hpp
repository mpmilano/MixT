#pragma once

#include "ast.hpp"
#include "../AST_parse.hpp"

namespace myria { namespace mtl{ 
    template<typename T> constexpr auto convert_to_old_parsed(const T&);
    namespace new_parse_phase { namespace as_types{
        template<char... s>
        using old_label_t = ::myria::Label<mutils::String<s...>>;
    template<typename T, std::size_t p>
    constexpr auto convert_to_old_parsed(const transaction<T,p>&){
        return convert_to_old_parsed(T{});
    }
    template<typename T, char... str>
    constexpr auto convert_to_old_parsed(const FieldReference<T,mutils::String<str...>>&){
        return parse_phase::FieldReference<DECT(convert_to_old_parsed(T{})),mutils::String<str...>>{};
    }
    template<typename T, char... str>
    constexpr auto convert_to_old_parsed(const FieldPointerReference<T,mutils::String<str...>>&){
        return parse_phase::FieldPointerReference<DECT(convert_to_old_parsed(T{})),mutils::String<str...>>{};
    }
    template<typename T>
    constexpr auto convert_to_old_parsed(const Dereference<T>&){
        return parse_phase::Dereference<DECT(convert_to_old_parsed(T{}))>{};
    }
    template<typename T, char... str>
    constexpr auto convert_to_old_parsed(const Ensure<Label<mutils::String<str...>>,T>&){
        return parse_phase::Ensure<old_label_t<str...>,DECT(convert_to_old_parsed(T{}))>{};
    }
    template<typename T, char... str>
    constexpr auto convert_to_old_parsed(const Endorse<Label<mutils::String<str...>>,T>&){
        return parse_phase::Endorse<old_label_t<str...>,DECT(convert_to_old_parsed(T{}))>{};
    }
    template<typename T>
    constexpr auto convert_to_old_parsed(const IsValid<T>&){
        return parse_phase::IsValid<DECT(convert_to_old_parsed(T{}))>{};
    }
    template<char... s>
    constexpr auto convert_to_old_parsed(const VarReference<mutils::String<s...>>&){
        return parse_phase::VarReference<mutils::String<s...>>{};
    }
    template<std::size_t p>
    constexpr auto convert_to_old_parsed(const Constant<p>&){
        return parse_phase::Constant<p>{};
    }
    template<char op, typename L, typename R>
    constexpr auto convert_to_old_parsed(const BinOp<op,L,R>&){
        return parse_phase::BinOp<op,DECT(convert_to_old_parsed(L{})),DECT(convert_to_old_parsed(R{}))>{};
    }
    template<typename Binding, typename Body>
    constexpr auto convert_to_old_parsed(const Let<Binding,Body>&){
        return parse_phase::Let<DECT(convert_to_old_parsed(Binding{})),DECT(convert_to_old_parsed(Body{}))>{};
    }
    template<typename Binding, typename Body>
    constexpr auto convert_to_old_parsed(const LetRemote<Binding,Body>&){
        return parse_phase::LetRemote<DECT(convert_to_old_parsed(Binding{})),DECT(convert_to_old_parsed(Body{}))>{};
    }
    template<typename... T>
    constexpr auto convert_to_old_parsed(const operation_args_exprs<T...>&){
        return parse_phase::operation_args_exprs<DECT(convert_to_old_parsed(T{}))...>{};
    }
    template<typename... T>
    constexpr auto convert_to_old_parsed(const operation_args_varrefs<T...>&){
        return parse_phase::operation_args_varrefs<DECT(convert_to_old_parsed(T{}))...>{};
    }
    template<typename Hndl, typename eargs, typename vargs, char... name>
    constexpr auto convert_to_old_parsed(const Operation<mutils::String<name...>, Hndl,eargs,vargs>&){
        return parse_phase::Operation<mutils::String<name...>,DECT(convert_to_old_parsed(Hndl{})),DECT(convert_to_old_parsed(eargs{})),DECT(convert_to_old_parsed(vargs{}))>{};
    }
    template<typename T, typename R>
    constexpr auto convert_to_old_parsed(const Assignment<T,R>&){
        return parse_phase::Assignment<DECT(convert_to_old_parsed(T{})),DECT(convert_to_old_parsed(R{}))>{};
    }
    template<typename T>
    constexpr auto convert_to_old_parsed(const Return<T>&){
        return parse_phase::Return<DECT(convert_to_old_parsed(T{}))>{};
    }
    template<typename I, typename T, typename E>
    constexpr auto convert_to_old_parsed(const If<I,T,E>&){
        return parse_phase::If<DECT(convert_to_old_parsed(I{})),DECT(convert_to_old_parsed(T{})),DECT(convert_to_old_parsed(E{}))>{};
    }
    template<typename I, typename T>
    constexpr auto convert_to_old_parsed(const While<I,T>&){
        return parse_phase::While<DECT(convert_to_old_parsed(I{})),DECT(convert_to_old_parsed(T{}))>{};
    }
    template<typename Seq1, typename Seq2>
    constexpr auto convert_to_old_parsed(const Sequence<Seq1,Seq2>&){
        if constexpr (is_astnode_Sequence<Seq2>::value)
        return parse_phase::Sequence<DECT(convert_to_old_parsed(Seq1{}))>::append(typename DECT(convert_to_old_parsed(Seq2{}))::subseq{} );
        else if constexpr (is_astnode_Skip<Seq2>::value)
        return parse_phase::Sequence<DECT(convert_to_old_parsed(Seq1{}))>{};
        else return parse_phase::Sequence<DECT(convert_to_old_parsed(Seq1{})),DECT(convert_to_old_parsed(Seq2{}))>{};
    }

    template<typename T>
    constexpr auto convert_to_old_parsed(const Statement<T>&){
        using inner = DECT(convert_to_old_parsed(T{}));
        if constexpr (parse_phase::is_statement<inner>::value ) return inner{};
        else return parse_phase::Statement<inner>{};
    }

    template<typename T>
    constexpr auto convert_to_old_parsed(const Expression<T>&){
        return parse_phase::Expression<DECT(convert_to_old_parsed(T{}))>{};
    }

    template<typename T, char... str>
    constexpr auto convert_to_old_parsed(const Binding<mutils::String<str...>,T>&){
        return parse_phase::Binding<mutils::String<str...>,DECT(convert_to_old_parsed(T{}))>{};
    }

    constexpr auto convert_to_old_parsed(const Skip&){
        return parse_phase::Sequence<>{};
    }
    
}}
template<typename T> constexpr auto convert_to_old_parsed(const T& t){
    return new_parse_phase::as_types::convert_to_old_parsed(t);
}
}}