<?php
    require_once 'common.php';
    require_once 'ast.php';
    

    $types = array(new Statement('transaction',new Field('e',$Statement_t),new Field('payload','std::size_t','{0}')),
    new Expression('FieldReference', new Field('Struct',$Expression_t),new Field('Field',$String_t)),
    new Expression('FieldPointerReference',new Field('Struct',$Expression_t),new Field('Field',$String_t)),
    new Expression('Dereference',new Field('Struct',$Expression_t)),
    new Expression('Endorse',new Field('label',$Label_t),new Field('Hndl',$Expression_t)),
    new Expression('Ensure',new Field('label',$Label_t),new Field('Hndl',$Expression_t)),
    new Expression('IsValid',new Field('Hndl',$Expression_t)),
    new Expression('VarReference',new Field('Var',$String_t)),
    new Expression('Constant',new Field('i','std::size_t','{0}')),
    new Expression('BinOp', new Field('op','char'), new Field('L',$Expression_t),new Field('R',$Expression_t)),
    new Statement('Let',new Field('Binding',$Binding_t),new Field('Body',$Statement_t)),
    new Statement('LetRemote',new Field('Binding',$Binding_t),new Field('Body',$Statement_t)),
    new Argument_pack('operation_args_exprs','exprs'),
    new Argument_pack('operation_args_varrefs','vars'),
    new Either('Operation',new Field('name',$String_t),new Field('Hndl',$Expression_t),
                    new Field('expr_args',$Argument_pack_t), new Field('var_args',$Argument_pack_t)),
    new Statement('Assignment',new Field('Var',$Expression_t), new Field('Expr',$Expression_t)),
    new Statement('Return', new Field('Expr',$Expression_t)),
    new Statement('If', new Field('condition',$Expression_t), new Field('then',$Statement_t), new Field('els',$Statement_t)),
    new Statement('While', new Field('condition',$Expression_t), new Field('body',$Statement_t)),
    new Statement('Sequence',new Field('e',$Statement_t),new Field('next',$Statement_t)),
    new Skip('Skip')
   );
?>

<?php require "ast_skeleton.php"; ?>


