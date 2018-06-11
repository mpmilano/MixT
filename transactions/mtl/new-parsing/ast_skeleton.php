#pragma once
#include "allocator.hpp"
#include "mutils/type_utils.hpp"
#include "mutils/CTString.hpp"
#include "mutils/cstring.hpp"
#include "union.hpp"
#include <ostream>

namespace myria{
namespace mtl{
namespace new_parse_phase{

template<typename T>
using plain_array = T[<?php echo $max_var_length?>];


namespace as_values {

  struct Label{
    plain_array<char> label = {0};
    constexpr Label() = default;
    constexpr Label(const Label&) = delete;
    constexpr Label(Label&& l){
      ::mutils::cstring::str_cpy(label,l.label);
    }
    constexpr Label& operator=(Label&& l){
      ::mutils::cstring::str_cpy(label,l.label);
      return *this;
    }
  };

  <?php 
  foreach ($types as $type){
    echo $type->value_declaration();
  } ?>
  struct Binding;
using AST_elem = Union<<?php comma_separated(names($types))?>, Binding>;
template <std::size_t budget>
using AST_Allocator = Allocator<budget, <?php echo $types[0]->name;?>, AST_elem>;

constexpr bool is_non_null(const allocated_ref<AST_elem> &e){
  return e;
}


// Define structs. 
struct Expression{constexpr Expression(){}};
struct Statement{constexpr Statement(){}};
struct Binding{
  allocated_ref<AST_elem> rhs{};
  char var[<?php echo $max_var_length ?>] = {0};
  constexpr Binding(){}
};
<?php
foreach ($types as $type){
  echo $type->struct_declaration();
}
?>
} // namespace as_values

namespace as_types {
  template<typename> struct Label;
  template<char... c> struct Label<mutils::String<c...>>{using label = mutils::String<c...>; constexpr Label() = default;};
  template<typename> struct Expression;
  template<typename> struct Statement;

  template<typename var_name,typename expr> struct Binding;
  template<char... var_name,typename expr> 
  struct Binding<mutils::String<var_name...>, Expression<expr> >{};
  
<?php foreach ($types as $type){
     echo $type->define_type();
    echo $type->encapsulate_type();
  }
?>

} // namespace as_types

namespace as_types{
    <?php
    foreach ($types as $type){
        echo $type->is_astnode_defn();
    }
    ?>
    template<typename T>
    struct is_astnode_Statement : public std::false_type{};
    template<typename T>
    struct is_astnode_Statement<Statement<T>> : public std::true_type{};
}

namespace as_values {

template <typename prev_holder>
          struct as_type_f{
static constexpr const DECT(prev_holder::prev.allocator) &allocator{prev_holder::prev.allocator};

  template<std::size_t index>
  struct arg_struct{
    static_assert(index > 0);
    constexpr arg_struct() = default;
    constexpr const AST_elem &operator()() const {
      return allocated_ref<AST_elem>{typename allocated_ref<AST_elem>::really_set_index{}, index}.get(allocator);
    }
  };

template<long budget, typename F>
constexpr static auto as_type(std::enable_if_t<(budget > 0) && (budget <= 10000)>* = nullptr) {
  static_assert(budget > 0);
  if constexpr (budget > 0) {
    constexpr const AST_elem &e = F{}();
    <?php foreach ($types as $i => $type){
      if ($i > 0) echo 'else ';
      echo "if constexpr (e.template get_<$type->name>().is_this_elem) {\n";
      echo $type->to_type_body();
    echo "}\n";
  }?>
  else if constexpr (e.template get_<Binding>().is_this_elem){
    constexpr const auto& str = e.template get_<Binding>().t.var;
    using _arg0 = DECT(mutils::String<<?php echo char_seq_from_cstring("str",$max_var_length); ?>>::trim_ends());
    /*Declaring arg!*/ struct arg1 {
#ifndef __clang__
          const AST_elem &e{F{}()};
#endif
          constexpr arg1() {}
          constexpr const AST_elem &operator()() const {
            return e.template get_<Binding>().t.rhs.get(allocator);
          }
        };

        using _arg1 = DECT(as_type<budget - 1, arg1>());
        
        return as_types::Binding<_arg0, _arg1>{};
  }
  else {struct error {};
    return error{};}
  } else {
    static_assert(budget > 0);
    struct error {};
    return error{};
  }
}
};

template <typename prev_holder> constexpr auto as_type() {
  struct arg {
    constexpr arg() {}
    constexpr const AST_elem &operator()() const {
      return prev_holder::prev.allocator.top.e.get(prev_holder::prev.allocator);
    }
  };
  return as_types::Statement<as_types::transaction<DECT(as_type_f<prev_holder>::template as_type<1000,arg>()),prev_holder::prev.allocator.top.payload>>{};
}
} // namespace as_values

namespace as_types{

template<typename> struct sequence_assigner;
	  
template<std::size_t... nums> struct sequence_assigner<std::integer_sequence<std::size_t, nums...>> {
		template<typename... T_args>
    struct helper{
      template<typename ref, typename F, typename... F_args>
      constexpr static void assign(plain_array<ref>& r, const F &f, F_args&... args){
        ((r[nums] = f(T_args{}, args...)),...);
      }
    };
};



template<typename AST_Allocator, std::size_t budget>
struct as_values_ns_fns{
  using AST_elem = as_values::AST_elem;
  constexpr as_values_ns_fns() = default;
  as_values::AST_Allocator<budget> allocator;
  /*
  template<typename T> struct converter {
    static constexpr auto value() {return as_values_ns_fns::foo();}
  };*/
  template<typename ref, typename... type_args>
  constexpr void sequence_assign(plain_array<ref>& r){
    constexpr auto function_arg = [](const auto& true_arg, auto& _this) constexpr {return _this.as_value(true_arg);};
    return sequence_assigner<std::make_index_sequence<sizeof...(type_args)>>
    ::template helper<type_args...>::template assign<ref>(r, function_arg, *this);
  }

