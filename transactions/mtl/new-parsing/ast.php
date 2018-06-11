<?php
require_once 'util.php';
assert_options(ASSERT_ACTIVE, 1);
assert_options(ASSERT_WARNING, 1);
assert_options(ASSERT_QUIET_EVAL, 1);

$Expression_t = 0;
$Statement_t = 1;
$Binding_t = 2;
$Argument_pack_t = 3;
$List_t = 4;
$Either_t = 5;
$String_t = 6;
$Label_t = 7;
$NODEMAX = 8;
$ASTMAX = 5;

class Field{
  public $type;
  public $name;
  public $initializer = "{}";
	public function __construct($name,$type,$initializer = "{}") {
		$this->name = $name;
		$this->type = $type;
		$this->initializer = $initializer;
	}
    
	public function value_type_name(){
		global $String_t; global $Label_t;
		if (is_ast_node($this->type)) return "allocated_ref<AST_elem>";
		else if ($this->type == $String_t) return "plain_array<char>";
		else if ($this->type == $Label_t) return "Label";
		else return $this->type;
	}
	public function declare_struct_member(){
		return $this->value_type_name()." $this->name $this->initializer;";
	}
}

class proto_field {
		public $name;
		public $type;
		#type defaults to $Expression_t, but we can't use globals here.
		public function __construct($name,$type = 0){
			$this->name = $name;
			$this->type = $type;
		}
}
class AST_node{
	public $name;
	public $type;
	public $fields;
	public function __construct($type_name, $type, $fields) {
		$this->name = $type_name;
		$this->type = $type;
		$this->fields = $fields;
	}
    
	public function encapsulator_name() : string{
		return '$tnh_error';
	}

	public function field_accessors() {
		return $this->fields;
	}
    
	public function template_defn($prefix = ''){
		global $String_t; global $Label_t;
		$type = $this;
		$out = '';
		$field_num = count($type->fields);
		if ($field_num > 0) {
			$out = $out."template <";
			foreach ($type->fields as $i => $field){
				if (is_ast_node($field->type) || $field->type === $String_t || $field->type === $Label_t){
					$out = $out."typename ";
				}
				else {$out = $out. "$field->type ";}
				$out = $out. $prefix."$field->name";
				if ($i+1 != $field_num){
					$out = $out. ",";
				}
			}
			$out = $out. "> ";
		}
		return $out;
	}
	public function encapsulated_type_name($prefix = '') : string{
		$type = $this;
		global $Argument_pack_t;
		assert ($type->type != $Argument_pack_t);
		$out = $this->encapsulator_name();
		$out = $out.'<'.($type->name);
		$field_num = count($type->fields);
		if ($field_num > 0) {
			$out = $out."<";
			foreach ($type->fields as $i => $field){
				$out = $out.$prefix."$field->name";
				if ($i+1 != $field_num){
					$out = $out.",";
				}
			}
			$out = $out.">";
		}
		return $out.">";
	}
    
	public function define_type() : string{
		return $this->template_defn() .
      "struct $this->name{};\n";
	}
      
	public function encapsulate_type() : string {
		$tmp = $this->full_template_defn('_');
		$out = "$tmp struct ".$this->encapsulated_type_name('_')."{";
		foreach ($this->fields as $field){
			global $String_t; global $Label_t;
			if (is_ast_node($field->type) || $field->type === $String_t || $field->type === $Label_t){
				$out = $out."using $field->name = _$field->name;";
			}
			else {
				$out = $out."$field->type $field->name{_$field->name};";
			}
		}
		$out = $out."};\n";
		return $out;
	}
        
	public function value_declaration(){
		return "struct $this->name;";
	}
		
	public function full_template_defn_for_method($prefix = '') : string{
		return $this->full_template_defn($prefix);
	}
	public function full_template_defn($prefix = ''){
		return $this->template_defn($prefix);
	}
        
	public function is_astnode_defn(){
		$type = $this;
		return "template<typename> struct is_astnode_$type->name : public std::false_type{};".
			$type->full_template_defn()." struct is_astnode_$type->name<".$type->encapsulated_type_name()."> : public std::true_type{};";
	}
            
	public function encapsulator_names(){
		return array($this->encapsulator_name());
	}
            
