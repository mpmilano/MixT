

#pragma once
#include "allocator.hpp"
#include "mutils/CTString.hpp"
#include "mutils/cstring.hpp"
#include "mutils/type_utils.hpp"
#include "union.hpp"
#include <ostream>

namespace myria {
namespace mtl {
namespace new_parse_phase {

template <typename T>
using plain_array = T[20];

namespace as_values {

struct Label
{
  plain_array<char> label = { 0 };
  constexpr Label() = default;
  constexpr Label(const Label&) = delete;
  constexpr Label(Label&& l) { ::mutils::cstring::str_cpy(label, l.label); }
  constexpr Label& operator=(Label&& l)
  {
    ::mutils::cstring::str_cpy(label, l.label);
    return *this;
  }
};

struct transaction;
struct FieldReference;
struct FieldPointerReference;
struct Dereference;
struct Endorse;
struct Ensure;
struct IsValid;
struct VarReference;
struct Constant;
struct BinOp;
struct Let;
struct LetRemote;
struct operation_args_exprs;
struct operation_args_varrefs;
struct Operation;
struct Assignment;
struct Return;
struct If;
struct While;
struct Sequence;
struct Skip;
struct Binding;
using AST_elem = Union<transaction, FieldReference, FieldPointerReference, Dereference, Endorse, Ensure, IsValid, VarReference, Constant, BinOp, Let, LetRemote,
                       operation_args_exprs, operation_args_varrefs, Operation, Assignment, Return, If, While, Sequence, Skip, Binding>;
template <std::size_t budget>
using AST_Allocator = Allocator<budget, transaction, AST_elem>;

constexpr bool
is_non_null(const allocated_ref<AST_elem>& e)
{
  return e;
}

// Define structs.
struct Expression
{
  constexpr Expression() {}
};
struct Statement
{
  constexpr Statement() {}
};
struct Binding
{
  allocated_ref<AST_elem> rhs{};
  char var[20] = { 0 };
  constexpr Binding() {}
};
struct transaction : public Statement
{
  allocated_ref<AST_elem> e{};
  std::size_t payload{ 0 };
  // default constructor
  constexpr transaction(){};
  constexpr transaction(transaction&& p)
    : e{ std::move(p.e) }
    , payload{ std::move(p.payload) }
  {
  }
  // move-assignment
  constexpr transaction& operator=(transaction&& p)
  {
    e = std::move(p.e);
    payload = std::move(p.payload);
    return *this;
  }
};
struct FieldReference : public Expression
{
  allocated_ref<AST_elem> Struct{};
  plain_array<char> Field{};
  // default constructor
  constexpr FieldReference(){};
  constexpr FieldReference(FieldReference&& p)
    : Struct{ std::move(p.Struct) }
  {
    mutils::cstring::str_cpy(Field, p.Field);
  }
  // move-assignment
  constexpr FieldReference& operator=(FieldReference&& p)
  {
    Struct = std::move(p.Struct);
    mutils::cstring::str_cpy(Field, p.Field);
    return *this;
  }
};
struct FieldPointerReference : public Expression
{
  allocated_ref<AST_elem> Struct{};
  plain_array<char> Field{};
  // default constructor
  constexpr FieldPointerReference(){};
  constexpr FieldPointerReference(FieldPointerReference&& p)
    : Struct{ std::move(p.Struct) }
  {
    mutils::cstring::str_cpy(Field, p.Field);
  }
  // move-assignment
  constexpr FieldPointerReference& operator=(FieldPointerReference&& p)
  {
    Struct = std::move(p.Struct);
    mutils::cstring::str_cpy(Field, p.Field);
    return *this;
  }
};
struct Dereference : public Expression
{
  allocated_ref<AST_elem> Struct{};
  // default constructor
  constexpr Dereference(){};
  constexpr Dereference(Dereference&& p)
    : Struct{ std::move(p.Struct) }
  {
  }
  // move-assignment
  constexpr Dereference& operator=(Dereference&& p)
  {
    Struct = std::move(p.Struct);
    return *this;
  }
};
struct Endorse : public Expression
{
  Label label{};
  allocated_ref<AST_elem> Hndl{};
  // default constructor
  constexpr Endorse(){};
  constexpr Endorse(Endorse&& p)
    : label{ std::move(p.label) }
    , Hndl{ std::move(p.Hndl) }
  {
  }
  // move-assignment
  constexpr Endorse& operator=(Endorse&& p)
  {
    label = std::move(p.label);
    Hndl = std::move(p.Hndl);
    return *this;
  }
};
struct Ensure : public Expression
{
  Label label{};
  allocated_ref<AST_elem> Hndl{};
  // default constructor
  constexpr Ensure(){};
  constexpr Ensure(Ensure&& p)
    : label{ std::move(p.label) }
    , Hndl{ std::move(p.Hndl) }
  {
  }
  // move-assignment
  constexpr Ensure& operator=(Ensure&& p)
  {
    label = std::move(p.label);
    Hndl = std::move(p.Hndl);
    return *this;
  }
};
struct IsValid : public Expression
{
  allocated_ref<AST_elem> Hndl{};
  // default constructor
  constexpr IsValid(){};
  constexpr IsValid(IsValid&& p)
    : Hndl{ std::move(p.Hndl) }
  {
  }
  // move-assignment
  constexpr IsValid& operator=(IsValid&& p)
  {
    Hndl = std::move(p.Hndl);
    return *this;
  }
};
struct VarReference : public Expression
{
  plain_array<char> Var{};
  // default constructor
  constexpr VarReference(){};
  constexpr VarReference(VarReference&& p) { mutils::cstring::str_cpy(Var, p.Var); }
  // move-assignment
  constexpr VarReference& operator=(VarReference&& p)
  {
    mutils::cstring::str_cpy(Var, p.Var);
    return *this;
  }
};
struct Constant : public Expression
{
  std::size_t i{ 0 };
  // default constructor
  constexpr Constant(){};
  constexpr Constant(Constant&& p)
    : i{ std::move(p.i) }
  {
  }
  // move-assignment
  constexpr Constant& operator=(Constant&& p)
  {
    i = std::move(p.i);
    return *this;
  }
};
struct BinOp : public Expression
{
  char op{};
  allocated_ref<AST_elem> L{};
  allocated_ref<AST_elem> R{};
  // default constructor
  constexpr BinOp(){};
  constexpr BinOp(BinOp&& p)
    : op{ std::move(p.op) }
    , L{ std::move(p.L) }
    , R{ std::move(p.R) }
  {
  }
  // move-assignment
  constexpr BinOp& operator=(BinOp&& p)
  {
    op = std::move(p.op);
    L = std::move(p.L);
    R = std::move(p.R);
    return *this;
  }
};
struct Let : public Statement
{
  allocated_ref<AST_elem> Binding{};
  allocated_ref<AST_elem> Body{};
  // default constructor
  constexpr Let(){};
  constexpr Let(Let&& p)
    : Binding{ std::move(p.Binding) }
    , Body{ std::move(p.Body) }
  {
  }
  // move-assignment
  constexpr Let& operator=(Let&& p)
  {
    Binding = std::move(p.Binding);
    Body = std::move(p.Body);
    return *this;
  }
};
struct LetRemote : public Statement
{
  allocated_ref<AST_elem> Binding{};
  allocated_ref<AST_elem> Body{};
  // default constructor
  constexpr LetRemote(){};
  constexpr LetRemote(LetRemote&& p)
    : Binding{ std::move(p.Binding) }
    , Body{ std::move(p.Body) }
  {
  }
  // move-assignment
  constexpr LetRemote& operator=(LetRemote&& p)
  {
    Binding = std::move(p.Binding);
    Body = std::move(p.Body);
    return *this;
  }
};
struct operation_args_exprs
{
  plain_array<allocated_ref<AST_elem>> exprs;
  // default constructor
  constexpr operation_args_exprs(){};
  // move constructor
  constexpr operation_args_exprs(operation_args_exprs&& p)
    : exprs{ { allocated_ref<AST_elem>{} } }
  {
    for (auto i = 0u; i < 20; ++i) {
      exprs[i] = std::move(p.exprs[i]);
    }
  }

  // move-assignment
  constexpr operation_args_exprs& operator=(operation_args_exprs&& p)
  {
    for (auto i = 0u; i < 20; ++i) {
      exprs[i] = std::move(p.exprs[i]);
    }
    return *this;
  }
};
struct operation_args_varrefs
{
  plain_array<allocated_ref<AST_elem>> vars;
  // default constructor
  constexpr operation_args_varrefs(){};
  // move constructor
  constexpr operation_args_varrefs(operation_args_varrefs&& p)
    : vars{ { allocated_ref<AST_elem>{} } }
  {
    for (auto i = 0u; i < 20; ++i) {
      vars[i] = std::move(p.vars[i]);
    }
  }

