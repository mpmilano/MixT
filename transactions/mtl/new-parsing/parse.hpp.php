#pragma once
#include "ast.hpp"

/*
<?php require_once 'common.php'; require_once 'util.php'; ?>
*/
namespace myria{
namespace mtl{
namespace new_parse_phase{

struct parse_error : public std::logic_error {
  template<typename... T>
  parse_error(T&&... t):std::logic_error(std::forward<T>(t)...){}
};

using Alloc = as_values::AST_Allocator<400>;

template <typename string> struct parse {
  const string _str;
  using string_length = std::integral_constant<std::size_t,::mutils::cstring::str_len(string{}.str)>;
  using str_t = char const[string_length::value + 1];
  using str_nc = char[string_length::value + 1];
  Alloc allocator;

  constexpr allocated_ref<as_values::AST_elem> parse_binop(const str_t& str, const char* cause){
    using namespace mutils;
    using namespace cstring;
    <?php echo alloc("ret", "ref","BinOp")?>
    str_nc operands[2] = {{0}};
    last_split(cause,str,operands);
    ref.L = parse_expression(operands[0]);
    ref.R = parse_expression(operands[1]);
    ref.op = cause[0];
    return ret;
  }

  constexpr allocated_ref<as_values::AST_elem> parse_builtin_op(const str_t& str, const char* specific_op){
    /*
    <?php function instantiate_builtin_op($type, $arg = "") : string{
      return "
      {".alloc("ret", "ref","$type")."
      ref.Hndl = parse_expression(split[0]); ".
      ($arg !== "" ? 
        "trim(ref.$arg.label, op_args);" : "")."
      return ret;}
      ";
    }?>
*/
    using namespace mutils;
    using namespace cstring;
    str_nc trimmed = {0};
    trim(trimmed,str);
    str_nc split[2] = {{0}};
    last_split('.',trimmed,split);
    str_nc op_args = {0};
    copy_within_parens(op_args,split[1]);
    switch (specific_op[2]){
      case 'd': <?php echo instantiate_builtin_op("Endorse","label") ?> //endorse
      case 's': <?php echo instantiate_builtin_op("Ensure","label") ?> //ensure
      case 'V': <?php echo instantiate_builtin_op("IsValid") ?> //isValid
    }
    throw parse_error{"Internal Error: ran off the end finding builtin operations."};
  }

  constexpr allocated_ref<as_values::AST_elem> parse_args(const str_t &str){
    using namespace mutils;
    using namespace cstring;
    <?php echo alloc("eargs","evags","operation_args_exprs")?>
    str_nc expr_strs[<?php echo $max_var_length ?>] = {{0}};
    split_outside_parens(',',str,expr_strs);
    for (auto i = 0u; i < <?php echo $max_var_length ?>; ++i){
      if (expr_strs[i][0]) evags.exprs[i] = parse_expression(expr_strs[i]);
    }
    return eargs;
  }

  constexpr allocated_ref<as_values::AST_elem> parse_operation(const str_t &str, bool is_statement){
    using namespace mutils;
    using namespace cstring;
    str_nc splits[2] = {{0}};
    last_split('.',str,splits);
    auto& Struct = splits[0];
    auto& Call = splits[1];
    <?php echo alloc("ret", "ref","Operation")?>
    <?php echo alloc("vargs","rvags","operation_args_varrefs")?>
    (void) rvags;
    ref.is_statement = is_statement;
    ref.var_args = std::move(vargs);
    str_nc method_name = {0};
    pre_paren(method_name,Call);
    str_cpy(ref.name, method_name);
    str_nc argseq = {0};
    next_paren_group(argseq,Call);
    ref.expr_args = parse_args(argseq);
    ref.Hndl = parse_expression(Struct);
    return ret;
  }

  constexpr allocated_ref<as_values::AST_elem> parse_expr_operation(const str_t &str){
    return parse_operation(str,false);
  }
/*
<?php function error_check($name, $where, ...$look_for){
  $ret = "if (contains_paren($where)) throw parse_error{\"Parse error: We thought this was a $name, but it contains parens\"};";
  foreach ($look_for as $target){
    $ret = $ret."if (contains_outside_parens(\"$target\",$where)) throw parse_error{\"Parse error: This should be a $name, but it contains a '$target', which is not allowed\"};";
  }
  return $ret;
}?>
*/
  constexpr allocated_ref<as_values::AST_elem> parse_fieldref(const str_t& str){
    using namespace mutils;
    using namespace cstring;
    str_nc trimmed = {0};
    str_nc operands[2] = {{0}};
    trim(trimmed,str);
    last_split('.',trimmed,operands);
    {
      //error checking
      <?php echo error_check("fieldref","operands[1]",'.','->','*',' ')?>
    }
    <?php echo alloc("ret", "ref","FieldReference")?>
    ref.Struct = parse_expression(operands[0]);
    str_nc field_trimmed = {0};
    trim(field_trimmed,operands[1]);
    str_cpy(ref.Field, field_trimmed);
    return ret;
  }

  constexpr allocated_ref<as_values::AST_elem> parse_fieldptrref(const str_t& str, const char* ){
    using namespace mutils;
    using namespace cstring;
    str_nc trimmed = {0};
    str_nc operands[2] = {{0}};
    trim(trimmed,str);
    last_split("->",trimmed,operands);
    {
      //error checking
      <?php echo error_check("fieldptrref","operands[1]",'.','->','*',' ')?>
    }
    <?php echo alloc("ret", "ref","FieldPointerReference")?>
    ref.Struct = parse_expression(operands[0]);
    str_nc field_trimmed = {0};
    trim(field_trimmed,operands[1]);
    str_cpy(ref.Field, field_trimmed);
    return ret;
  }

  constexpr allocated_ref<as_values::AST_elem> parse_deref(const str_t& str, const char* ){
    using namespace mutils;
    using namespace cstring;
    str_nc trimmed = {0};
    trim(trimmed,str);
    assert(trimmed[0] = '*'); //this better be true;
    str_nc body = {0};
    str_cpy(body,trimmed+1);
    <?php echo alloc("ret", "ref","Dereference")?>
    ref.Struct = parse_expression(body);
    return ret;
  }

  constexpr allocated_ref<as_values::AST_elem> parse_constant(const str_t& str){
    using namespace mutils;
    using namespace cstring;
    <?php echo alloc("ret", "ref","Constant")?>
    ref.i = parse_int(str);
    return ret;
  }

  constexpr allocated_ref<as_values::AST_elem> parse_varref(const str_t& str){
    using namespace mutils;
    using namespace cstring;
    <?php echo alloc("ret", "ref","VarReference")?>
    trim(ref.Var,str);
		{
			//error checking
      <?php echo error_check("variable access",'ref.Var','.','->','*',' ')?>
		}
    return ret;
  }

/*
<?php function parse_expr($subcase,$containstr, ...$symbols ) : string {
  $ret = "";
  foreach ($symbols as $symbol){
    $ret = $ret."if (contains_outside_parens(\"$symbol\",$containstr)){
      return parse_$subcase(str,\"$symbol\");
    } else ";
  }
  return $ret;

} ?>
*/

  constexpr allocated_ref<as_values::AST_elem> parse_expression(const str_t &str) {
    using namespace mutils;
    using namespace cstring;
    <?php echo parse_expr("binop","str","+","- ","* ","/","==","&&","||","!=",'>','<','>=','<=') ?>
    if (contains_outside_parens(".",str)){
      str_nc pretrim_splits[2] = {{0}};
      last_split(".",str,pretrim_splits);
      str_nc splits[2] = {{0}};
      trim(splits[0],pretrim_splits[0]);
      trim(splits[1],pretrim_splits[1]);
      assert(!contains_outside_parens(".",splits[1]));
      <?php echo parse_expr("fieldptrref","splits[1]","->")?>
      <?php echo parse_expr("builtin_op","splits[1]","isValid(","endorse(","ensure(") ?> 
      if (contains_paren(splits[1])){
        return parse_expr_operation(str);
      }
      else {
        //it's just a normal string at this point.
        return parse_fieldref(str);
      }
    }
    else <?php echo parse_expr("fieldptrref","str","->")?>
    <?php echo parse_expr("deref","str","*")?>
    {
      //constants and variables here.
      str_nc atom = {0};
      trim(atom,str);
      static_assert('0' < '9');
      if (atom[0] >= '0' && atom[0] <= '9') return parse_constant(atom);
      else return parse_varref(atom);
    }
    throw parse_error{std::string{"Parse Error:  Could not find Expression to match input of "} + str};
  }

  constexpr allocated_ref<as_values::AST_elem> parse_binding(const str_t &str){
    using namespace mutils;
    using namespace cstring;
    <?php echo alloc("ret","binding","Binding")?>;
    str_nc binding_components[2] = {{0}};
    first_split('=',str,binding_components);
    trim(binding.var,binding_components[0]);
    binding.rhs = parse_expression(binding_components[1]);
    return ret;
  }

constexpr allocated_ref<as_values::AST_elem> parse_var(const str_t &str) {
    using namespace mutils;
    using namespace cstring;
    <?php echo alloc("ret","var","Let")?>
    str_nc let_expr = {0};
    remove_first_word(let_expr,str);
    str_nc let_components[2] = {{0}};
    first_split(',',let_expr,let_components);
    var.Binding = parse_binding(let_components[0]);
    var.Body = parse_statement(let_components[1]);
    return ret;
}
constexpr allocated_ref<as_values::AST_elem> parse_return(const str_t &str) {
    using namespace mutils;
    using namespace cstring;
    <?php echo alloc("ret","retref","Return")?>
    str_nc next = {0};
    remove_first_word(next,str);
    retref.Expr = parse_expression(next);
    return ret;
}
constexpr allocated_ref<as_values::AST_elem> parse_while(const str_t &str) {
    using namespace mutils;
    using namespace cstring;
    <?php echo alloc("ret","whileref","While")?>
    str_nc without_first = {0};
    remove_first_word(without_first,str);
    str_nc condition = {0};
    auto offset = next_paren_group(condition,without_first);
    str_nc body = {0};
    copy_within_parens(body, without_first + offset);
    whileref.condition = parse_expression(condition);
    whileref.body = parse_statement(body);
    return ret;
}
constexpr allocated_ref<as_values::AST_elem> parse_if(const str_t &str) {
    using namespace mutils;
    using namespace cstring;
    <?php echo alloc("ret","ifref","If")?>
    str_nc if_condition = {0};
    auto if_offset = next_paren_group(if_condition,str);
    const char* post_condition = str + if_offset;
    str_nc then_body = {0};
    auto then_offset = next_paren_group(then_body,post_condition);
    const char* post_then = then_offset + post_condition;
    str_nc else_body = {0};
    ifref.condition = parse_expression(if_condition);
    ifref.then = parse_statement(then_body);
    if (first_word_is("else",post_then)){
      next_paren_group(else_body,post_then);
      ifref.els = parse_statement(else_body);
    }
    else {
      <?php echo alloc("ptr","skipref","Skip")?>
      (void)skipref;
      ifref.els = std::move(ptr);
    }
    return ret;
}
constexpr allocated_ref<as_values::AST_elem> parse_assignment(const str_t &str) {
    using namespace mutils;
    using namespace cstring;
    <?php echo alloc("ret","assignment","Assignment")?>
    str_nc string_bufs[2] = {{0}};
    split_outside_parens('=',str,string_bufs);
    assignment.Var = parse_expression(string_bufs[0]);
    assignment.Expr = parse_expression(string_bufs[1]);
    return ret;
}

/* <?php function parse_case($target) { 
    return "
    if (contains_outside_parens(\"$target\",str)) {
        //assert(first_word_is(\"$target\",str));
        return parse_$target(str);
    }
    ";
} ?> */

  constexpr allocated_ref<as_values::AST_elem> parse_statement(const str_t &str) {
    using namespace mutils;
    using namespace cstring;
    if (first_word_is("var",str)){
      return parse_var(str);
    }
    else if (contains_outside_parens(',',str)){
        return parse_sequence(str);
    }
    else <?php echo parse_case("return")?>
    else <?php echo parse_case("while")?>
    else <?php echo parse_case("if")?>
    else if (contains_outside_parens("=",str)){
        return parse_assignment(str);
    }
    else if (first_word_is("{",str)){
        str_nc new_string = {0};
        copy_within_parens(new_string, str);
        return parse_statement(new_string);
    }
    else if (contains_paren(str)){
      return parse_operation(str,true);
    }
    else {
      str_nc trimit = {0};
      trim(trimit,str);
      if (str[0] == 0){
        <?php echo alloc("ret","sr","Skip") ?>;
        (void) sr;
        return ret;
      }
    }
    throw parse_error{std::string{"Parse Error:  Could not find Statement to match input of "} + str};
  }

  constexpr allocated_ref<as_values::AST_elem> parse_sequence(const str_t &str) {
    using namespace mutils;
    using namespace cstring;
    <?php echo alloc("ret","seqref","Sequence") ?>
    auto *seq = &seqref;
    str_nc string_bufs[2] = {{0}};
    first_split(',', str, string_bufs);
    seq->e = parse_statement(string_bufs[0]);
    seq->next = parse_statement(string_bufs[1]);
    return ret;
  }

  constexpr parse() {
    str_nc local_copy{0};
    constexpr string w;
    for (std::size_t i = 0; i < string_length::value; ++i){
      local_copy[i] = w.str[i];
    }
    // all parsing implemented in the constructor, so that
    // future things can just build this and expect it to work
    using namespace mutils::cstring;
    if (contains(';',local_copy)) throw parse_error{"Parse Error: Semicolons have no place here.  Did you mean ','? "};
    allocator.top.e = parse_statement(local_copy);
  }
};
}}}