	public function struct_constructor() : string{
		return "
              //default constructor
              constexpr $this->name(){}; ";
	}
	public function struct_fieldlist() {
		return $this->fields;
	}
	public function struct_move_constructor() : string{
		//move constructor
		global $String_t;
		$out = "constexpr $this->name($this->name &&p) ";
		$has_string_field = false;
		foreach ($this->struct_fieldlist() as $i => $field){
			if ($i === 0 && ($field->type != $String_t || count($this->struct_fieldlist()) > 1)) $out = $out.':';
			if ($field->type === $String_t){
				$string_field = $field;
				$has_string_field = true;
			}
			else { $out = $out."$field->name{std::move(p.$field->name)}"; 
				$out = $out.",";
			}
			
		}
		$out = substr($out,0,-1);
		$extra = '';
		if ($has_string_field){
			$extra = "mutils::cstring::str_cpy($string_field->name, p.$string_field->name);";
		}
		return $out.'{'.$extra.'}';
	}
	public function struct_move_assignment() : string{
		global $String_t;
		$out =  "
                  //move-assignment 
                  constexpr $this->name &operator=($this->name &&p) {"; 
		foreach ($this->struct_fieldlist() as $field){
			if ($field->type === $String_t){
				$out =  $out."mutils::cstring::str_cpy($field->name, p.$field->name);";
			}
			else {
				$out =  $out. "$field->name = std::move(p.$field->name);";
			}
		}
		$out =  $out. "return *this; 
                  } ";
		return $out;
  }

  public function struct_fields() : string {
	  $out = '';
		foreach ($this->struct_fieldlist() as $field){
			$out = $out.$field->declare_struct_member();
		}
		return $out;
  }
  	                
	public function struct_declaration() : string{
		$type = $this;
		$out = "struct $type->name";
		foreach ($this->encapsulator_names() as $i => $name){
      if ($i == 0) $out = "$out : ";
      if ($i > 0) $out = "$out, ";
			$out = "$out public $name";
		}
		$out = $out."{ ".$this->struct_fields();
		
		$out = $out.
			$this->struct_constructor().
			$this->struct_move_constructor().
			$this->struct_move_assignment();
		return $out."};";
	}
	public function from_e(){
		return "e.template get_<$this->name>().t";
	}
	public function get_field_from_e($field){
		return $this->from_e().".$field->name";
	}
	public function declare_arg($field, $i){
		$type = $this;
		return "using arg$i = arg_struct<".$this->get_field_from_e($field).".get_index()>;";
	}
	public function to_type_body() : string{
		global $max_var_length;
		$type = $this; 
		$ret = '';
		foreach ($type->field_accessors() as $i => $field){
			global $String_t; global $Label_t;
			if (is_ast_node($field->type)) {
				$ret = $ret.'/*Declaring arg!*/'.$this->declare_arg($field,$i).
				 "using _arg$i = DECT(as_type<budget - 1, arg$i>());\n";
			}
			elseif ($field->type === $String_t){
				$ret = $ret."
				constexpr auto& __str$i = e.template get_<$type->name>().t.$field->name;
				using _arg$i = DECT(mutils::String<".char_seq_from_cstring("__str$i",$max_var_length).">::trim_ends());";
			}
			elseif ($field->type === $Label_t){
				$ret = $ret."
				constexpr auto& __lbl$i = e.template get_<$type->name>().t.$field->name;
				using _arg$i = as_types::Label<DECT(mutils::String<".char_seq_from_cstring("__lbl$i.label",$max_var_length).">::trim_ends())>;";
			}
			else $ret = $ret. "constexpr auto _arg$i = e.template get_<$type->name>().t.$field->name;\n";
		}
    $ret = $ret. "return as_types::".$type->encapsulator_name()."<as_types::$type->name";
    $field_count = count($type->field_accessors());
    if ($field_count > 0) $ret = $ret. "<";
    foreach ($type->field_accessors() as $i => $field){
      $ret = $ret. "_arg$i";
      if ($i + 1 != $field_count){
        $ret = $ret. ",";
      }
    }
    if ($field_count > 0) $ret = $ret. ">";
    $ret = $ret. '>';
	$ret = $ret. "{};";
	return $ret;
	}