  // move-assignment
  constexpr operation_args_varrefs& operator=(operation_args_varrefs&& p)
  {
    for (auto i = 0u; i < 20; ++i) {
      vars[i] = std::move(p.vars[i]);
    }
    return *this;
  }
};
struct Operation : public Statement, public Expression
{
  plain_array<char> name{};
  allocated_ref<AST_elem> Hndl{};
  allocated_ref<AST_elem> expr_args{};
  allocated_ref<AST_elem> var_args{};
  bool is_statement{};
  // default constructor
  constexpr Operation(){};
  constexpr Operation(Operation&& p)
    : Hndl{ std::move(p.Hndl) }
    , expr_args{ std::move(p.expr_args) }
    , var_args{ std::move(p.var_args) }
    , is_statement{ std::move(p.is_statement) }
  {
    mutils::cstring::str_cpy(name, p.name);
  }
  // move-assignment
  constexpr Operation& operator=(Operation&& p)
  {
    mutils::cstring::str_cpy(name, p.name);
    Hndl = std::move(p.Hndl);
    expr_args = std::move(p.expr_args);
    var_args = std::move(p.var_args);
    is_statement = std::move(p.is_statement);
    return *this;
  }
};
struct Assignment : public Statement
{
  allocated_ref<AST_elem> Var{};
  allocated_ref<AST_elem> Expr{};
  // default constructor
  constexpr Assignment(){};
  constexpr Assignment(Assignment&& p)
    : Var{ std::move(p.Var) }
    , Expr{ std::move(p.Expr) }
  {
  }
  // move-assignment
  constexpr Assignment& operator=(Assignment&& p)
  {
    Var = std::move(p.Var);
    Expr = std::move(p.Expr);
    return *this;
  }
};
struct Return : public Statement
{
  allocated_ref<AST_elem> Expr{};
  // default constructor
  constexpr Return(){};
  constexpr Return(Return&& p)
    : Expr{ std::move(p.Expr) }
  {
  }
  // move-assignment
  constexpr Return& operator=(Return&& p)
  {
    Expr = std::move(p.Expr);
    return *this;
  }
};
struct If : public Statement
{
  allocated_ref<AST_elem> condition{};
  allocated_ref<AST_elem> then{};
  allocated_ref<AST_elem> els{};
  // default constructor
  constexpr If(){};
  constexpr If(If&& p)
    : condition{ std::move(p.condition) }
    , then{ std::move(p.then) }
    , els{ std::move(p.els) }
  {
  }
  // move-assignment
  constexpr If& operator=(If&& p)
  {
    condition = std::move(p.condition);
    then = std::move(p.then);
    els = std::move(p.els);
    return *this;
  }
};
struct While : public Statement
{
  allocated_ref<AST_elem> condition{};
  allocated_ref<AST_elem> body{};
  // default constructor
  constexpr While(){};
  constexpr While(While&& p)
    : condition{ std::move(p.condition) }
    , body{ std::move(p.body) }
  {
  }
  // move-assignment
  constexpr While& operator=(While&& p)
  {
    condition = std::move(p.condition);
    body = std::move(p.body);
    return *this;
  }
};
struct Sequence : public Statement
{
  allocated_ref<AST_elem> e{};
  allocated_ref<AST_elem> next{};
  // default constructor
  constexpr Sequence(){};
  constexpr Sequence(Sequence&& p)
    : e{ std::move(p.e) }
    , next{ std::move(p.next) }
  {
  }
  // move-assignment
  constexpr Sequence& operator=(Sequence&& p)
  {
    e = std::move(p.e);
    next = std::move(p.next);
    return *this;
  }
};
struct Skip : public Statement
{
  // default constructor
  constexpr Skip(){};
  constexpr Skip(Skip&&) {}
  // move-assignment
  constexpr Skip& operator=(Skip&&) { return *this; }
};
} // namespace as_values

namespace as_types {
template <typename>
struct Label;
template <char... c>
struct Label<mutils::String<c...>>
{
  using label = mutils::String<c...>;
  constexpr Label() = default;
};
template <typename>
struct Expression;
template <typename>
struct Statement;

template <typename var_name, typename expr>
struct Binding;
template <char... var_name, typename expr>
struct Binding<mutils::String<var_name...>, Expression<expr>>
{
};

template <typename e, std::size_t payload>
struct transaction
{
};
template <typename _e, std::size_t _payload>
struct Statement<transaction<_e, _payload>>
{
  using e = _e;
  std::size_t payload{ _payload };
};
template <typename Struct, typename Field>
struct FieldReference
{
};
template <typename _Struct, typename _Field>
struct Expression<FieldReference<_Struct, _Field>>
{
  using Struct = _Struct;
  using Field = _Field;
};
template <typename Struct, typename Field>
struct FieldPointerReference
{
};
template <typename _Struct, typename _Field>
struct Expression<FieldPointerReference<_Struct, _Field>>
{
  using Struct = _Struct;
  using Field = _Field;
};
template <typename Struct>
struct Dereference
{
};
template <typename _Struct>
struct Expression<Dereference<_Struct>>
{
  using Struct = _Struct;
};
template <typename label, typename Hndl>
struct Endorse
{
};
template <typename _label, typename _Hndl>
struct Expression<Endorse<_label, _Hndl>>
{
  using label = _label;
  using Hndl = _Hndl;
};
template <typename label, typename Hndl>
struct Ensure
{
};
template <typename _label, typename _Hndl>
struct Expression<Ensure<_label, _Hndl>>
{
  using label = _label;
  using Hndl = _Hndl;
};
template <typename Hndl>
struct IsValid
{
};
template <typename _Hndl>
struct Expression<IsValid<_Hndl>>
{
  using Hndl = _Hndl;
};
template <typename Var>
struct VarReference
{
};
template <typename _Var>
struct Expression<VarReference<_Var>>
{
  using Var = _Var;
};
template <std::size_t i>
struct Constant
{
};
template <std::size_t _i>
struct Expression<Constant<_i>>
{
  std::size_t i{ _i };
};
template <char op, typename L, typename R>
struct BinOp
{
};
template <char _op, typename _L, typename _R>
struct Expression<BinOp<_op, _L, _R>>
{
  char op{ _op };
  using L = _L;
  using R = _R;
};
template <typename Binding, typename Body>
struct Let
{
};
template <typename _Binding, typename _Body>
struct Statement<Let<_Binding, _Body>>
{
  using Binding = _Binding;
  using Body = _Body;
};
template <typename Binding, typename Body>
struct LetRemote
{
};
template <typename _Binding, typename _Body>
struct Statement<LetRemote<_Binding, _Body>>
{
  using Binding = _Binding;
  using Body = _Body;
};
template <typename...>
struct operation_args_exprs;
template <typename...>
struct operation_args_exprs
{
};
template <typename...>
struct operation_args_varrefs;
template <typename...>
struct operation_args_varrefs
{
};
template <typename name, typename Hndl, typename expr_args, typename var_args>
struct Operation
{
};
template <typename _name, typename _Hndl, typename _expr_args, typename _var_args>
struct Statement<Operation<_name, _Hndl, _expr_args, _var_args>>
{
  using name = _name;
  using Hndl = _Hndl;
  using expr_args = _expr_args;
  using var_args = _var_args;
};
template <typename _name, typename _Hndl, typename _expr_args, typename _var_args>
struct Expression<Operation<_name, _Hndl, _expr_args, _var_args>>
{
  using name = _name;
  using Hndl = _Hndl;
  using expr_args = _expr_args;
  using var_args = _var_args;
};
template <typename Var, typename Expr>
struct Assignment
{
};
template <typename _Var, typename _Expr>
struct Statement<Assignment<_Var, _Expr>>
{
  using Var = _Var;
  using Expr = _Expr;
};
template <typename Expr>
struct Return
{
};
template <typename _Expr>
struct Statement<Return<_Expr>>
{
  using Expr = _Expr;
};
template <typename condition, typename then, typename els>
struct If
{
};
template <typename _condition, typename _then, typename _els>
struct Statement<If<_condition, _then, _els>>
{
  using condition = _condition;
  using then = _then;
  using els = _els;
};
template <typename condition, typename body>
struct While
{
};
template <typename _condition, typename _body>
struct Statement<While<_condition, _body>>
{
  using condition = _condition;
  using body = _body;
};
template <typename e, typename next>
struct Sequence
{
};
template <typename _e, typename _next>
struct Statement<Sequence<_e, _next>>
{
  using e = _e;
  using next = _next;
};
struct Skip
{};
template <>
struct Statement<Skip>
{
};

} // namespace as_types

namespace as_types {
template <typename>
struct is_astnode_transaction : public std::false_type
{
};
template <typename e, std::size_t payload>
struct is_astnode_transaction<Statement<transaction<e, payload>>> : public std::true_type
{
};
template <typename>
struct is_astnode_FieldReference : public std::false_type
{
};
template <typename Struct, typename Field>
struct is_astnode_FieldReference<Expression<FieldReference<Struct, Field>>> : public std::true_type
{
};
template <typename>
struct is_astnode_FieldPointerReference : public std::false_type
{
};
template <typename Struct, typename Field>
struct is_astnode_FieldPointerReference<Expression<FieldPointerReference<Struct, Field>>> : public std::true_type
{
};
template <typename>
struct is_astnode_Dereference : public std::false_type
{
};
template <typename Struct>
struct is_astnode_Dereference<Expression<Dereference<Struct>>> : public std::true_type
{
};
template <typename>
struct is_astnode_Endorse : public std::false_type
{
};
template <typename label, typename Hndl>
struct is_astnode_Endorse<Expression<Endorse<label, Hndl>>> : public std::true_type
{
};
template <typename>
struct is_astnode_Ensure : public std::false_type
{
};
template <typename label, typename Hndl>
struct is_astnode_Ensure<Expression<Ensure<label, Hndl>>> : public std::true_type
{
};
template <typename>
struct is_astnode_IsValid : public std::false_type
{
};
template <typename Hndl>
struct is_astnode_IsValid<Expression<IsValid<Hndl>>> : public std::true_type
{
};
template <typename>
struct is_astnode_VarReference : public std::false_type
{
};
template <typename Var>
struct is_astnode_VarReference<Expression<VarReference<Var>>> : public std::true_type
{
};
template <typename>
struct is_astnode_Constant : public std::false_type
{
};
template <std::size_t i>
struct is_astnode_Constant<Expression<Constant<i>>> : public std::true_type
{
};
template <typename>
struct is_astnode_BinOp : public std::false_type
{
};
template <char op, typename L, typename R>
struct is_astnode_BinOp<Expression<BinOp<op, L, R>>> : public std::true_type
{
};
template <typename>
struct is_astnode_Let : public std::false_type
{
};
template <typename Binding, typename Body>
struct is_astnode_Let<Statement<Let<Binding, Body>>> : public std::true_type
{
};
template <typename>
struct is_astnode_LetRemote : public std::false_type
{
};
template <typename Binding, typename Body>
struct is_astnode_LetRemote<Statement<LetRemote<Binding, Body>>> : public std::true_type
{
};
template <typename>
struct is_astnode_Operation : public std::false_type
{
};
template <typename name, typename Hndl, typename expr_args, typename var_args>
struct is_astnode_Operation<Statement<Operation<name, Hndl, expr_args, var_args>>> : public std::true_type
{
};
template <typename name, typename Hndl, typename expr_args, typename var_args>
struct is_astnode_Operation<Expression<Operation<name, Hndl, expr_args, var_args>>> : public std::true_type
{
};
template <typename>
struct is_astnode_Assignment : public std::false_type
{
};
template <typename Var, typename Expr>
struct is_astnode_Assignment<Statement<Assignment<Var, Expr>>> : public std::true_type
{
};
template <typename>
struct is_astnode_Return : public std::false_type
{
};
template <typename Expr>
struct is_astnode_Return<Statement<Return<Expr>>> : public std::true_type
{
};
template <typename>
struct is_astnode_If : public std::false_type
{
};
template <typename condition, typename then, typename els>
struct is_astnode_If<Statement<If<condition, then, els>>> : public std::true_type
{
};
template <typename>
struct is_astnode_While : public std::false_type
{
};
template <typename condition, typename body>
struct is_astnode_While<Statement<While<condition, body>>> : public std::true_type
{
};
template <typename>
struct is_astnode_Sequence : public std::false_type
{
};
template <typename e, typename next>
struct is_astnode_Sequence<Statement<Sequence<e, next>>> : public std::true_type
{
};
template <typename>
struct is_astnode_Skip : public std::false_type
{
};
template <>
struct is_astnode_Skip<Statement<Skip>> : public std::true_type
{
};
template <typename T>
struct is_astnode_Statement : public std::false_type
{
};
template <typename T>
struct is_astnode_Statement<Statement<T>> : public std::true_type
{
};
} // namespace as_types

namespace as_values {

template <typename prev_holder>
struct as_type_f
{
  static constexpr const DECT(prev_holder::prev.allocator) & allocator{ prev_holder::prev.allocator };