  <?php 
  foreach ($types as $type){
    echo $type->to_value();
  }
  ?>

  template<char... str, typename t> 
  constexpr allocated_ref<AST_elem> as_value(const Binding<mutils::String<str...>, Expression<t> >&){
    auto elem = allocator.template allocate<AST_elem>();
    auto &this_node =
        elem.get(allocator).template get_<as_values::Binding>();
    this_node.is_this_elem = true;
    elem.get(allocator).is_initialized = true;
    this_node.t.rhs = as_value(Expression<t>{});
    mutils::cstring::str_cpy(this_node.t.var, mutils::String<str...>{}.string);
    return std::move(elem);
  }
};

  template<std::size_t budget, typename hd>
  constexpr as_values::AST_Allocator<budget> as_value(){
    static_assert(is_astnode_transaction<hd>::value);
    as_values_ns_fns<as_values::AST_Allocator<budget>, budget> ret;
    ret.allocator.top = std::move(ret.as_value(hd{}).get(ret.allocator).template get<as_values::transaction>());
    return std::move(ret.allocator);
  }
}

namespace as_values {
  template<typename Allocator>
  std::ostream& print(std::ostream& o, const std::size_t& st, const Allocator &){
    return o << st;
  }
  template<typename Allocator>
  std::ostream& print(std::ostream& o, const AST_elem& e, const Allocator &allocator);
  <?php 
  foreach ($types as $type){
  echo "
  template<typename Allocator>
  std::ostream& print(std::ostream& o, const $type->name& e, const Allocator& allocator){
    o << \"$type->name{\";";
    foreach ($type->fields as $field){
      echo "print(o,e.$field->name,allocator);
      o << \",\";";
    }
  echo "
    return o << \"}\";
    }";
  }
  ?>

  template<typename Allocator>
  std::ostream& print(std::ostream& o, const Binding& b, const Allocator &allocator){
    o << b.var << " = ";
    return print(o,b.rhs,allocator);
  }

  template<char... c, typename Allocator>
  std::ostream& print(std::ostream &o, const mutils::String<c...>, const Allocator&){
    return o << mutils::String<c...>{}.string;
  }

  template<typename Allocator>
  std::ostream& print(std::ostream &o, const plain_array<char> &cstr, const Allocator&){
    const char* str = cstr;
    return o << str;
  }

  template<typename Allocator>
  std::ostream& print(std::ostream& o, const AST_elem& e, const Allocator &allocator){
    <?php 
    foreach ($types as $type){
      echo "if (e.template get_<$type->name>().is_this_elem) {
          return print(o,e.template get<$type->name>(),allocator);
      }";
    }
    ?>
    if (e.template get_<Binding>().is_this_elem){
      return print(o,e.template get<Binding>(),allocator);
    }
    return o;
  }

}
namespace as_values {

    template<typename Allocator, typename size_t>
  std::ostream& pretty_print(std::ostream& o, const size_t& st, const Allocator &, std::enable_if_t<std::is_same_v<size_t,std::size_t>>* = nullptr){
    return o << st;
  }
  template<typename Allocator>
  std::ostream& pretty_print(std::ostream& o, const AST_elem& e, const Allocator &allocator);
  

  template<typename Allocator>
  std::ostream& pretty_print(std::ostream& o, const Binding& b, const Allocator &allocator){
    o << b.var << " = ";
    return pretty_print(o,b.rhs,allocator);
  }

  template<char... c, typename Allocator>
  std::ostream& pretty_print(std::ostream &o, const mutils::String<c...>, const Allocator&){
    return o << mutils::String<c...>{}.string;
  }

  template<typename Allocator>
  std::ostream& pretty_print(std::ostream &o, const char* cstr, const Allocator&){
    return o << cstr;
  }

  template<typename Allocator>
  std::ostream& pretty_print(std::ostream &o, const char cstr, const Allocator&){
    char str[2] = {cstr,0};
    return o << str;
  }

  template<typename Allocator>
  std::ostream& pretty_print(std::ostream& o, const AST_elem& e, const Allocator &allocator){
    <?php 
    foreach ($types as $type){
      echo "if (e.template get_<$type->name>().is_this_elem) {
          return pretty_print(o,e.template get<$type->name>(),allocator);
      }";
    }
    ?>
    if (e.template get_<Binding>().is_this_elem){
      return pretty_print(o,e.template get<Binding>(),allocator);
    }
    return o;
  }
}

}}}