	public function to_value() :string {
		global $String_t; global $Label_t;
		$type = $this;
		$ret = ''.
		$type->full_template_defn_for_method();
		$decl = $type->encapsulated_type_name();
	  $ret = $ret. "constexpr allocated_ref<AST_elem> as_value(const $decl &){
		auto elem = allocator.template allocate<AST_elem>();
		auto &this_node = elem.get(allocator).template get_<as_values::$type->name>();
		this_node.is_this_elem = true;
		elem.get(allocator).is_initialized = true;";
		foreach ($type->fields as $field){
		  if (is_ast_node($field->type))
			$ret = $ret. "this_node.t.$field->name = as_value($field->name{});";
		elseif($field->type === $String_t){
			$ret = $ret. "mutils::cstring::str_cpy(this_node.t.$field->name, $field->name{}.string);";
		}
		elseif($field->type === $Label_t){
			$ret = $ret. "
			using ____label = typename $field->name::label;
			mutils::cstring::str_cpy(this_node.t.$field->name.label, ____label{}.string);";
		}
		  else $ret = $ret. "this_node.t.$field->name = $field->name;";
		}
		$ret = $ret. $this->to_value_hook().'return std::move(elem);';
	  $ret = $ret. "}\n";
	  return $ret;
	}
	function to_value_hook() : string {return '';}
}

function cpp_list($type) : string{
	return "plain_array<$type>";
}
                
class Statement extends AST_node {
	public function __construct($type_name, ...$fields) {
		global $Statement_t;
		parent::__construct($type_name, $Statement_t, $fields);
	}
	public function encapsulator_name() : string{
		return 'Statement';
	}
}
class Binding extends AST_node {
	public function __construct($type_name, ...$fields) {
		global $Binding_t;
		parent::__construct($type_name, $Binding_t, $fields);
	}
	public function encapsulator_name() : string{
		return 'Binding';
	}
}
class Expression extends AST_node {
	public function __construct($type_name, ...$fields) {
		global $Expression_t;
		parent::__construct($type_name, $Expression_t, $fields);
	}
	public function encapsulator_name() : string{
		return 'Expression';
	}
}
class Either extends AST_node {
	private $current_identity = '$error_identity_unset';
	private function reset_identity(){
		$this->current_identity = '$error_identity_unset';
	}
	private function set_identity(string $str){
		$this->current_identity = $str;
	}
	public function __construct($type_name, ...$fields) {
		global $Either_t;
		//array_push($fields,new Field("is_statement","bool"));
		parent::__construct($type_name, $Either_t, $fields);
	}
	public function encapsulator_name() : string{
		return $this->current_identity;
	}
			
	public function encapsulator_names(){
		return array('Statement','Expression');
	}

	private $to_value_hook_val = '';
	public function to_value_hook() : string {
		return $this->to_value_hook_val;
	}

	public function struct_fieldlist(){
		$ret = parent::struct_fieldlist();
		array_push($ret,new Field("is_statement","bool"));
		return $ret;
	}

	public function is_astnode_defn(){
		$type = $this;
		$ret = "template<typename> struct is_astnode_$type->name : public std::false_type{};";
		foreach ($this->encapsulator_names() as $name){
			$this->set_identity($name);
			$ret = $ret.$type->full_template_defn()." struct is_astnode_$type->name<".$type->encapsulated_type_name()."> : public std::true_type{};";
		}
		$this->reset_identity();
		return $ret;
	}
	public function to_value() : string {
		$ret = '';
		foreach ($this->encapsulator_names() as $name){
			$this->set_identity($name);
			$this->to_value_hook_val = "this_node.t.is_statement = ".($name === "Statement" ? "true" : "false").";";
			$ret = $ret.parent::to_value();
		}
		$this->reset_identity();
		return $ret;
	}
                  
	public function encapsulate_type() : string{
		$out = '';
		foreach ($this->encapsulator_names() as $name){
			$this->set_identity($name);
			$out = $out.parent::encapsulate_type();
		}
		$this->reset_identity();
		return $out;
	}
	public function to_type_body() : string {
		$ret = "using is_statement = std::integral_constant<bool,".$this->from_e().".is_statement>;".
		"if constexpr (is_statement::value) {";
		$this->set_identity("Statement");
		$ret = $ret.parent::to_type_body(). "}".
		"else {";
		$this->set_identity("Expression");
		$ret = $ret.parent::to_type_body(). "}".
		$this->reset_identity();
		return $ret;
	}
}
class Argument_pack extends AST_node {
	public $field_name;
	public function __construct($type_name,$field_name) {
		global $Expression_t; global $Argument_pack_t; global $List_t;
		parent::__construct($type_name, $Argument_pack_t, array(new Field($field_name,$List_t)));
		$this->field_name = $field_name;
	}
	public function encapsulated_type_name($prefix = '') : string{
		assert(false); //we don't encapsulate Argument_packs.
		echo 'assert_fail_argument_pack';
		return $this->name;
	}
	public function struct_fields() : string {
		return cpp_list('allocated_ref<AST_elem>')." $this->field_name;";
	}