  template <std::size_t index>
  struct arg_struct
  {
    static_assert(index > 0);
    constexpr arg_struct() = default;
    constexpr const AST_elem& operator()() const
    {
      return allocated_ref<AST_elem>{ typename allocated_ref<AST_elem>::really_set_index{}, index }.get(allocator);
    }
  };

  template <long budget, typename F>
  constexpr static auto as_type(std::enable_if_t<(budget > 0) && (budget <= 10000)>* = nullptr)
  {
    static_assert(budget > 0);
    if constexpr (budget > 0) {
      constexpr const AST_elem& e = F{}();
      if constexpr (e.template get_<transaction>().is_this_elem) {
        /*Declaring arg!*/ using arg0 = arg_struct<e.template get_<transaction>().t.e.get_index()>;
        using _arg0 = DECT(as_type<budget - 1, arg0>());
        constexpr auto _arg1 = e.template get_<transaction>().t.payload;
        return as_types::Statement<as_types::transaction<_arg0, _arg1>>{};
      } else if constexpr (e.template get_<FieldReference>().is_this_elem) {
        /*Declaring arg!*/ using arg0 = arg_struct<e.template get_<FieldReference>().t.Struct.get_index()>;
        using _arg0 = DECT(as_type<budget - 1, arg0>());

        constexpr auto& __str1 = e.template get_<FieldReference>().t.Field;
        using _arg1 =
          DECT(mutils::String<__str1[0], __str1[1], __str1[2], __str1[3], __str1[4], __str1[5], __str1[6], __str1[7], __str1[8], __str1[9], __str1[10],
                              __str1[11], __str1[12], __str1[13], __str1[14], __str1[15], __str1[16], __str1[17], __str1[18], __str1[19]>::trim_ends());
        return as_types::Expression<as_types::FieldReference<_arg0, _arg1>>{};
      } else if constexpr (e.template get_<FieldPointerReference>().is_this_elem) {
        /*Declaring arg!*/ using arg0 = arg_struct<e.template get_<FieldPointerReference>().t.Struct.get_index()>;
        using _arg0 = DECT(as_type<budget - 1, arg0>());

        constexpr auto& __str1 = e.template get_<FieldPointerReference>().t.Field;
        using _arg1 =
          DECT(mutils::String<__str1[0], __str1[1], __str1[2], __str1[3], __str1[4], __str1[5], __str1[6], __str1[7], __str1[8], __str1[9], __str1[10],
                              __str1[11], __str1[12], __str1[13], __str1[14], __str1[15], __str1[16], __str1[17], __str1[18], __str1[19]>::trim_ends());
        return as_types::Expression<as_types::FieldPointerReference<_arg0, _arg1>>{};
      } else if constexpr (e.template get_<Dereference>().is_this_elem) {
        /*Declaring arg!*/ using arg0 = arg_struct<e.template get_<Dereference>().t.Struct.get_index()>;
        using _arg0 = DECT(as_type<budget - 1, arg0>());
        return as_types::Expression<as_types::Dereference<_arg0>>{};
      } else if constexpr (e.template get_<Endorse>().is_this_elem) {

        constexpr auto& __lbl0 = e.template get_<Endorse>().t.label;
        using _arg0 = as_types::Label<DECT(
          mutils::String<__lbl0.label[0], __lbl0.label[1], __lbl0.label[2], __lbl0.label[3], __lbl0.label[4], __lbl0.label[5], __lbl0.label[6], __lbl0.label[7],
                         __lbl0.label[8], __lbl0.label[9], __lbl0.label[10], __lbl0.label[11], __lbl0.label[12], __lbl0.label[13], __lbl0.label[14],
                         __lbl0.label[15], __lbl0.label[16], __lbl0.label[17], __lbl0.label[18], __lbl0.label[19]>::trim_ends())>; /*Declaring arg!*/
        using arg1 = arg_struct<e.template get_<Endorse>().t.Hndl.get_index()>;
        using _arg1 = DECT(as_type<budget - 1, arg1>());
        return as_types::Expression<as_types::Endorse<_arg0, _arg1>>{};
      } else if constexpr (e.template get_<Ensure>().is_this_elem) {

        constexpr auto& __lbl0 = e.template get_<Ensure>().t.label;
        using _arg0 = as_types::Label<DECT(
          mutils::String<__lbl0.label[0], __lbl0.label[1], __lbl0.label[2], __lbl0.label[3], __lbl0.label[4], __lbl0.label[5], __lbl0.label[6], __lbl0.label[7],
                         __lbl0.label[8], __lbl0.label[9], __lbl0.label[10], __lbl0.label[11], __lbl0.label[12], __lbl0.label[13], __lbl0.label[14],
                         __lbl0.label[15], __lbl0.label[16], __lbl0.label[17], __lbl0.label[18], __lbl0.label[19]>::trim_ends())>; /*Declaring arg!*/
        using arg1 = arg_struct<e.template get_<Ensure>().t.Hndl.get_index()>;
        using _arg1 = DECT(as_type<budget - 1, arg1>());
        return as_types::Expression<as_types::Ensure<_arg0, _arg1>>{};
      } else if constexpr (e.template get_<IsValid>().is_this_elem) {
        /*Declaring arg!*/ using arg0 = arg_struct<e.template get_<IsValid>().t.Hndl.get_index()>;
        using _arg0 = DECT(as_type<budget - 1, arg0>());
        return as_types::Expression<as_types::IsValid<_arg0>>{};
      } else if constexpr (e.template get_<VarReference>().is_this_elem) {

        constexpr auto& __str0 = e.template get_<VarReference>().t.Var;
        using _arg0 =
          DECT(mutils::String<__str0[0], __str0[1], __str0[2], __str0[3], __str0[4], __str0[5], __str0[6], __str0[7], __str0[8], __str0[9], __str0[10],
                              __str0[11], __str0[12], __str0[13], __str0[14], __str0[15], __str0[16], __str0[17], __str0[18], __str0[19]>::trim_ends());
        return as_types::Expression<as_types::VarReference<_arg0>>{};
      } else if constexpr (e.template get_<Constant>().is_this_elem) {
        constexpr auto _arg0 = e.template get_<Constant>().t.i;
        return as_types::Expression<as_types::Constant<_arg0>>{};
      } else if constexpr (e.template get_<BinOp>().is_this_elem) {
        constexpr auto _arg0 = e.template get_<BinOp>().t.op;
        /*Declaring arg!*/ using arg1 = arg_struct<e.template get_<BinOp>().t.L.get_index()>;
        using _arg1 = DECT(as_type<budget - 1, arg1>());
        /*Declaring arg!*/ using arg2 = arg_struct<e.template get_<BinOp>().t.R.get_index()>;
        using _arg2 = DECT(as_type<budget - 1, arg2>());
        return as_types::Expression<as_types::BinOp<_arg0, _arg1, _arg2>>{};
      } else if constexpr (e.template get_<Let>().is_this_elem) {
        /*Declaring arg!*/ using arg0 = arg_struct<e.template get_<Let>().t.Binding.get_index()>;
        using _arg0 = DECT(as_type<budget - 1, arg0>());
        /*Declaring arg!*/ using arg1 = arg_struct<e.template get_<Let>().t.Body.get_index()>;
        using _arg1 = DECT(as_type<budget - 1, arg1>());
        return as_types::Statement<as_types::Let<_arg0, _arg1>>{};
      } else if constexpr (e.template get_<LetRemote>().is_this_elem) {
        /*Declaring arg!*/ using arg0 = arg_struct<e.template get_<LetRemote>().t.Binding.get_index()>;
        using _arg0 = DECT(as_type<budget - 1, arg0>());
        /*Declaring arg!*/ using arg1 = arg_struct<e.template get_<LetRemote>().t.Body.get_index()>;
        using _arg1 = DECT(as_type<budget - 1, arg1>());
        return as_types::Statement<as_types::LetRemote<_arg0, _arg1>>{};
      } else if constexpr (e.template get_<operation_args_exprs>().is_this_elem) {
        {
          constexpr std::size_t __indx = e.template get_<operation_args_exprs>().t.exprs[0].get_index();
          if constexpr (__indx > 0) {
            using arg0 = arg_struct<__indx>;
            using _arg0 = DECT(as_type<budget - 1, arg0>());
            {
              constexpr std::size_t __indx = e.template get_<operation_args_exprs>().t.exprs[1].get_index();
              if constexpr (__indx > 0) {
                using arg1 = arg_struct<__indx>;
                using _arg1 = DECT(as_type<budget - 1, arg1>());
                {
                  constexpr std::size_t __indx = e.template get_<operation_args_exprs>().t.exprs[2].get_index();
                  if constexpr (__indx > 0) {
                    using arg2 = arg_struct<__indx>;
                    using _arg2 = DECT(as_type<budget - 1, arg2>());
                    {
                      constexpr std::size_t __indx = e.template get_<operation_args_exprs>().t.exprs[3].get_index();
                      if constexpr (__indx > 0) {
                        using arg3 = arg_struct<__indx>;
                        using _arg3 = DECT(as_type<budget - 1, arg3>());
                        {
                          constexpr std::size_t __indx = e.template get_<operation_args_exprs>().t.exprs[4].get_index();
                          if constexpr (__indx > 0) {
                            using arg4 = arg_struct<__indx>;
                            using _arg4 = DECT(as_type<budget - 1, arg4>());
                            {
                              constexpr std::size_t __indx = e.template get_<operation_args_exprs>().t.exprs[5].get_index();
                              if constexpr (__indx > 0) {
                                using arg5 = arg_struct<__indx>;
                                using _arg5 = DECT(as_type<budget - 1, arg5>());
                                {
                                  constexpr std::size_t __indx = e.template get_<operation_args_exprs>().t.exprs[6].get_index();
                                  if constexpr (__indx > 0) {
                                    using arg6 = arg_struct<__indx>;
                                    using _arg6 = DECT(as_type<budget - 1, arg6>());
                                    {
                                      constexpr std::size_t __indx = e.template get_<operation_args_exprs>().t.exprs[7].get_index();
                                      if constexpr (__indx > 0) {
                                        using arg7 = arg_struct<__indx>;
                                        using _arg7 = DECT(as_type<budget - 1, arg7>());
                                        {
                                          constexpr std::size_t __indx = e.template get_<operation_args_exprs>().t.exprs[8].get_index();
                                          if constexpr (__indx > 0) {
                                            using arg8 = arg_struct<__indx>;
                                            using _arg8 = DECT(as_type<budget - 1, arg8>());
                                            {
                                              constexpr std::size_t __indx = e.template get_<operation_args_exprs>().t.exprs[9].get_index();
                                              if constexpr (__indx > 0) {
                                                using arg9 = arg_struct<__indx>;
                                                using _arg9 = DECT(as_type<budget - 1, arg9>());
                                                {
                                                  constexpr std::size_t __indx = e.template get_<operation_args_exprs>().t.exprs[10].get_index();
                                                  if constexpr (__indx > 0) {
                                                    using arg10 = arg_struct<__indx>;
                                                    using _arg10 = DECT(as_type<budget - 1, arg10>());
                                                    {
                                                      constexpr std::size_t __indx = e.template get_<operation_args_exprs>().t.exprs[11].get_index();
                                                      if constexpr (__indx > 0) {
                                                        using arg11 = arg_struct<__indx>;
                                                        using _arg11 = DECT(as_type<budget - 1, arg11>());
                                                        {
                                                          constexpr std::size_t __indx = e.template get_<operation_args_exprs>().t.exprs[12].get_index();
                                                          if constexpr (__indx > 0) {
                                                            using arg12 = arg_struct<__indx>;
                                                            using _arg12 = DECT(as_type<budget - 1, arg12>());
                                                            {
                                                              constexpr std::size_t __indx = e.template get_<operation_args_exprs>().t.exprs[13].get_index();
                                                              if constexpr (__indx > 0) {
                                                                using arg13 = arg_struct<__indx>;
                                                                using _arg13 = DECT(as_type<budget - 1, arg13>());
                                                                {
                                                                  constexpr std::size_t __indx =
                                                                    e.template get_<operation_args_exprs>().t.exprs[14].get_index();
                                                                  if constexpr (__indx > 0) {
                                                                    using arg14 = arg_struct<__indx>;
                                                                    using _arg14 = DECT(as_type<budget - 1, arg14>());
                                                                    {
                                                                      constexpr std::size_t __indx =
                                                                        e.template get_<operation_args_exprs>().t.exprs[15].get_index();
                                                                      if constexpr (__indx > 0) {
                                                                        using arg15 = arg_struct<__indx>;
                                                                        using _arg15 = DECT(as_type<budget - 1, arg15>());
                                                                        {
                                                                          constexpr std::size_t __indx =
                                                                            e.template get_<operation_args_exprs>().t.exprs[16].get_index();
                                                                          if constexpr (__indx > 0) {
                                                                            using arg16 = arg_struct<__indx>;
                                                                            using _arg16 = DECT(as_type<budget - 1, arg16>());
                                                                            {
                                                                              constexpr std::size_t __indx =
                                                                                e.template get_<operation_args_exprs>().t.exprs[17].get_index();
                                                                              if constexpr (__indx > 0) {
                                                                                using arg17 = arg_struct<__indx>;
                                                                                using _arg17 = DECT(as_type<budget - 1, arg17>());
                                                                                {
                                                                                  constexpr std::size_t __indx =
                                                                                    e.template get_<operation_args_exprs>().t.exprs[18].get_index();
                                                                                  if constexpr (__indx > 0) {
                                                                                    using arg18 = arg_struct<__indx>;
                                                                                    using _arg18 = DECT(as_type<budget - 1, arg18>());
                                                                                    {
                                                                                      constexpr std::size_t __indx =
                                                                                        e.template get_<operation_args_exprs>().t.exprs[19].get_index();
                                                                                      if constexpr (__indx > 0) {
                                                                                        using arg19 = arg_struct<__indx>;
                                                                                        using _arg19 = DECT(as_type<budget - 1, arg19>());
                                                                                        return as_types::operation_args_exprs<
                                                                                          _arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7, _arg8, _arg9,
                                                                                          _arg10, _arg11, _arg12, _arg13, _arg14, _arg15, _arg16, _arg17,
                                                                                          _arg18, _arg19>{};
                                                                                      } else {
                                                                                        return as_types::operation_args_exprs<
                                                                                          _arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7, _arg8, _arg9,
                                                                                          _arg10, _arg11, _arg12, _arg13, _arg14, _arg15, _arg16, _arg17,
                                                                                          _arg18>{};
                                                                                      }
                                                                                    }
                                                                                  } else {
                                                                                    return as_types::operation_args_exprs<
                                                                                      _arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7, _arg8, _arg9,
                                                                                      _arg10, _arg11, _arg12, _arg13, _arg14, _arg15, _arg16, _arg17>{};
                                                                                  }
                                                                                }
                                                                              } else {
                                                                                return as_types::operation_args_exprs<
                                                                                  _arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7, _arg8, _arg9, _arg10,
                                                                                  _arg11, _arg12, _arg13, _arg14, _arg15, _arg16>{};
                                                                              }
                                                                            }
                                                                          } else {
                                                                            return as_types::operation_args_exprs<_arg0, _arg1, _arg2, _arg3, _arg4, _arg5,
                                                                                                                  _arg6, _arg7, _arg8, _arg9, _arg10, _arg11,
                                                                                                                  _arg12, _arg13, _arg14, _arg15>{};
                                                                          }
                                                                        }
                                                                      } else {
                                                                        return as_types::operation_args_exprs<_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6,
                                                                                                              _arg7, _arg8, _arg9, _arg10, _arg11, _arg12,
                                                                                                              _arg13, _arg14>{};
                                                                      }
                                                                    }
                                                                  } else {
                                                                    return as_types::operation_args_exprs<_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6,
                                                                                                          _arg7, _arg8, _arg9, _arg10, _arg11, _arg12,
                                                                                                          _arg13>{};
                                                                  }
                                                                }
                                                              } else {
                                                                return as_types::operation_args_exprs<_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7,
                                                                                                      _arg8, _arg9, _arg10, _arg11, _arg12>{};
                                                              }
                                                            }
                                                          } else {
                                                            return as_types::operation_args_exprs<_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7, _arg8,
                                                                                                  _arg9, _arg10, _arg11>{};
                                                          }
                                                        }
                                                      } else {
                                                        return as_types::operation_args_exprs<_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7, _arg8,
                                                                                              _arg9, _arg10>{};
                                                      }
                                                    }
                                                  } else {
                                                    return as_types::operation_args_exprs<_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7, _arg8,
                                                                                          _arg9>{};
                                                  }
                                                }
                                              } else {
                                                return as_types::operation_args_exprs<_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7, _arg8>{};
                                              }
                                            }
                                          } else {
                                            return as_types::operation_args_exprs<_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7>{};
                                          }
                                        }
                                      } else {
                                        return as_types::operation_args_exprs<_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6>{};
                                      }
                                    }
                                  } else {
                                    return as_types::operation_args_exprs<_arg0, _arg1, _arg2, _arg3, _arg4, _arg5>{};
                                  }
                                }
                              } else {
                                return as_types::operation_args_exprs<_arg0, _arg1, _arg2, _arg3, _arg4>{};
                              }
                            }
                          } else {
                            return as_types::operation_args_exprs<_arg0, _arg1, _arg2, _arg3>{};
                          }
                        }
                      } else {
                        return as_types::operation_args_exprs<_arg0, _arg1, _arg2>{};
                      }
                    }
                  } else {
                    return as_types::operation_args_exprs<_arg0, _arg1>{};
                  }
                }
              } else {
                return as_types::operation_args_exprs<_arg0>{};
              }
            }
          } else {
            return as_types::operation_args_exprs<>{};
          }
        }
      } else if constexpr (e.template get_<operation_args_varrefs>().is_this_elem) {
        {
          constexpr std::size_t __indx = e.template get_<operation_args_varrefs>().t.vars[0].get_index();
          if constexpr (__indx > 0) {
            using arg0 = arg_struct<__indx>;
            using _arg0 = DECT(as_type<budget - 1, arg0>());
            {
              constexpr std::size_t __indx = e.template get_<operation_args_varrefs>().t.vars[1].get_index();
              if constexpr (__indx > 0) {
                using arg1 = arg_struct<__indx>;
                using _arg1 = DECT(as_type<budget - 1, arg1>());
                {
                  constexpr std::size_t __indx = e.template get_<operation_args_varrefs>().t.vars[2].get_index();
                  if constexpr (__indx > 0) {
                    using arg2 = arg_struct<__indx>;
                    using _arg2 = DECT(as_type<budget - 1, arg2>());
                    {
                      constexpr std::size_t __indx = e.template get_<operation_args_varrefs>().t.vars[3].get_index();
                      if constexpr (__indx > 0) {
                        using arg3 = arg_struct<__indx>;
                        using _arg3 = DECT(as_type<budget - 1, arg3>());
                        {
                          constexpr std::size_t __indx = e.template get_<operation_args_varrefs>().t.vars[4].get_index();
                          if constexpr (__indx > 0) {
                            using arg4 = arg_struct<__indx>;
                            using _arg4 = DECT(as_type<budget - 1, arg4>());
                            {
                              constexpr std::size_t __indx = e.template get_<operation_args_varrefs>().t.vars[5].get_index();
                              if constexpr (__indx > 0) {
                                using arg5 = arg_struct<__indx>;
                                using _arg5 = DECT(as_type<budget - 1, arg5>());
                                {
                                  constexpr std::size_t __indx = e.template get_<operation_args_varrefs>().t.vars[6].get_index();
                                  if constexpr (__indx > 0) {
                                    using arg6 = arg_struct<__indx>;
                                    using _arg6 = DECT(as_type<budget - 1, arg6>());
                                    {
                                      constexpr std::size_t __indx = e.template get_<operation_args_varrefs>().t.vars[7].get_index();
                                      if constexpr (__indx > 0) {
                                        using arg7 = arg_struct<__indx>;
                                        using _arg7 = DECT(as_type<budget - 1, arg7>());
                                        {
                                          constexpr std::size_t __indx = e.template get_<operation_args_varrefs>().t.vars[8].get_index();
                                          if constexpr (__indx > 0) {
                                            using arg8 = arg_struct<__indx>;
                                            using _arg8 = DECT(as_type<budget - 1, arg8>());
                                            {
                                              constexpr std::size_t __indx = e.template get_<operation_args_varrefs>().t.vars[9].get_index();
                                              if constexpr (__indx > 0) {
                                                using arg9 = arg_struct<__indx>;
                                                using _arg9 = DECT(as_type<budget - 1, arg9>());
                                                {
                                                  constexpr std::size_t __indx = e.template get_<operation_args_varrefs>().t.vars[10].get_index();
                                                  if constexpr (__indx > 0) {
                                                    using arg10 = arg_struct<__indx>;
                                                    using _arg10 = DECT(as_type<budget - 1, arg10>());
                                                    {
                                                      constexpr std::size_t __indx = e.template get_<operation_args_varrefs>().t.vars[11].get_index();
                                                      if constexpr (__indx > 0) {
                                                        using arg11 = arg_struct<__indx>;
                                                        using _arg11 = DECT(as_type<budget - 1, arg11>());
                                                        {
                                                          constexpr std::size_t __indx = e.template get_<operation_args_varrefs>().t.vars[12].get_index();
                                                          if constexpr (__indx > 0) {
                                                            using arg12 = arg_struct<__indx>;
                                                            using _arg12 = DECT(as_type<budget - 1, arg12>());
                                                            {
                                                              constexpr std::size_t __indx = e.template get_<operation_args_varrefs>().t.vars[13].get_index();
                                                              if constexpr (__indx > 0) {
                                                                using arg13 = arg_struct<__indx>;
                                                                using _arg13 = DECT(as_type<budget - 1, arg13>());
                                                                {
                                                                  constexpr std::size_t __indx =
                                                                    e.template get_<operation_args_varrefs>().t.vars[14].get_index();
                                                                  if constexpr (__indx > 0) {
                                                                    using arg14 = arg_struct<__indx>;
                                                                    using _arg14 = DECT(as_type<budget - 1, arg14>());
                                                                    {
                                                                      constexpr std::size_t __indx =
                                                                        e.template get_<operation_args_varrefs>().t.vars[15].get_index();
                                                                      if constexpr (__indx > 0) {
                                                                        using arg15 = arg_struct<__indx>;
                                                                        using _arg15 = DECT(as_type<budget - 1, arg15>());
                                                                        {
                                                                          constexpr std::size_t __indx =
                                                                            e.template get_<operation_args_varrefs>().t.vars[16].get_index();
                                                                          if constexpr (__indx > 0) {
                                                                            using arg16 = arg_struct<__indx>;
                                                                            using _arg16 = DECT(as_type<budget - 1, arg16>());
                                                                            {
                                                                              constexpr std::size_t __indx =
                                                                                e.template get_<operation_args_varrefs>().t.vars[17].get_index();
                                                                              if constexpr (__indx > 0) {
                                                                                using arg17 = arg_struct<__indx>;
                                                                                using _arg17 = DECT(as_type<budget - 1, arg17>());
                                                                                {
                                                                                  constexpr std::size_t __indx =
                                                                                    e.template get_<operation_args_varrefs>().t.vars[18].get_index();
                                                                                  if constexpr (__indx > 0) {
                                                                                    using arg18 = arg_struct<__indx>;
                                                                                    using _arg18 = DECT(as_type<budget - 1, arg18>());
                                                                                    {
                                                                                      constexpr std::size_t __indx =
                                                                                        e.template get_<operation_args_varrefs>().t.vars[19].get_index();
                                                                                      if constexpr (__indx > 0) {
                                                                                        using arg19 = arg_struct<__indx>;
                                                                                        using _arg19 = DECT(as_type<budget - 1, arg19>());
                                                                                        return as_types::operation_args_varrefs<
                                                                                          _arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7, _arg8, _arg9,
                                                                                          _arg10, _arg11, _arg12, _arg13, _arg14, _arg15, _arg16, _arg17,
                                                                                          _arg18, _arg19>{};
                                                                                      } else {
                                                                                        return as_types::operation_args_varrefs<
                                                                                          _arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7, _arg8, _arg9,
                                                                                          _arg10, _arg11, _arg12, _arg13, _arg14, _arg15, _arg16, _arg17,
                                                                                          _arg18>{};
                                                                                      }
                                                                                    }
                                                                                  } else {
                                                                                    return as_types::operation_args_varrefs<
                                                                                      _arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7, _arg8, _arg9,
                                                                                      _arg10, _arg11, _arg12, _arg13, _arg14, _arg15, _arg16, _arg17>{};
                                                                                  }
                                                                                }
                                                                              } else {
                                                                                return as_types::operation_args_varrefs<
                                                                                  _arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7, _arg8, _arg9, _arg10,
                                                                                  _arg11, _arg12, _arg13, _arg14, _arg15, _arg16>{};
                                                                              }
                                                                            }
                                                                          } else {
                                                                            return as_types::operation_args_varrefs<_arg0, _arg1, _arg2, _arg3, _arg4, _arg5,
                                                                                                                    _arg6, _arg7, _arg8, _arg9, _arg10, _arg11,
                                                                                                                    _arg12, _arg13, _arg14, _arg15>{};
                                                                          }
                                                                        }
                                                                      } else {
                                                                        return as_types::operation_args_varrefs<_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6,
                                                                                                                _arg7, _arg8, _arg9, _arg10, _arg11, _arg12,
                                                                                                                _arg13, _arg14>{};
                                                                      }
                                                                    }
                                                                  } else {
                                                                    return as_types::operation_args_varrefs<_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6,
                                                                                                            _arg7, _arg8, _arg9, _arg10, _arg11, _arg12,
                                                                                                            _arg13>{};
                                                                  }
                                                                }
                                                              } else {
                                                                return as_types::operation_args_varrefs<_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7,
                                                                                                        _arg8, _arg9, _arg10, _arg11, _arg12>{};
                                                              }
                                                            }
                                                          } else {
                                                            return as_types::operation_args_varrefs<_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7,
                                                                                                    _arg8, _arg9, _arg10, _arg11>{};
                                                          }
                                                        }
                                                      } else {
                                                        return as_types::operation_args_varrefs<_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7, _arg8,
                                                                                                _arg9, _arg10>{};
                                                      }
                                                    }
                                                  } else {
                                                    return as_types::operation_args_varrefs<_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7, _arg8,
                                                                                            _arg9>{};
                                                  }
                                                }
                                              } else {
                                                return as_types::operation_args_varrefs<_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7, _arg8>{};
                                              }
                                            }
                                          } else {
                                            return as_types::operation_args_varrefs<_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7>{};
                                          }
                                        }
                                      } else {
                                        return as_types::operation_args_varrefs<_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6>{};
                                      }
                                    }
                                  } else {
                                    return as_types::operation_args_varrefs<_arg0, _arg1, _arg2, _arg3, _arg4, _arg5>{};
                                  }
                                }
                              } else {
                                return as_types::operation_args_varrefs<_arg0, _arg1, _arg2, _arg3, _arg4>{};
                              }
                            }
                          } else {
                            return as_types::operation_args_varrefs<_arg0, _arg1, _arg2, _arg3>{};
                          }
                        }
                      } else {
                        return as_types::operation_args_varrefs<_arg0, _arg1, _arg2>{};
                      }
                    }
                  } else {
                    return as_types::operation_args_varrefs<_arg0, _arg1>{};
                  }
                }
              } else {
                return as_types::operation_args_varrefs<_arg0>{};
              }
            }
          } else {
            return as_types::operation_args_varrefs<>{};
          }
        }
      } else if constexpr (e.template get_<Operation>().is_this_elem) {
        using is_statement = std::integral_constant<bool, e.template get_<Operation>().t.is_statement>;
        if constexpr (is_statement::value) {
          constexpr auto& __str0 = e.template get_<Operation>().t.name;
          using _arg0 = DECT(
            mutils::String<__str0[0], __str0[1], __str0[2], __str0[3], __str0[4], __str0[5], __str0[6], __str0[7], __str0[8], __str0[9], __str0[10], __str0[11],
                           __str0[12], __str0[13], __str0[14], __str0[15], __str0[16], __str0[17], __str0[18], __str0[19]>::trim_ends()); /*Declaring arg!*/
          using arg1 = arg_struct<e.template get_<Operation>().t.Hndl.get_index()>;
          using _arg1 = DECT(as_type<budget - 1, arg1>());
          /*Declaring arg!*/ using arg2 = arg_struct<e.template get_<Operation>().t.expr_args.get_index()>;
          using _arg2 = DECT(as_type<budget - 1, arg2>());
          /*Declaring arg!*/ using arg3 = arg_struct<e.template get_<Operation>().t.var_args.get_index()>;
          using _arg3 = DECT(as_type<budget - 1, arg3>());
          return as_types::Statement<as_types::Operation<_arg0, _arg1, _arg2, _arg3>>{};
        } else {
          constexpr auto& __str0 = e.template get_<Operation>().t.name;
          using _arg0 = DECT(
            mutils::String<__str0[0], __str0[1], __str0[2], __str0[3], __str0[4], __str0[5], __str0[6], __str0[7], __str0[8], __str0[9], __str0[10], __str0[11],
                           __str0[12], __str0[13], __str0[14], __str0[15], __str0[16], __str0[17], __str0[18], __str0[19]>::trim_ends()); /*Declaring arg!*/
          using arg1 = arg_struct<e.template get_<Operation>().t.Hndl.get_index()>;
          using _arg1 = DECT(as_type<budget - 1, arg1>());
          /*Declaring arg!*/ using arg2 = arg_struct<e.template get_<Operation>().t.expr_args.get_index()>;
          using _arg2 = DECT(as_type<budget - 1, arg2>());
          /*Declaring arg!*/ using arg3 = arg_struct<e.template get_<Operation>().t.var_args.get_index()>;
          using _arg3 = DECT(as_type<budget - 1, arg3>());
          return as_types::Expression<as_types::Operation<_arg0, _arg1, _arg2, _arg3>>{};
        }
      } else if constexpr (e.template get_<Assignment>().is_this_elem) {
        /*Declaring arg!*/ using arg0 = arg_struct<e.template get_<Assignment>().t.Var.get_index()>;
        using _arg0 = DECT(as_type<budget - 1, arg0>());
        /*Declaring arg!*/ using arg1 = arg_struct<e.template get_<Assignment>().t.Expr.get_index()>;
        using _arg1 = DECT(as_type<budget - 1, arg1>());
        return as_types::Statement<as_types::Assignment<_arg0, _arg1>>{};
      } else if constexpr (e.template get_<Return>().is_this_elem) {
        /*Declaring arg!*/ using arg0 = arg_struct<e.template get_<Return>().t.Expr.get_index()>;
        using _arg0 = DECT(as_type<budget - 1, arg0>());
        return as_types::Statement<as_types::Return<_arg0>>{};
      } else if constexpr (e.template get_<If>().is_this_elem) {
        /*Declaring arg!*/ using arg0 = arg_struct<e.template get_<If>().t.condition.get_index()>;
        using _arg0 = DECT(as_type<budget - 1, arg0>());
        /*Declaring arg!*/ using arg1 = arg_struct<e.template get_<If>().t.then.get_index()>;
        using _arg1 = DECT(as_type<budget - 1, arg1>());
        /*Declaring arg!*/ using arg2 = arg_struct<e.template get_<If>().t.els.get_index()>;
        using _arg2 = DECT(as_type<budget - 1, arg2>());
        return as_types::Statement<as_types::If<_arg0, _arg1, _arg2>>{};
      } else if constexpr (e.template get_<While>().is_this_elem) {
        /*Declaring arg!*/ using arg0 = arg_struct<e.template get_<While>().t.condition.get_index()>;
        using _arg0 = DECT(as_type<budget - 1, arg0>());
        /*Declaring arg!*/ using arg1 = arg_struct<e.template get_<While>().t.body.get_index()>;
        using _arg1 = DECT(as_type<budget - 1, arg1>());
        return as_types::Statement<as_types::While<_arg0, _arg1>>{};
      } else if constexpr (e.template get_<Sequence>().is_this_elem) {
        /*Declaring arg!*/ using arg0 = arg_struct<e.template get_<Sequence>().t.e.get_index()>;
        using _arg0 = DECT(as_type<budget - 1, arg0>());
        /*Declaring arg!*/ using arg1 = arg_struct<e.template get_<Sequence>().t.next.get_index()>;
        using _arg1 = DECT(as_type<budget - 1, arg1>());
        return as_types::Statement<as_types::Sequence<_arg0, _arg1>>{};
      } else if constexpr (e.template get_<Skip>().is_this_elem) {
        return as_types::Statement<as_types::Skip>{};
      } else if constexpr (e.template get_<Binding>().is_this_elem) {
        constexpr const auto& str = e.template get_<Binding>().t.var;
        using _arg0 = DECT(mutils::String<str[0], str[1], str[2], str[3], str[4], str[5], str[6], str[7], str[8], str[9], str[10], str[11], str[12], str[13],
                                          str[14], str[15], str[16], str[17], str[18], str[19]>::trim_ends());
        /*Declaring arg!*/ struct arg1
        {
#ifndef __clang__
          const AST_elem& e{ F{}() };
#endif
          constexpr arg1() {}
          constexpr const AST_elem& operator()() const { return e.template get_<Binding>().t.rhs.get(allocator); }
        };

        using _arg1 = DECT(as_type<budget - 1, arg1>());

        return as_types::Binding<_arg0, _arg1>{};
      } else {
        struct error
        {};
        return error{};
      }
    } else {
      static_assert(budget > 0);
      struct error
      {};
      return error{};
    }
  }
};

template <typename prev_holder>
constexpr auto
as_type()
{
  struct arg
  {
    constexpr arg() {}
    constexpr const AST_elem& operator()() const { return prev_holder::prev.allocator.top.e.get(prev_holder::prev.allocator); }
  };
  return as_types::Statement<as_types::transaction<DECT(as_type_f<prev_holder>::template as_type<1000, arg>()), prev_holder::prev.allocator.top.payload>>{};
}
} // namespace as_values

namespace as_types {

template <typename>
struct sequence_assigner;

template <std::size_t... nums>
struct sequence_assigner<std::integer_sequence<std::size_t, nums...>>
{
  template <typename... T_args>
  struct helper
  {
    template <typename ref, typename F, typename... F_args>
    constexpr static void assign(plain_array<ref>& r, const F& f, F_args&... args)
    {
      ((r[nums] = f(T_args{}, args...)), ...);
    }
  };
};

template <typename AST_Allocator, std::size_t budget>
struct as_values_ns_fns
{
  using AST_elem = as_values::AST_elem;
  constexpr as_values_ns_fns() = default;
  as_values::AST_Allocator<budget> allocator;
  /*
  template<typename T> struct converter {
    static constexpr auto value() {return as_values_ns_fns::foo();}
  };*/
  template <typename ref, typename... type_args>
  constexpr void sequence_assign(plain_array<ref>& r)
  {
    constexpr auto function_arg = [](const auto& true_arg, auto& _this) constexpr { return _this.as_value(true_arg); };
    return sequence_assigner<std::make_index_sequence<sizeof...(type_args)>>::template helper<type_args...>::template assign<ref>(r, function_arg, *this);
  }