	public function encapsulate_type() : string {
	return "template<typename...> struct $this->name {};";
	}

	public function encapsulator_name() : string{
		//this type is not encapsulated.
		return '';
  }
  
  public function encapsulator_names() {
    return array();
  }
  
	public function define_type() : string {
		return "template<typename...> struct $this->name;";
	}
	public function struct_move_constructor() : string{
		
		global $max_var_length;
		return "
		//move constructor
		constexpr $this->name($this->name &&p) : $this->field_name{{allocated_ref<AST_elem>{}}}{
			for (auto i = 0u; i < $max_var_length; ++i){
				$this->field_name[i] = std::move(p.$this->field_name[i]);
			}
		}
		";
	}
	public function struct_move_assignment() : string{
		global $max_var_length;
		return "
        //move-assignment 
        constexpr $this->name &operator=($this->name &&p) {
			for (auto i = 0u; i < $max_var_length; ++i){
				$this->field_name[i] = std::move(p.$this->field_name[i]);
			}
			return *this;
		}
		";
  }

  public function field_accessors() {
	  global $max_var_length;
	  global $Expression_t;
	  $ret = array();
	  for ($i =0; $i < $max_var_length; ++$i){
			array_push($ret,new proto_field("$this->field_name[$i]"));
	  }
	  return $ret;
  }

  public function assemble_to_type_return($max_var_length) : string {
	$type = $this;
	$ret = "return ";
	if ($type->encapsulator_name() === ''){
		$ret = $ret.'';
	}
	else {
		$ret = $ret. "as_types::".$type->encapsulator_name()."<";
		
	}
	$ret = $ret."as_types::$type->name<";
	
	if ($max_var_length > 0){
	    $field_count = $max_var_length;
    	for ($j = 0; $j < $max_var_length; ++$j){
      	$ret = $ret. "_arg$j";
      	if ($j + 1 != $field_count){
	        $ret = $ret. ",";
      		}
    	}
	}
	if ($type->encapsulator_name() === ''){}
		else {$ret = $ret.">";}
	return $ret.">{};";
  }

  public function to_type_body() : string{
	  global $max_var_length;
	  $ret = '';
		#is always an AST node, apparently
			for ($i = 0; $i < $max_var_length; ++$i){
				$proto_f = new proto_field("$this->field_name[$i]");
				$ret = $ret.
				"{ constexpr std::size_t __indx = ".$this->get_field_from_e($proto_f).".get_index();
				if constexpr (__indx > 0){
				using arg$i = arg_struct<__indx>;
				using _arg$i = DECT(as_type<budget - 1, arg$i>());";
				if ($i + 1 == $max_var_length) {
					$ret = $ret.$this->assemble_to_type_return($max_var_length);
				}
			}
			for ($i = 0; $i < $max_var_length - 1; ++$i){
			$ret = $ret."} else {".
				$this->assemble_to_type_return($max_var_length - $i - 1).
				"}}";
			}
		return $ret."} else {".$this->assemble_to_type_return(0)."}}";
	}
	
	public function is_astnode_defn() : string {
		return '';
	}

	public function to_value() :string {
		global $String_t; global $Label_t;
		$type = $this;
		return "

	  template<typename... Args>
	  constexpr allocated_ref<AST_elem> as_value(const $this->name<Args...> &){
		auto elem = allocator.template allocate<AST_elem>();
		auto &this_node = elem.get(allocator).template get_<as_values::$type->name>();
		this_node.is_this_elem = true;
		elem.get(allocator).is_initialized = true;
		sequence_assign<allocated_ref<AST_elem>,Args...>(this_node.t.$this->field_name);
		return std::move(elem);
	  }";
	}
}
                
class Skip extends AST_node {
	public function __construct($type_name) {
		global $Statement_t;
		parent::__construct($type_name, $Statement_t, array());
	}
                  
	public function encapsulator_name() : string{
		return 'Statement';
	}
                  
	public function template_defn($prefix = ''){
		return '';
	}
                  
	public function full_template_defn($prefix = ''){
		return 'template<>';
	}
	public function full_template_defn_for_method($prefix = '') : string {
		return '';
	}
}
                
function is_ast_node($node_t){
	global $Statement_t; global $Expression_t; global $Binding_t;
	global $String_t; global $Label_t; global $Argument_pack_t;
	global $List_t; global $ASTMAX;
	if (is_int($node_t)){
		return $node_t >= $Expression_t && $node_t <= $ASTMAX;
	}
}
?>