  template <typename e, std::size_t payload>
  constexpr allocated_ref<AST_elem> as_value(const Statement<transaction<e, payload>>&)
  {
    auto elem = allocator.template allocate<AST_elem>();
    auto& this_node = elem.get(allocator).template get_<as_values::transaction>();
    this_node.is_this_elem = true;
    elem.get(allocator).is_initialized = true;
    this_node.t.e = as_value(e{});
    this_node.t.payload = payload;
    return std::move(elem);
  }
  template <typename Struct, typename Field>
  constexpr allocated_ref<AST_elem> as_value(const Expression<FieldReference<Struct, Field>>&)
  {
    auto elem = allocator.template allocate<AST_elem>();
    auto& this_node = elem.get(allocator).template get_<as_values::FieldReference>();
    this_node.is_this_elem = true;
    elem.get(allocator).is_initialized = true;
    this_node.t.Struct = as_value(Struct{});
    mutils::cstring::str_cpy(this_node.t.Field, Field{}.string);
    return std::move(elem);
  }
  template <typename Struct, typename Field>
  constexpr allocated_ref<AST_elem> as_value(const Expression<FieldPointerReference<Struct, Field>>&)
  {
    auto elem = allocator.template allocate<AST_elem>();
    auto& this_node = elem.get(allocator).template get_<as_values::FieldPointerReference>();
    this_node.is_this_elem = true;
    elem.get(allocator).is_initialized = true;
    this_node.t.Struct = as_value(Struct{});
    mutils::cstring::str_cpy(this_node.t.Field, Field{}.string);
    return std::move(elem);
  }
  template <typename Struct>
  constexpr allocated_ref<AST_elem> as_value(const Expression<Dereference<Struct>>&)
  {
    auto elem = allocator.template allocate<AST_elem>();
    auto& this_node = elem.get(allocator).template get_<as_values::Dereference>();
    this_node.is_this_elem = true;
    elem.get(allocator).is_initialized = true;
    this_node.t.Struct = as_value(Struct{});
    return std::move(elem);
  }
  template <typename label, typename Hndl>
  constexpr allocated_ref<AST_elem> as_value(const Expression<Endorse<label, Hndl>>&)
  {
    auto elem = allocator.template allocate<AST_elem>();
    auto& this_node = elem.get(allocator).template get_<as_values::Endorse>();
    this_node.is_this_elem = true;
    elem.get(allocator).is_initialized = true;
    using ____label = typename label::label;
    mutils::cstring::str_cpy(this_node.t.label.label, ____label{}.string);
    this_node.t.Hndl = as_value(Hndl{});
    return std::move(elem);
  }
  template <typename label, typename Hndl>
  constexpr allocated_ref<AST_elem> as_value(const Expression<Ensure<label, Hndl>>&)
  {
    auto elem = allocator.template allocate<AST_elem>();
    auto& this_node = elem.get(allocator).template get_<as_values::Ensure>();
    this_node.is_this_elem = true;
    elem.get(allocator).is_initialized = true;
    using ____label = typename label::label;
    mutils::cstring::str_cpy(this_node.t.label.label, ____label{}.string);
    this_node.t.Hndl = as_value(Hndl{});
    return std::move(elem);
  }
  template <typename Hndl>
  constexpr allocated_ref<AST_elem> as_value(const Expression<IsValid<Hndl>>&)
  {
    auto elem = allocator.template allocate<AST_elem>();
    auto& this_node = elem.get(allocator).template get_<as_values::IsValid>();
    this_node.is_this_elem = true;
    elem.get(allocator).is_initialized = true;
    this_node.t.Hndl = as_value(Hndl{});
    return std::move(elem);
  }
  template <typename Var>
  constexpr allocated_ref<AST_elem> as_value(const Expression<VarReference<Var>>&)
  {
    auto elem = allocator.template allocate<AST_elem>();
    auto& this_node = elem.get(allocator).template get_<as_values::VarReference>();
    this_node.is_this_elem = true;
    elem.get(allocator).is_initialized = true;
    mutils::cstring::str_cpy(this_node.t.Var, Var{}.string);
    return std::move(elem);
  }
  template <std::size_t i>
  constexpr allocated_ref<AST_elem> as_value(const Expression<Constant<i>>&)
  {
    auto elem = allocator.template allocate<AST_elem>();
    auto& this_node = elem.get(allocator).template get_<as_values::Constant>();
    this_node.is_this_elem = true;
    elem.get(allocator).is_initialized = true;
    this_node.t.i = i;
    return std::move(elem);
  }
  template <char op, typename L, typename R>
  constexpr allocated_ref<AST_elem> as_value(const Expression<BinOp<op, L, R>>&)
  {
    auto elem = allocator.template allocate<AST_elem>();
    auto& this_node = elem.get(allocator).template get_<as_values::BinOp>();
    this_node.is_this_elem = true;
    elem.get(allocator).is_initialized = true;
    this_node.t.op = op;
    this_node.t.L = as_value(L{});
    this_node.t.R = as_value(R{});
    return std::move(elem);
  }
  template <typename Binding, typename Body>
  constexpr allocated_ref<AST_elem> as_value(const Statement<Let<Binding, Body>>&)
  {
    auto elem = allocator.template allocate<AST_elem>();
    auto& this_node = elem.get(allocator).template get_<as_values::Let>();
    this_node.is_this_elem = true;
    elem.get(allocator).is_initialized = true;
    this_node.t.Binding = as_value(Binding{});
    this_node.t.Body = as_value(Body{});
    return std::move(elem);
  }
  template <typename Binding, typename Body>
  constexpr allocated_ref<AST_elem> as_value(const Statement<LetRemote<Binding, Body>>&)
  {
    auto elem = allocator.template allocate<AST_elem>();
    auto& this_node = elem.get(allocator).template get_<as_values::LetRemote>();
    this_node.is_this_elem = true;
    elem.get(allocator).is_initialized = true;
    this_node.t.Binding = as_value(Binding{});
    this_node.t.Body = as_value(Body{});
    return std::move(elem);
  }

  template <typename... Args>
  constexpr allocated_ref<AST_elem> as_value(const operation_args_exprs<Args...>&)
  {
    auto elem = allocator.template allocate<AST_elem>();
    auto& this_node = elem.get(allocator).template get_<as_values::operation_args_exprs>();
    this_node.is_this_elem = true;
    elem.get(allocator).is_initialized = true;
    sequence_assign<allocated_ref<AST_elem>, Args...>(this_node.t.exprs);
    return std::move(elem);
  }

  template <typename... Args>
  constexpr allocated_ref<AST_elem> as_value(const operation_args_varrefs<Args...>&)
  {
    auto elem = allocator.template allocate<AST_elem>();
    auto& this_node = elem.get(allocator).template get_<as_values::operation_args_varrefs>();
    this_node.is_this_elem = true;
    elem.get(allocator).is_initialized = true;
    sequence_assign<allocated_ref<AST_elem>, Args...>(this_node.t.vars);
    return std::move(elem);
  }
  template <typename name, typename Hndl, typename expr_args, typename var_args>
  constexpr allocated_ref<AST_elem> as_value(const Statement<Operation<name, Hndl, expr_args, var_args>>&)
  {
    auto elem = allocator.template allocate<AST_elem>();
    auto& this_node = elem.get(allocator).template get_<as_values::Operation>();
    this_node.is_this_elem = true;
    elem.get(allocator).is_initialized = true;
    mutils::cstring::str_cpy(this_node.t.name, name{}.string);
    this_node.t.Hndl = as_value(Hndl{});
    this_node.t.expr_args = as_value(expr_args{});
    this_node.t.var_args = as_value(var_args{});
    this_node.t.is_statement = true;
    return std::move(elem);
  }
  template <typename name, typename Hndl, typename expr_args, typename var_args>
  constexpr allocated_ref<AST_elem> as_value(const Expression<Operation<name, Hndl, expr_args, var_args>>&)
  {
    auto elem = allocator.template allocate<AST_elem>();
    auto& this_node = elem.get(allocator).template get_<as_values::Operation>();
    this_node.is_this_elem = true;
    elem.get(allocator).is_initialized = true;
    mutils::cstring::str_cpy(this_node.t.name, name{}.string);
    this_node.t.Hndl = as_value(Hndl{});
    this_node.t.expr_args = as_value(expr_args{});
    this_node.t.var_args = as_value(var_args{});
    this_node.t.is_statement = false;
    return std::move(elem);
  }
  template <typename Var, typename Expr>
  constexpr allocated_ref<AST_elem> as_value(const Statement<Assignment<Var, Expr>>&)
  {
    auto elem = allocator.template allocate<AST_elem>();
    auto& this_node = elem.get(allocator).template get_<as_values::Assignment>();
    this_node.is_this_elem = true;
    elem.get(allocator).is_initialized = true;
    this_node.t.Var = as_value(Var{});
    this_node.t.Expr = as_value(Expr{});
    return std::move(elem);
  }
  template <typename Expr>
  constexpr allocated_ref<AST_elem> as_value(const Statement<Return<Expr>>&)
  {
    auto elem = allocator.template allocate<AST_elem>();
    auto& this_node = elem.get(allocator).template get_<as_values::Return>();
    this_node.is_this_elem = true;
    elem.get(allocator).is_initialized = true;
    this_node.t.Expr = as_value(Expr{});
    return std::move(elem);
  }
  template <typename condition, typename then, typename els>
  constexpr allocated_ref<AST_elem> as_value(const Statement<If<condition, then, els>>&)
  {
    auto elem = allocator.template allocate<AST_elem>();
    auto& this_node = elem.get(allocator).template get_<as_values::If>();
    this_node.is_this_elem = true;
    elem.get(allocator).is_initialized = true;
    this_node.t.condition = as_value(condition{});
    this_node.t.then = as_value(then{});
    this_node.t.els = as_value(els{});
    return std::move(elem);
  }
  template <typename condition, typename body>
  constexpr allocated_ref<AST_elem> as_value(const Statement<While<condition, body>>&)
  {
    auto elem = allocator.template allocate<AST_elem>();
    auto& this_node = elem.get(allocator).template get_<as_values::While>();
    this_node.is_this_elem = true;
    elem.get(allocator).is_initialized = true;
    this_node.t.condition = as_value(condition{});
    this_node.t.body = as_value(body{});
    return std::move(elem);
  }
  template <typename e, typename next>
  constexpr allocated_ref<AST_elem> as_value(const Statement<Sequence<e, next>>&)
  {
    auto elem = allocator.template allocate<AST_elem>();
    auto& this_node = elem.get(allocator).template get_<as_values::Sequence>();
    this_node.is_this_elem = true;
    elem.get(allocator).is_initialized = true;
    this_node.t.e = as_value(e{});
    this_node.t.next = as_value(next{});
    return std::move(elem);
  }
  constexpr allocated_ref<AST_elem> as_value(const Statement<Skip>&)
  {
    auto elem = allocator.template allocate<AST_elem>();
    auto& this_node = elem.get(allocator).template get_<as_values::Skip>();
    this_node.is_this_elem = true;
    elem.get(allocator).is_initialized = true;
    return std::move(elem);
  }

  template <char... str, typename t>
  constexpr allocated_ref<AST_elem> as_value(const Binding<mutils::String<str...>, Expression<t>>&)
  {
    auto elem = allocator.template allocate<AST_elem>();
    auto& this_node = elem.get(allocator).template get_<as_values::Binding>();
    this_node.is_this_elem = true;
    elem.get(allocator).is_initialized = true;
    this_node.t.rhs = as_value(Expression<t>{});
    mutils::cstring::str_cpy(this_node.t.var, mutils::String<str...>{}.string);
    return std::move(elem);
  }
};

template <std::size_t budget, typename hd>
constexpr as_values::AST_Allocator<budget>
as_value()
{
  static_assert(is_astnode_transaction<hd>::value);
  as_values_ns_fns<as_values::AST_Allocator<budget>, budget> ret;
  ret.allocator.top = std::move(ret.as_value(hd{}).get(ret.allocator).template get<as_values::transaction>());
  return std::move(ret.allocator);
}
} // namespace as_types

namespace as_values {
template <typename Allocator>
std::ostream&
print(std::ostream& o, const std::size_t& st, const Allocator&)
{
  return o << st;
}
template <typename Allocator>
std::ostream& print(std::ostream& o, const AST_elem& e, const Allocator& allocator);

template <typename Allocator>
std::ostream&
print(std::ostream& o, const transaction& e, const Allocator& allocator)
{
  o << "transaction{";
  print(o, e.e, allocator);
  o << ",";
  print(o, e.payload, allocator);
  o << ",";
  return o << "}";
}
template <typename Allocator>
std::ostream&
print(std::ostream& o, const FieldReference& e, const Allocator& allocator)
{
  o << "FieldReference{";
  print(o, e.Struct, allocator);
  o << ",";
  print(o, e.Field, allocator);
  o << ",";
  return o << "}";
}
template <typename Allocator>
std::ostream&
print(std::ostream& o, const FieldPointerReference& e, const Allocator& allocator)
{
  o << "FieldPointerReference{";
  print(o, e.Struct, allocator);
  o << ",";
  print(o, e.Field, allocator);
  o << ",";
  return o << "}";
}
template <typename Allocator>
std::ostream&
print(std::ostream& o, const Dereference& e, const Allocator& allocator)
{
  o << "Dereference{";
  print(o, e.Struct, allocator);
  o << ",";
  return o << "}";
}
template <typename Allocator>
std::ostream&
print(std::ostream& o, const Endorse& e, const Allocator& allocator)
{
  o << "Endorse{";
  print(o, e.label, allocator);
  o << ",";
  print(o, e.Hndl, allocator);
  o << ",";
  return o << "}";
}
template <typename Allocator>
std::ostream&
print(std::ostream& o, const Ensure& e, const Allocator& allocator)
{
  o << "Ensure{";
  print(o, e.label, allocator);
  o << ",";
  print(o, e.Hndl, allocator);
  o << ",";
  return o << "}";
}
template <typename Allocator>
std::ostream&
print(std::ostream& o, const IsValid& e, const Allocator& allocator)
{
  o << "IsValid{";
  print(o, e.Hndl, allocator);
  o << ",";
  return o << "}";
}
template <typename Allocator>
std::ostream&
print(std::ostream& o, const VarReference& e, const Allocator& allocator)
{
  o << "VarReference{";
  print(o, e.Var, allocator);
  o << ",";
  return o << "}";
}
template <typename Allocator>
std::ostream&
print(std::ostream& o, const Constant& e, const Allocator& allocator)
{
  o << "Constant{";
  print(o, e.i, allocator);
  o << ",";
  return o << "}";
}
template <typename Allocator>
std::ostream&
print(std::ostream& o, const BinOp& e, const Allocator& allocator)
{
  o << "BinOp{";
  print(o, e.op, allocator);
  o << ",";
  print(o, e.L, allocator);
  o << ",";
  print(o, e.R, allocator);
  o << ",";
  return o << "}";
}
template <typename Allocator>
std::ostream&
print(std::ostream& o, const Let& e, const Allocator& allocator)
{
  o << "Let{";
  print(o, e.Binding, allocator);
  o << ",";
  print(o, e.Body, allocator);
  o << ",";
  return o << "}";
}
template <typename Allocator>
std::ostream&
print(std::ostream& o, const LetRemote& e, const Allocator& allocator)
{
  o << "LetRemote{";
  print(o, e.Binding, allocator);
  o << ",";
  print(o, e.Body, allocator);
  o << ",";
  return o << "}";
}
template <typename Allocator>
std::ostream&
print(std::ostream& o, const operation_args_exprs& e, const Allocator& allocator)
{
  o << "operation_args_exprs{";
  print(o, e.exprs, allocator);
  o << ",";
  return o << "}";
}
template <typename Allocator>
std::ostream&
print(std::ostream& o, const operation_args_varrefs& e, const Allocator& allocator)
{
  o << "operation_args_varrefs{";
  print(o, e.vars, allocator);
  o << ",";
  return o << "}";
}
template <typename Allocator>
std::ostream&
print(std::ostream& o, const Operation& e, const Allocator& allocator)
{
  o << "Operation{";
  print(o, e.name, allocator);
  o << ",";
  print(o, e.Hndl, allocator);
  o << ",";
  print(o, e.expr_args, allocator);
  o << ",";
  print(o, e.var_args, allocator);
  o << ",";
  return o << "}";
}
template <typename Allocator>
std::ostream&
print(std::ostream& o, const Assignment& e, const Allocator& allocator)
{
  o << "Assignment{";
  print(o, e.Var, allocator);
  o << ",";
  print(o, e.Expr, allocator);
  o << ",";
  return o << "}";
}
template <typename Allocator>
std::ostream&
print(std::ostream& o, const Return& e, const Allocator& allocator)
{
  o << "Return{";
  print(o, e.Expr, allocator);
  o << ",";
  return o << "}";
}
template <typename Allocator>
std::ostream&
print(std::ostream& o, const If& e, const Allocator& allocator)
{
  o << "If{";
  print(o, e.condition, allocator);
  o << ",";
  print(o, e.then, allocator);
  o << ",";
  print(o, e.els, allocator);
  o << ",";
  return o << "}";
}
template <typename Allocator>
std::ostream&
print(std::ostream& o, const While& e, const Allocator& allocator)
{
  o << "While{";
  print(o, e.condition, allocator);
  o << ",";
  print(o, e.body, allocator);
  o << ",";
  return o << "}";
}
template <typename Allocator>
std::ostream&
print(std::ostream& o, const Sequence& e, const Allocator& allocator)
{
  o << "Sequence{";
  print(o, e.e, allocator);
  o << ",";
  print(o, e.next, allocator);
  o << ",";
  return o << "}";
}
template <typename Allocator>
std::ostream&
print(std::ostream& o, const Skip&, const Allocator&)
{
  o << "Skip{";
  return o << "}";
}
template <typename Allocator>
std::ostream&
print(std::ostream& o, const Binding& b, const Allocator& allocator)
{
  o << b.var << " = ";
  return print(o, b.rhs, allocator);
}

template <char... c, typename Allocator>
std::ostream&
print(std::ostream& o, const mutils::String<c...>, const Allocator&)
{
  return o << mutils::String<c...>{}.string;
}

template <typename Allocator>
std::ostream&
print(std::ostream& o, const plain_array<char>& cstr, const Allocator&)
{
  const char* str = cstr;
  return o << str;
}

template <typename Allocator>
std::ostream&
print(std::ostream& o, const Label& l, const Allocator&)
{
  return o << "Label[" << l.label << "]";
}

template <typename Allocator>
std::ostream&
print(std::ostream& o, const plain_array<allocated_ref<myria::mtl::new_parse_phase::as_values::AST_elem>>& arr, const Allocator& a)
{
  o << "{";
  for (auto& ref : arr) {
    if (ref) {
      print(o, ref, a);
      o << ",";
    }
  }
  return o << "}";
}

template <typename Allocator>
std::ostream&
print(std::ostream& o, const AST_elem& e, const Allocator& allocator)
{
  if (e.template get_<transaction>().is_this_elem) {
    return print(o, e.template get<transaction>(), allocator);
  }
  if (e.template get_<FieldReference>().is_this_elem) {
    return print(o, e.template get<FieldReference>(), allocator);
  }
  if (e.template get_<FieldPointerReference>().is_this_elem) {
    return print(o, e.template get<FieldPointerReference>(), allocator);
  }
  if (e.template get_<Dereference>().is_this_elem) {
    return print(o, e.template get<Dereference>(), allocator);
  }
  if (e.template get_<Endorse>().is_this_elem) {
    return print(o, e.template get<Endorse>(), allocator);
  }
  if (e.template get_<Ensure>().is_this_elem) {
    return print(o, e.template get<Ensure>(), allocator);
  }
  if (e.template get_<IsValid>().is_this_elem) {
    return print(o, e.template get<IsValid>(), allocator);
  }
  if (e.template get_<VarReference>().is_this_elem) {
    return print(o, e.template get<VarReference>(), allocator);
  }
  if (e.template get_<Constant>().is_this_elem) {
    return print(o, e.template get<Constant>(), allocator);
  }
  if (e.template get_<BinOp>().is_this_elem) {
    return print(o, e.template get<BinOp>(), allocator);
  }
  if (e.template get_<Let>().is_this_elem) {
    return print(o, e.template get<Let>(), allocator);
  }
  if (e.template get_<LetRemote>().is_this_elem) {
    return print(o, e.template get<LetRemote>(), allocator);
  }
  if (e.template get_<operation_args_exprs>().is_this_elem) {
    return print(o, e.template get<operation_args_exprs>(), allocator);
  }
  if (e.template get_<operation_args_varrefs>().is_this_elem) {
    return print(o, e.template get<operation_args_varrefs>(), allocator);
  }
  if (e.template get_<Operation>().is_this_elem) {
    return print(o, e.template get<Operation>(), allocator);
  }
  if (e.template get_<Assignment>().is_this_elem) {
    return print(o, e.template get<Assignment>(), allocator);
  }
  if (e.template get_<Return>().is_this_elem) {
    return print(o, e.template get<Return>(), allocator);
  }
  if (e.template get_<If>().is_this_elem) {
    return print(o, e.template get<If>(), allocator);
  }
  if (e.template get_<While>().is_this_elem) {
    return print(o, e.template get<While>(), allocator);
  }
  if (e.template get_<Sequence>().is_this_elem) {
    return print(o, e.template get<Sequence>(), allocator);
  }
  if (e.template get_<Skip>().is_this_elem) {
    return print(o, e.template get<Skip>(), allocator);
  }
  if (e.template get_<Binding>().is_this_elem) {
    return print(o, e.template get<Binding>(), allocator);
  }
  return o;
}

} // namespace as_values
namespace as_values {

template <typename Allocator, typename size_t>
std::ostream&
pretty_print(std::ostream& o, const size_t& st, const Allocator&, std::enable_if_t<std::is_same_v<size_t, std::size_t>>* = nullptr)
{
  return o << st;
}
template <typename Allocator>
std::ostream& pretty_print(std::ostream& o, const AST_elem& e, const Allocator& allocator);

template <typename Allocator>
std::ostream&
pretty_print(std::ostream& o, const Binding& b, const Allocator& allocator)
{
  o << b.var << " = ";
  return pretty_print(o, b.rhs, allocator);
}

template <char... c, typename Allocator>
std::ostream&
pretty_print(std::ostream& o, const mutils::String<c...>, const Allocator&)
{
  return o << mutils::String<c...>{}.string;
}

template <typename Allocator>
std::ostream&
pretty_print(std::ostream& o, const char* cstr, const Allocator&)
{
  return o << cstr;
}

template <typename Allocator>
std::ostream&
pretty_print(std::ostream& o, const char cstr, const Allocator&)
{
  char str[2] = { cstr, 0 };
  return o << str;
}

template <typename Allocator>
std::ostream&
pretty_print(std::ostream& o, const AST_elem& e, const Allocator& allocator)
{
  if (e.template get_<transaction>().is_this_elem) {
    return pretty_print(o, e.template get<transaction>(), allocator);
  }
  if (e.template get_<FieldReference>().is_this_elem) {
    return pretty_print(o, e.template get<FieldReference>(), allocator);
  }
  if (e.template get_<FieldPointerReference>().is_this_elem) {
    return pretty_print(o, e.template get<FieldPointerReference>(), allocator);
  }
  if (e.template get_<Dereference>().is_this_elem) {
    return pretty_print(o, e.template get<Dereference>(), allocator);
  }
  if (e.template get_<Endorse>().is_this_elem) {
    return pretty_print(o, e.template get<Endorse>(), allocator);
  }
  if (e.template get_<Ensure>().is_this_elem) {
    return pretty_print(o, e.template get<Ensure>(), allocator);
  }
  if (e.template get_<IsValid>().is_this_elem) {
    return pretty_print(o, e.template get<IsValid>(), allocator);
  }
  if (e.template get_<VarReference>().is_this_elem) {
    return pretty_print(o, e.template get<VarReference>(), allocator);
  }
  if (e.template get_<Constant>().is_this_elem) {
    return pretty_print(o, e.template get<Constant>(), allocator);
  }
  if (e.template get_<BinOp>().is_this_elem) {
    return pretty_print(o, e.template get<BinOp>(), allocator);
  }
  if (e.template get_<Let>().is_this_elem) {
    return pretty_print(o, e.template get<Let>(), allocator);
  }
  if (e.template get_<LetRemote>().is_this_elem) {
    return pretty_print(o, e.template get<LetRemote>(), allocator);
  }
  if (e.template get_<operation_args_exprs>().is_this_elem) {
    return pretty_print(o, e.template get<operation_args_exprs>(), allocator);
  }
  if (e.template get_<operation_args_varrefs>().is_this_elem) {
    return pretty_print(o, e.template get<operation_args_varrefs>(), allocator);
  }
  if (e.template get_<Operation>().is_this_elem) {
    return pretty_print(o, e.template get<Operation>(), allocator);
  }
  if (e.template get_<Assignment>().is_this_elem) {
    return pretty_print(o, e.template get<Assignment>(), allocator);
  }
  if (e.template get_<Return>().is_this_elem) {
    return pretty_print(o, e.template get<Return>(), allocator);
  }
  if (e.template get_<If>().is_this_elem) {
    return pretty_print(o, e.template get<If>(), allocator);
  }
  if (e.template get_<While>().is_this_elem) {
    return pretty_print(o, e.template get<While>(), allocator);
  }
  if (e.template get_<Sequence>().is_this_elem) {
    return pretty_print(o, e.template get<Sequence>(), allocator);
  }
  if (e.template get_<Skip>().is_this_elem) {
    return pretty_print(o, e.template get<Skip>(), allocator);
  }
  if (e.template get_<Binding>().is_this_elem) {
    return pretty_print(o, e.template get<Binding>(), allocator);
  }
  return o;
}
} // namespace as_values

} // namespace new_parse_phase
} // namespace mtl
} // namespace myria
