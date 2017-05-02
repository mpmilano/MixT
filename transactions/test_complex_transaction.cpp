#include "transaction.hpp"
#include "transaction_macros.hpp"
#include "split_printer.hpp"
#include <iostream>
using namespace myria;
using namespace mtl;
using namespace tracker;

int main(){
  using test_struct = STRUCT(int, a, int, b, int, c);
  test_struct tstruct2;
  test_struct tstruct;
  tstruct.a = 0;
  tstruct.b = 0;
  tstruct.c = 0;
  tstruct2.a = 0;
  tstruct2.b = 0;
  tstruct2.c = 0;
  Handle<myria::Label<myria::bottom>,int> hndl;
  Handle<myria::Label<myria::bottom>,test_struct> hndl2;
	/*
  constexpr auto txn = TRANSACTION(var x = 3,
			  var y = 5,
			  x = 7,
			  y = y + x,
			  var condition = 0,
			  while (condition < x) {
			    y = y + 3,
			    condition = tstruct2.a + 1,
			    tstruct2.a = condition,
			    remote z = hndl,
			    z = condition,
			    var unused = *hndl,
			    unused = hndl2->a - 1,
			    z = unused,
			    var a = z,
			    z = a
			  },
			  if (45 > y) {
			    y = tstruct.a,
			    tstruct.b = x
			  } else {if (hndl.isValid()){return 7} else {}}
				   )::WITH(tstruct,tstruct2,hndl,hndl2);
	txn.just_print_it();//*/
constexpr
myria::mtl::transaction_struct<0,
      myria::mtl::previous_transaction_phases<myria::mtl::typecheck_phase::Statement<myria::Label<top>,
      myria::mtl::typecheck_phase::Let<myria::mtl::typecheck_phase::Binding<myria::Label<top>, int,
      mutils::String<'z', 'e', 'r', 'o', '\x00', '\x01'>, 
myria::mtl::typecheck_phase::Expression<myria::Label<top>,
      int, myria::mtl::typecheck_phase::Constant<0> > >, 
myria::mtl::typecheck_phase::Statement<myria::Label<top>,
      myria::mtl::typecheck_phase::Let<myria::mtl::typecheck_phase::Binding<myria::Label<top>, int,
      mutils::String<'o', 'n', 'e', '\x00', '\x01'>, 
myria::mtl::typecheck_phase::Expression<myria::Label<top>, int,
      myria::mtl::typecheck_phase::Constant<1> > >, myria::mtl::typecheck_phase::Statement<myria::Label<top>,
      myria::mtl::typecheck_phase::Let<myria::mtl::typecheck_phase::Binding<myria::Label<top>, bool,
      mutils::String<'t', 'r', 'u', 'e'>, myria::mtl::typecheck_phase::Expression<myria::Label<top>, bool,
      myria::mtl::typecheck_phase::BinOp<'=', myria::mtl::typecheck_phase::Expression<myria::Label<top>, int,
      myria::mtl::typecheck_phase::VarReference<mutils::String<'z', 'e', 'r', 'o', '\x00', '\x01'> > >,
      myria::mtl::typecheck_phase::Expression<myria::Label<top>, int,
      myria::mtl::typecheck_phase::VarReference<mutils::String<'z', 'e', 'r', 'o', '\x00', '\x01'> > > > > >,
      myria::mtl::typecheck_phase::Statement<myria::Label<top>,
      myria::mtl::typecheck_phase::Let<myria::mtl::typecheck_phase::Binding<myria::Label<top>, bool,
      mutils::String<'f', 'a', 'l', 's', 'e'>, myria::mtl::typecheck_phase::Expression<myria::Label<top>, 
bool,
      myria::mtl::typecheck_phase::BinOp<'=', myria::mtl::typecheck_phase::Expression<myria::Label<top>, int,
      myria::mtl::typecheck_phase::VarReference<mutils::String<'z', 'e', 'r', 'o', '\x00', '\x01'> > >,
      myria::mtl::typecheck_phase::Expression<myria::Label<top>, int,
      myria::mtl::typecheck_phase::VarReference<mutils::String<'o', 'n', 'e', '\x00', '\x01'> > > > > >,
      myria::mtl::typecheck_phase::Statement<myria::Label<top>,
      myria::mtl::typecheck_phase::Let<myria::mtl::typecheck_phase::Binding<myria::Label<top>, int,
      mutils::String<'x'>, myria::mtl::typecheck_phase::Expression<myria::Label<top>, int,
      myria::mtl::typecheck_phase::Constant<3> > >, myria::mtl::typecheck_phase::Statement<myria::Label<top>,
      myria::mtl::typecheck_phase::Let<myria::mtl::typecheck_phase::Binding<myria::Label<top>, int,
      mutils::String<'y'>, myria::mtl::typecheck_phase::Expression<myria::Label<top>, int,
      myria::mtl::typecheck_phase::Constant<5> > >, myria::mtl::typecheck_phase::Statement<myria::Label<top>,
      myria::mtl::typecheck_phase::Sequence<myria::mtl::typecheck_phase::Statement<myria::Label<top>,
      myria::mtl::typecheck_phase::Let<myria::mtl::typecheck_phase::Binding<myria::Label<top>, int,
      mutils::String<'a', 'n', 'o', 'r', 'm', '\x01', '\x03'>,
      myria::mtl::typecheck_phase::Expression<myria::Label<top>, int, myria::mtl::typecheck_phase::Constant<7> 
> >,
      myria::mtl::typecheck_phase::Statement<myria::Label<top>,
      myria::mtl::typecheck_phase::Assignment<myria::mtl::typecheck_phase::Expression<myria::Label<top>, int,
      myria::mtl::typecheck_phase::VarReference<mutils::String<'x'> > >,
      myria::mtl::typecheck_phase::Expression<myria::Label<top>, int,
      myria::mtl::typecheck_phase::VarReference<mutils::String<'a', 'n', 'o', 'r', 'm', '\x01', '\x03'> > > > 
> > >,
      myria::mtl::typecheck_phase::Statement<myria::Label<top>,
      myria::mtl::typecheck_phase::Let<myria::mtl::typecheck_phase::Binding<myria::Label<top>, int,
      mutils::String<'a', 'n', 'o', 'r', 'm', '\x02', '\x03'>,
      myria::mtl::typecheck_phase::Expression<myria::Label<top>, int, myria::mtl::typecheck_phase::BinOp<'+',
      myria::mtl::typecheck_phase::Expression<myria::Label<top>, int,
      myria::mtl::typecheck_phase::VarReference<mutils::String<'y'> > >,
      myria::mtl::typecheck_phase::Expression<myria::Label<top>, int,
      myria::mtl::typecheck_phase::VarReference<mutils::String<'x'> > > > > >,
      myria::mtl::typecheck_phase::Statement<myria::Label<top>,
      myria::mtl::typecheck_phase::Assignment<myria::mtl::typecheck_phase::Expression<myria::Label<top>, int,
      myria::mtl::typecheck_phase::VarReference<mutils::String<'y'> > >,
      myria::mtl::typecheck_phase::Expression<myria::Label<top>, int,
      myria::mtl::typecheck_phase::VarReference<mutils::String<'a', 'n', 'o', 'r', 'm', '\x02', '\x03'> > > > 
> > >,
      myria::mtl::typecheck_phase::Statement<myria::Label<top>,
      myria::mtl::typecheck_phase::Let<myria::mtl::typecheck_phase::Binding<myria::Label<top>, int,
      mutils::String<'c', 'o', 'n', 'd', 'i', 't', 'i', 'o', 'n'>,
      myria::mtl::typecheck_phase::Expression<myria::Label<top>, int, myria::mtl::typecheck_phase::Constant<0> 
> >,
      myria::mtl::typecheck_phase::Statement<myria::Label<top>,
      myria::mtl::typecheck_phase::Sequence<myria::mtl::typecheck_phase::Statement<myria::Label<top>,
      myria::mtl::typecheck_phase::Let<myria::mtl::typecheck_phase::Binding<myria::Label<top>, bool,
      mutils::String<'w', 'h', 'i', 'l', 'e', '\x03', '\x04'>,
      myria::mtl::typecheck_phase::Expression<myria::Label<top>, bool,
      myria::mtl::typecheck_phase::VarReference<mutils::String<'t', 'r', 'u', 'e'> > > >,
      myria::mtl::typecheck_phase::Statement<myria::Label<top>,
      myria::mtl::typecheck_phase::While<myria::mtl::typecheck_phase::Expression<myria::Label<top>, bool,
      myria::mtl::typecheck_phase::VarReference<mutils::String<'w', 'h', 'i', 'l', 'e', '\x03', '\x04'> > >,
      myria::mtl::typecheck_phase::Statement<myria::Label<top>,
      myria::mtl::typecheck_phase::Let<myria::mtl::typecheck_phase::Binding<myria::Label<top>, bool,
      mutils::String<'a', 'n', 'o', 'r', 'm', '\x03', '\x06'>,
      myria::mtl::typecheck_phase::Expression<myria::Label<top>, bool, myria::mtl::typecheck_phase::BinOp<'<',
      myria::mtl::typecheck_phase::Expression<myria::Label<top>, int,
      myria::mtl::typecheck_phase::VarReference<mutils::String<'c', 'o', 'n', 'd', 'i', 't', 'i', 'o', 'n'> > 
>,
      myria::mtl::typecheck_phase::Expression<myria::Label<top>, int,
      myria::mtl::typecheck_phase::VarReference<mutils::String<'x'> > > > > >,
      myria::mtl::typecheck_phase::Statement<myria::Label<top>,
      myria::mtl::typecheck_phase::If<myria::mtl::typecheck_phase::Expression<myria::Label<top>, bool,
      myria::mtl::typecheck_phase::VarReference<mutils::String<'a', 'n', 'o', 'r', 'm', '\x03', '\x06'> > >,
      myria::mtl::typecheck_phase::Statement<myria::Label<top>,
      myria::mtl::typecheck_phase::Sequence<myria::mtl::typecheck_phase::Statement<myria::Label<top>,
      myria::mtl::typecheck_phase::Let<myria::mtl::typecheck_phase::Binding<myria::Label<top>, int,
      mutils::String<'a', 'n', 'o', 'r', 'm', '\x03', '\t'>,
      myria::mtl::typecheck_phase::Expression<myria::Label<top>, int, myria::mtl::typecheck_phase::Constant<3> 
> >,
      myria::mtl::typecheck_phase::Statement<myria::Label<top>,
      myria::mtl::typecheck_phase::Let<myria::mtl::typecheck_phase::Binding<myria::Label<top>, int,
      mutils::String<'a', 'n', 'o', 'r', 'm', '\x03', '\x0B'>,
      myria::mtl::typecheck_phase::Expression<myria::Label<top>, int, myria::mtl::typecheck_phase::BinOp<'+',
      myria::mtl::typecheck_phase::Expression<myria::Label<top>, int,
      myria::mtl::typecheck_phase::VarReference<mutils::String<'y'> > >,
      myria::mtl::typecheck_phase::Expression<myria::Label<top>, int,
      myria::mtl::typecheck_phase::VarReference<mutils::String<'a', 'n', 'o', 'r', 'm', '\x03', '\t'> > > > > 
>,
      myria::mtl::typecheck_phase::Statement<myria::Label<top>,
      myria::mtl::typecheck_phase::Assignment<myria::mtl::typecheck_phase::Expression<myria::Label<top>, int,
      myria::mtl::typecheck_phase::VarReference<mutils::String<'y'> > >,
      myria::mtl::typecheck_phase::Expression<myria::Label<top>, int,
      myria::mtl::typecheck_phase::VarReference<mutils::String<'a', 'n', 'o', 'r', 'm', '\x03', '\x0B'> > > > 
> > > >
      >, myria::mtl::typecheck_phase::Statement<myria::Label<top>,
      myria::mtl::typecheck_phase::Let<myria::mtl::typecheck_phase::Binding<myria::Label<top>, int,
      mutils::String<'a', 'n', 'o', 'r', 'm', '\x04', '\t'>,
      myria::mtl::typecheck_phase::Expression<myria::Label<top>, int,
      myria::mtl::typecheck_phase::FieldReference<myria::mtl::typecheck_phase::Expression<myria::Label<top>,
      test_struct, myria::mtl::typecheck_phase::VarReference<mutils::String<'t', 's', 't', 'r', 'u', 'c', 't', 
'2'> >
      >, mutils::String<'a'> > > >, myria::mtl::typecheck_phase::Statement<myria::Label<top>,
      myria::mtl::typecheck_phase::Let<myria::mtl::typecheck_phase::Binding<myria::Label<top>, int,
      mutils::String<'a', 'n', 'o', 'r', 'm', '\x04', '\x0B'>,
      myria::mtl::typecheck_phase::Expression<myria::Label<top>, int, myria::mtl::typecheck_phase::Constant<1> 
> >,
      myria::mtl::typecheck_phase::Statement<myria::Label<top>,
      myria::mtl::typecheck_phase::Let<myria::mtl::typecheck_phase::Binding<myria::Label<top>, int,
      mutils::String<'a', 'n', 'o', 'r', 'm', '\x04', '\x0D'>,
      myria::mtl::typecheck_phase::Expression<myria::Label<top>, int, myria::mtl::typecheck_phase::BinOp<'+',
      myria::mtl::typecheck_phase::Expression<myria::Label<top>, int,
      myria::mtl::typecheck_phase::VarReference<mutils::String<'a', 'n', 'o', 'r', 'm', '\x04', '\t'> > >,
      myria::mtl::typecheck_phase::Expression<myria::Label<top>, int,
      myria::mtl::typecheck_phase::VarReference<mutils::String<'a', 'n', 'o', 'r', 'm', '\x04', '\x0B'> > > > 
> >,
      myria::mtl::typecheck_phase::Statement<myria::Label<top>,
      myria::mtl::typecheck_phase::Assignment<myria::mtl::typecheck_phase::Expression<myria::Label<top>, int,
      myria::mtl::typecheck_phase::VarReference<mutils::String<'c', 'o', 'n', 'd', 'i', 't', 'i', 'o', 'n'> > 
>,
      myria::mtl::typecheck_phase::Expression<myria::Label<top>, int,
      myria::mtl::typecheck_phase::VarReference<mutils::String<'a', 'n', 'o', 'r', 'm', '\x04', '\x0D'> > > > 
> > > >
      > > >, myria::mtl::typecheck_phase::Statement<myria::Label<top>,
      myria::mtl::typecheck_phase::Assignment<myria::mtl::typecheck_phase::Expression<myria::Label<top>, int,
      myria::mtl::typecheck_phase::FieldReference<myria::mtl::typecheck_phase::Expression<myria::Label<top>,
      test_struct, myria::mtl::typecheck_phase::VarReference<mutils::String<'t', 's', 't', 'r', 'u', 'c', 't', 
'2'> >
      >, mutils::String<'a'> > >, myria::mtl::typecheck_phase::Expression<myria::Label<top>, int,
      myria::mtl::typecheck_phase::VarReference<mutils::String<'c', 'o', 'n', 'd', 'i', 't', 'i', 'o', 'n'> > 
> > >,
      myria::mtl::typecheck_phase::Statement<myria::Label<bottom>,
      myria::mtl::typecheck_phase::LetRemote<myria::mtl::typecheck_phase::Binding<myria::Label<bottom>, int,
      mutils::String<'z'>, myria::mtl::typecheck_phase::Expression<myria::Label<top>,
      myria::Handle<myria::Label<bottom>, int>, myria::mtl::typecheck_phase::VarReference<mutils::String<'h', 
'n',
      'd', 'l'> > > >, myria::mtl::typecheck_phase::Statement<myria::Label<top>,
      myria::mtl::typecheck_phase::Sequence<myria::mtl::typecheck_phase::Statement<myria::Label<bottom>,
      myria::mtl::typecheck_phase::Assignment<myria::mtl::typecheck_phase::Expression<myria::Label<bottom>, 
int,
      myria::mtl::typecheck_phase::VarReference<mutils::String<'z'> > >,
      myria::mtl::typecheck_phase::Expression<myria::Label<top>, int,
      myria::mtl::typecheck_phase::VarReference<mutils::String<'c', 'o', 'n', 'd', 'i', 't', 'i', 'o', 'n'> > 
> > >,
      myria::mtl::typecheck_phase::Statement<myria::Label<bottom>,
      myria::mtl::typecheck_phase::LetRemote<myria::mtl::typecheck_phase::Binding<myria::Label<bottom>, int,
      mutils::String<'r', 'e', 'm', 'o', 't', 'e', '_', 'b', 'o', 'u', 'n', 'd', '_', 't', 'm', 'p', '\x00', 
'\x07',
      '\n'>, myria::mtl::typecheck_phase::Expression<myria::Label<top>, myria::Handle<myria::Label<bottom>, 
int>,
      myria::mtl::typecheck_phase::VarReference<mutils::String<'h', 'n', 'd', 'l'> > > >,
      myria::mtl::typecheck_phase::Statement<myria::Label<bottom>,
      myria::mtl::typecheck_phase::Let<myria::mtl::typecheck_phase::Binding<myria::Label<bottom>, int,
      mutils::String<'u', 'n', 'u', 's', 'e', 'd'>, 
myria::mtl::typecheck_phase::Expression<myria::Label<bottom>, int,
      myria::mtl::typecheck_phase::VarReference<mutils::String<'r', 'e', 'm', 'o', 't', 'e', '_', 'b', 'o', 
'u', 'n',
      'd', '_', 't', 'm', 'p', '\x00', '\x07', '\n'> > > >, 
myria::mtl::typecheck_phase::Statement<myria::Label<top>,
      myria::mtl::typecheck_phase::Sequence<myria::mtl::typecheck_phase::Statement<myria::Label<bottom>,
      myria::mtl::typecheck_phase::LetRemote<myria::mtl::typecheck_phase::Binding<myria::Label<bottom>, 
test_struct,
      mutils::String<'r', 'e', 'm', 'o', 't', 'e', '_', 'b', 'o', 'u', 'n', 'd', '_', 't', 'm', 'p', '\x00', 
'\x07',
      '\x0D'>, myria::mtl::typecheck_phase::Expression<myria::Label<top>, myria::Handle<myria::Label<bottom>,
      test_struct>, myria::mtl::typecheck_phase::VarReference<mutils::String<'h', 'n', 'd', 'l', '2'> > > >,
      myria::mtl::typecheck_phase::Statement<myria::Label<bottom>,
      myria::mtl::typecheck_phase::Let<myria::mtl::typecheck_phase::Binding<myria::Label<bottom>, int,
      mutils::String<'a', 'n', 'o', 'r', 'm', '\x07', '\x0F'>,
      myria::mtl::typecheck_phase::Expression<myria::Label<bottom>, int,
      
myria::mtl::typecheck_phase::FieldReference<myria::mtl::typecheck_phase::Expression<myria::Label<bottom>,
      test_struct, myria::mtl::typecheck_phase::VarReference<mutils::String<'r', 'e', 'm', 'o', 't', 'e', '_', 
'b',
      'o', 'u', 'n', 'd', '_', 't', 'm', 'p', '\x00', '\x07', '\x0D'> > >, mutils::String<'a'> > > >,
      myria::mtl::typecheck_phase::Statement<myria::Label<top>,
      myria::mtl::typecheck_phase::Let<myria::mtl::typecheck_phase::Binding<myria::Label<bottom>, int,
      mutils::String<'a', 'n', 'o', 'r', 'm', '\x07', '\x11'>,
      myria::mtl::typecheck_phase::Expression<myria::Label<top>, int, myria::mtl::typecheck_phase::Constant<1> 
> >,
      myria::mtl::typecheck_phase::Statement<myria::Label<bottom>,
      myria::mtl::typecheck_phase::Let<myria::mtl::typecheck_phase::Binding<myria::Label<bottom>, int,
      mutils::String<'a', 'n', 'o', 'r', 'm', '\x07', '\x13'>,
      myria::mtl::typecheck_phase::Expression<myria::Label<bottom>, int, 
myria::mtl::typecheck_phase::BinOp<'-',
      myria::mtl::typecheck_phase::Expression<myria::Label<bottom>, int,
      myria::mtl::typecheck_phase::VarReference<mutils::String<'a', 'n', 'o', 'r', 'm', '\x07', '\x0F'> > >,
      myria::mtl::typecheck_phase::Expression<myria::Label<bottom>, int,
      myria::mtl::typecheck_phase::VarReference<mutils::String<'a', 'n', 'o', 'r', 'm', '\x07', '\x11'> > > > 
> >,
      myria::mtl::typecheck_phase::Statement<myria::Label<bottom>,
      myria::mtl::typecheck_phase::Assignment<myria::mtl::typecheck_phase::Expression<myria::Label<bottom>, 
int,
      myria::mtl::typecheck_phase::VarReference<mutils::String<'u', 'n', 'u', 's', 'e', 'd'> > >,
      myria::mtl::typecheck_phase::Expression<myria::Label<bottom>, int,
      myria::mtl::typecheck_phase::VarReference<mutils::String<'a', 'n', 'o', 'r', 'm', '\x07', '\x13'> > > > 
> > > >
      > > > > >, myria::mtl::typecheck_phase::Statement<myria::Label<bottom>,
      myria::mtl::typecheck_phase::Assignment<myria::mtl::typecheck_phase::Expression<myria::Label<bottom>, 
int,
      myria::mtl::typecheck_phase::VarReference<mutils::String<'z'> > >,
      myria::mtl::typecheck_phase::Expression<myria::Label<bottom>, int,
      myria::mtl::typecheck_phase::VarReference<mutils::String<'u', 'n', 'u', 's', 'e', 'd'> > > > >,
      myria::mtl::typecheck_phase::Statement<myria::Label<bottom>,
      myria::mtl::typecheck_phase::Let<myria::mtl::typecheck_phase::Binding<myria::Label<bottom>, int,
      mutils::String<'a'>, myria::mtl::typecheck_phase::Expression<myria::Label<bottom>, int,
      myria::mtl::typecheck_phase::VarReference<mutils::String<'z'> > > >,
      myria::mtl::typecheck_phase::Statement<myria::Label<top>,
      myria::mtl::typecheck_phase::Sequence<myria::mtl::typecheck_phase::Statement<myria::Label<bottom>,
      myria::mtl::typecheck_phase::Assignment<myria::mtl::typecheck_phase::Expression<myria::Label<bottom>, 
int,
      myria::mtl::typecheck_phase::VarReference<mutils::String<'z'> > >,
      myria::mtl::typecheck_phase::Expression<myria::Label<bottom>, int,
      myria::mtl::typecheck_phase::VarReference<mutils::String<'a'> > > > > > > > > > > > > > > > > > > > >,
      myria::mtl::typecheck_phase::Statement<myria::Label<top>,
      myria::mtl::typecheck_phase::Assignment<myria::mtl::typecheck_phase::Expression<myria::Label<top>, bool,
      myria::mtl::typecheck_phase::VarReference<mutils::String<'w', 'h', 'i', 'l', 'e', '\x03', '\x04'> > >,
      myria::mtl::typecheck_phase::Expression<myria::Label<top>, bool,
      myria::mtl::typecheck_phase::VarReference<mutils::String<'f', 'a', 'l', 's', 'e'> > > > > > > > >> > > 
>,
      myria::mtl::typecheck_phase::Statement<myria::Label<top>,
      myria::mtl::typecheck_phase::Let<myria::mtl::typecheck_phase::Binding<myria::Label<top>, int,
      mutils::String<'a', 'n', 'o', 'r', 'm', '\x04', '\x04'>,
      myria::mtl::typecheck_phase::Expression<myria::Label<top>, int, 
myria::mtl::typecheck_phase::Constant<45> > >,
      myria::mtl::typecheck_phase::Statement<myria::Label<top>,
      myria::mtl::typecheck_phase::Let<myria::mtl::typecheck_phase::Binding<myria::Label<top>, bool,
      mutils::String<'a', 'n', 'o', 'r', 'm', '\x04', '\x06'>,
      myria::mtl::typecheck_phase::Expression<myria::Label<top>, bool, myria::mtl::typecheck_phase::BinOp<'>',
      myria::mtl::typecheck_phase::Expression<myria::Label<top>, int,
      myria::mtl::typecheck_phase::VarReference<mutils::String<'a', 'n', 'o', 'r', 'm', '\x04', '\x04'> > >,
      myria::mtl::typecheck_phase::Expression<myria::Label<top>, int,
      myria::mtl::typecheck_phase::VarReference<mutils::String<'y'> > > > > >,
      myria::mtl::typecheck_phase::Statement<myria::Label<top>,
      myria::mtl::typecheck_phase::If<myria::mtl::typecheck_phase::Expression<myria::Label<top>, bool,
      myria::mtl::typecheck_phase::VarReference<mutils::String<'a', 'n', 'o', 'r', 'm', '\x04', '\x06'> > >,
      myria::mtl::typecheck_phase::Statement<myria::Label<top>,
      myria::mtl::typecheck_phase::Sequence<myria::mtl::typecheck_phase::Statement<myria::Label<top>,
      myria::mtl::typecheck_phase::Let<myria::mtl::typecheck_phase::Binding<myria::Label<top>, int,
      mutils::String<'a', 'n', 'o', 'r', 'm', '\x04', '\t'>,
      myria::mtl::typecheck_phase::Expression<myria::Label<top>, int,
      myria::mtl::typecheck_phase::FieldReference<myria::mtl::typecheck_phase::Expression<myria::Label<top>,
      test_struct, myria::mtl::typecheck_phase::VarReference<mutils::String<'t', 's', 't', 'r', 'u', 'c', 't'> 
> >,
      mutils::String<'a'> > > >, myria::mtl::typecheck_phase::Statement<myria::Label<top>,
      myria::mtl::typecheck_phase::Assignment<myria::mtl::typecheck_phase::Expression<myria::Label<top>, int,
      myria::mtl::typecheck_phase::VarReference<mutils::String<'y'> > >,
      myria::mtl::typecheck_phase::Expression<myria::Label<top>, int,
      myria::mtl::typecheck_phase::VarReference<mutils::String<'a', 'n', 'o', 'r', 'm', '\x04', '\t'> > > > > 
> >,
      myria::mtl::typecheck_phase::Statement<myria::Label<top>,
      myria::mtl::typecheck_phase::Assignment<myria::mtl::typecheck_phase::Expression<myria::Label<top>, int,
      myria::mtl::typecheck_phase::FieldReference<myria::mtl::typecheck_phase::Expression<myria::Label<top>,
      test_struct, myria::mtl::typecheck_phase::VarReference<mutils::String<'t', 's', 't', 'r', 'u', 'c', 't'> 
> >,
      mutils::String<'b'> > >, myria::mtl::typecheck_phase::Expression<myria::Label<top>, int,
      myria::mtl::typecheck_phase::VarReference<mutils::String<'x'> > > > > > >,
      myria::mtl::typecheck_phase::Statement<myria::Label<top>,
      myria::mtl::typecheck_phase::Sequence<myria::mtl::typecheck_phase::Statement<myria::Label<bottom>,
      myria::mtl::typecheck_phase::LetIsValid<mutils::String<'i', 's', 'V', 'a', 'l', 'i', 'd', '_', 't', 'm', 
'p',
      '\x00', '\x05', '\t'>, myria::mtl::typecheck_phase::Expression<myria::Label<top>,
      myria::Handle<myria::Label<bottom>, int>, myria::mtl::typecheck_phase::VarReference<mutils::String<'h', 
'n',
      'd', 'l'> > >, myria::mtl::typecheck_phase::Statement<myria::Label<bottom>,
      myria::mtl::typecheck_phase::If<myria::mtl::typecheck_phase::Expression<myria::Label<bottom>, bool,
      myria::mtl::typecheck_phase::VarReference<mutils::String<'i', 's', 'V', 'a', 'l', 'i', 'd', '_', 't', 
'm', 'p',
      '\x00', '\x05', '\t'> > >, myria::mtl::typecheck_phase::Statement<myria::Label<bottom>,
      myria::mtl::typecheck_phase::Sequence<myria::mtl::typecheck_phase::Statement<myria::Label<bottom>,
      myria::mtl::typecheck_phase::Let<myria::mtl::typecheck_phase::Binding<myria::Label<bottom>, int,
      mutils::String<'a', 'n', 'o', 'r', 'm', '\x05', '\x0C'>,
      myria::mtl::typecheck_phase::Expression<myria::Label<top>, int, myria::mtl::typecheck_phase::Constant<7> 
> >,
      myria::mtl::typecheck_phase::Statement<myria::Label<bottom>,
      myria::mtl::typecheck_phase::Return<myria::mtl::typecheck_phase::Expression<myria::Label<bottom>, int,
      myria::mtl::typecheck_phase::VarReference<mutils::String<'a', 'n', 'o', 'r', 'm', '\x05', '\x0C'> > > > 
> > > >
      >, myria::mtl::typecheck_phase::Statement<myria::Label<bottom>, myria::mtl::typecheck_phase::Sequence<> 
> > > >
      > > > > > > > > > > > > > > > > > > > > > > > > > > >, myria::mtl::value_holder<test_struct, 't', 's', 
't', 'r',
      'u', 'c', 't'>, myria::mtl::value_holder<test_struct, 't', 's', 't', 'r', 'u', 'c', 't', '2'>,
      myria::mtl::value_holder<myria::Handle<myria::Label<bottom>, int>, 'h', 'n', 'd', 'l'>,
      myria::mtl::value_holder<myria::Handle<myria::Label<bottom>, test_struct>, 'h', 'n', 'd', 'l', '2'> >,
      myria::mtl::runnable_transaction::transaction<myria::mtl::runnable_transaction::phase<myria::Label<top>, 
void,
      myria::mtl::split_phase::AST<myria::Label<top> 
>::Statement<myria::mtl::split_phase::AST<myria::Label<top>
      >::Let<myria::mtl::split_phase::AST<myria::Label<top> >::Binding<myria::Label<top>, int, 
mutils::String<'z',
      'e', 'r', 'o', '\x00', '\x01'>, myria::mtl::split_phase::AST<myria::Label<top> >::Expression<int,
      myria::mtl::split_phase::AST<myria::Label<top> >::Constant<0> > >,
      myria::mtl::split_phase::AST<myria::Label<top> 
>::Statement<myria::mtl::split_phase::AST<myria::Label<top>
      >::Let<myria::mtl::split_phase::AST<myria::Label<top> >::Binding<myria::Label<top>, int, 
mutils::String<'o',
      'n', 'e', '\x00', '\x01'>, myria::mtl::split_phase::AST<myria::Label<top> >::Expression<int,
      myria::mtl::split_phase::AST<myria::Label<top> >::Constant<1> > >,
      myria::mtl::split_phase::AST<myria::Label<top> 
>::Statement<myria::mtl::split_phase::AST<myria::Label<top>
      >::Let<myria::mtl::split_phase::AST<myria::Label<top> >::Binding<myria::Label<top>, bool, 
mutils::String<'t',
      'r', 'u', 'e'>, myria::mtl::split_phase::AST<myria::Label<top> >::Expression<bool,
      myria::mtl::split_phase::AST<myria::Label<top> >::BinOp<'=', 
myria::mtl::split_phase::AST<myria::Label<top>
      >::Expression<int, myria::mtl::split_phase::AST<myria::Label<top> >::VarReference<mutils::String<'z', 
'e', 'r',
      'o', '\x00', '\x01'> > >, myria::mtl::split_phase::AST<myria::Label<top> >::Expression<int,
      myria::mtl::split_phase::AST<myria::Label<top> >::VarReference<mutils::String<'z', 'e', 'r', 'o', 
'\x00',
      '\x01'> > > > > >, myria::mtl::split_phase::AST<myria::Label<top>
      >::Statement<myria::mtl::split_phase::AST<myria::Label<top>
      >::Let<myria::mtl::split_phase::AST<myria::Label<top> >::Binding<myria::Label<top>, bool, 
mutils::String<'f',
      'a', 'l', 's', 'e'>, myria::mtl::split_phase::AST<myria::Label<top> >::Expression<bool,
      myria::mtl::split_phase::AST<myria::Label<top> >::BinOp<'=', 
myria::mtl::split_phase::AST<myria::Label<top>
      >::Expression<int, myria::mtl::split_phase::AST<myria::Label<top> >::VarReference<mutils::String<'z', 
'e', 'r',
      'o', '\x00', '\x01'> > >, myria::mtl::split_phase::AST<myria::Label<top> >::Expression<int,
      myria::mtl::split_phase::AST<myria::Label<top> >::VarReference<mutils::String<'o', 'n', 'e', '\x00', 
'\x01'> > >
      > > >, myria::mtl::split_phase::AST<myria::Label<top>
      >::Statement<myria::mtl::split_phase::AST<myria::Label<top>
      >::Let<myria::mtl::split_phase::AST<myria::Label<top> >::Binding<myria::Label<top>, int, 
mutils::String<'x'>,
      myria::mtl::split_phase::AST<myria::Label<top> >::Expression<int, 
myria::mtl::split_phase::AST<myria::Label<top>
      >::Constant<3> > >, myria::mtl::split_phase::AST<myria::Label<top>
      >::Statement<myria::mtl::split_phase::AST<myria::Label<top>
      >::Let<myria::mtl::split_phase::AST<myria::Label<top> >::Binding<myria::Label<top>, int, 
mutils::String<'y'>,
      myria::mtl::split_phase::AST<myria::Label<top> >::Expression<int, 
myria::mtl::split_phase::AST<myria::Label<top>
      >::Constant<5> > >, myria::mtl::split_phase::AST<myria::Label<top>
      >::Statement<myria::mtl::split_phase::AST<myria::Label<top>
      >::Sequence<myria::mtl::split_phase::AST<myria::Label<top>
      >::Statement<myria::mtl::split_phase::AST<myria::Label<top>
      >::Assignment<myria::mtl::split_phase::AST<myria::Label<top> >::Expression<int,
      myria::mtl::split_phase::AST<myria::Label<top> >::VarReference<mutils::String<'x'> > >,
      myria::mtl::split_phase::AST<myria::Label<top> >::Expression<int, 
myria::mtl::split_phase::AST<myria::Label<top>
      >::Constant<7> > > >, myria::mtl::split_phase::AST<myria::Label<top>
      >::Statement<myria::mtl::split_phase::AST<myria::Label<top>
      >::Assignment<myria::mtl::split_phase::AST<myria::Label<top> >::Expression<int,
      myria::mtl::split_phase::AST<myria::Label<top> >::VarReference<mutils::String<'y'> > >,
      myria::mtl::split_phase::AST<myria::Label<top> >::Expression<int, 
myria::mtl::split_phase::AST<myria::Label<top>
      >::BinOp<'+', myria::mtl::split_phase::AST<myria::Label<top> >::Expression<int,
      myria::mtl::split_phase::AST<myria::Label<top> >::VarReference<mutils::String<'y'> > >,
      myria::mtl::split_phase::AST<myria::Label<top> >::Expression<int, 
myria::mtl::split_phase::AST<myria::Label<top>
      >::VarReference<mutils::String<'x'> > > > > > >, myria::mtl::split_phase::AST<myria::Label<top>
      >::Statement<myria::mtl::split_phase::AST<myria::Label<top>
      >::Let<myria::mtl::split_phase::AST<myria::Label<top> >::Binding<myria::Label<top>, int, 
mutils::String<'c',
      'o', 'n', 'd', 'i', 't', 'i', 'o', 'n'>, myria::mtl::split_phase::AST<myria::Label<top> 
>::Expression<int,
      myria::mtl::split_phase::AST<myria::Label<top> >::Constant<0> > >,
      myria::mtl::split_phase::AST<myria::Label<top> 
>::Statement<myria::mtl::split_phase::AST<myria::Label<top>
      >::Sequence<myria::mtl::split_phase::AST<myria::Label<top>
      >::Statement<myria::mtl::split_phase::AST<myria::Label<top>
      >::Let<myria::mtl::split_phase::AST<myria::Label<top> >::Binding<myria::Label<top>, bool, 
mutils::String<'w',
      'h', 'i', 'l', 'e', '\x03', '\x04'>, myria::mtl::split_phase::AST<myria::Label<top> >::Expression<bool,
      myria::mtl::split_phase::AST<myria::Label<top> >::VarReference<mutils::String<'t', 'r', 'u', 'e'> > > >,
      myria::mtl::split_phase::AST<myria::Label<top> 
>::Statement<myria::mtl::split_phase::AST<myria::Label<top>
      >::While<myria::mtl::split_phase::AST<myria::Label<top> >::Expression<bool,
      myria::mtl::split_phase::AST<myria::Label<top> >::VarReference<mutils::String<'w', 'h', 'i', 'l', 'e', 
'\x03',
      '\x04'> > >, myria::mtl::split_phase::AST<myria::Label<top>
      >::Statement<myria::mtl::split_phase::AST<myria::Label<top>
      >::Let<myria::mtl::split_phase::AST<myria::Label<top> >::Binding<myria::Label<top>, bool, 
mutils::String<'a',
      'n', 'o', 'r', 'm', '\x03', '\x06'>, myria::mtl::split_phase::AST<myria::Label<top> >::Expression<bool,
      myria::mtl::split_phase::AST<myria::Label<top> >::BinOp<'<', 
myria::mtl::split_phase::AST<myria::Label<top>
      >::Expression<int, myria::mtl::split_phase::AST<myria::Label<top> >::VarReference<mutils::String<'c', 
'o', 'n',
      'd', 'i', 't', 'i', 'o', 'n'> > >, myria::mtl::split_phase::AST<myria::Label<top> >::Expression<int,
      myria::mtl::split_phase::AST<myria::Label<top> >::VarReference<mutils::String<'x'> > > > > >,
      myria::mtl::split_phase::AST<myria::Label<top> 
>::Statement<myria::mtl::split_phase::AST<myria::Label<top>
      >::If<myria::mtl::split_phase::AST<myria::Label<top> >::Expression<bool,
      myria::mtl::split_phase::AST<myria::Label<top> >::VarReference<mutils::String<'a', 'n', 'o', 'r', 'm', 
'\x03',
      '\x06'> > >, myria::mtl::split_phase::AST<myria::Label<top>
      >::Statement<myria::mtl::split_phase::AST<myria::Label<top>
      >::Sequence<myria::mtl::split_phase::AST<myria::Label<top>
      >::Statement<myria::mtl::split_phase::AST<myria::Label<top>
      >::Assignment<myria::mtl::split_phase::AST<myria::Label<top> >::Expression<int,
      myria::mtl::split_phase::AST<myria::Label<top> >::VarReference<mutils::String<'y'> > >,
      myria::mtl::split_phase::AST<myria::Label<top> >::Expression<int, 
myria::mtl::split_phase::AST<myria::Label<top>
      >::BinOp<'+', myria::mtl::split_phase::AST<myria::Label<top> >::Expression<int,
      myria::mtl::split_phase::AST<myria::Label<top> >::VarReference<mutils::String<'y'> > >,
      myria::mtl::split_phase::AST<myria::Label<top> >::Expression<int, 
myria::mtl::split_phase::AST<myria::Label<top>
      >::Constant<3> > > > > >, myria::mtl::split_phase::AST<myria::Label<top>
      >::Statement<myria::mtl::split_phase::AST<myria::Label<top>
      >::Assignment<myria::mtl::split_phase::AST<myria::Label<top> >::Expression<int,
      myria::mtl::split_phase::AST<myria::Label<top> >::VarReference<mutils::String<'c', 'o', 'n', 'd', 'i', 
't', 'i',
      'o', 'n'> > >, myria::mtl::split_phase::AST<myria::Label<top> >::Expression<int,
      myria::mtl::split_phase::AST<myria::Label<top> >::BinOp<'+', 
myria::mtl::split_phase::AST<myria::Label<top>
      >::Expression<int, myria::mtl::split_phase::AST<myria::Label<top>
      >::FieldReference<myria::mtl::split_phase::AST<myria::Label<top> >::Expression<test_struct,
      myria::mtl::split_phase::AST<myria::Label<top> >::VarReference<mutils::String<'t', 's', 't', 'r', 'u', 
'c', 't',
      '2'> > >, mutils::String<'a'> > >, myria::mtl::split_phase::AST<myria::Label<top> >::Expression<int,
      myria::mtl::split_phase::AST<myria::Label<top> >::Constant<1> > > > > >,
      myria::mtl::split_phase::AST<myria::Label<top> 
>::Statement<myria::mtl::split_phase::AST<myria::Label<top>
      >::Assignment<myria::mtl::split_phase::AST<myria::Label<top> >::Expression<int,
      myria::mtl::split_phase::AST<myria::Label<top> 
>::FieldReference<myria::mtl::split_phase::AST<myria::Label<top>
      >::Expression<test_struct, myria::mtl::split_phase::AST<myria::Label<top> 
>::VarReference<mutils::String<'t',
      's', 't', 'r', 'u', 'c', 't', '2'> > >, mutils::String<'a'> > >, 
myria::mtl::split_phase::AST<myria::Label<top>
      >::Expression<int, myria::mtl::split_phase::AST<myria::Label<top> >::VarReference<mutils::String<'c', 
'o', 'n',
      'd', 'i', 't', 'i', 'o', 'n'> > > > >, myria::mtl::split_phase::AST<myria::Label<top>
      >::Statement<myria::mtl::split_phase::AST<myria::Label<top>
      >::Let<myria::mtl::split_phase::AST<myria::Label<top> >::Binding<myria::Label<bottom>, int, 
mutils::String<'a',
      'n', 'o', 'r', 'm', '\x07', '\x11'>, myria::mtl::split_phase::AST<myria::Label<top> >::Expression<int,
      myria::mtl::split_phase::AST<myria::Label<top> >::Constant<1> > >,
      myria::mtl::split_phase::AST<myria::Label<top> 
>::Statement<myria::mtl::split_phase::AST<myria::Label<top>
      >::Sequence<> > > > > >, myria::mtl::split_phase::AST<myria::Label<top>
      >::Statement<myria::mtl::split_phase::AST<myria::Label<top>
      >::Assignment<myria::mtl::split_phase::AST<myria::Label<top> >::Expression<bool,
      myria::mtl::split_phase::AST<myria::Label<top> >::VarReference<mutils::String<'w', 'h', 'i', 'l', 'e', 
'\x03',
      '\x04'> > >, myria::mtl::split_phase::AST<myria::Label<top> >::Expression<bool,
      myria::mtl::split_phase::AST<myria::Label<top> >::VarReference<mutils::String<'f', 'a', 'l', 's', 'e'> > 
> > > >
      > > >, '\x01', '\x05', '\x01'> > > >, myria::mtl::split_phase::AST<myria::Label<top>
      >::Statement<myria::mtl::split_phase::AST<myria::Label<top>
      >::Let<myria::mtl::split_phase::AST<myria::Label<top> >::Binding<myria::Label<top>, bool, 
mutils::String<'a',
      'n', 'o', 'r', 'm', '\x04', '\x06'>, myria::mtl::split_phase::AST<myria::Label<top> >::Expression<bool,
      myria::mtl::split_phase::AST<myria::Label<top> >::BinOp<'>', 
myria::mtl::split_phase::AST<myria::Label<top>
      >::Expression<int, myria::mtl::split_phase::AST<myria::Label<top> >::Constant<45> >,
      myria::mtl::split_phase::AST<myria::Label<top> >::Expression<int, 
myria::mtl::split_phase::AST<myria::Label<top>
      >::VarReference<mutils::String<'y'> > > > > >, myria::mtl::split_phase::AST<myria::Label<top>
      >::Statement<myria::mtl::split_phase::AST<myria::Label<top> 
>::If<myria::mtl::split_phase::AST<myria::Label<top>
      >::Expression<bool, myria::mtl::split_phase::AST<myria::Label<top> >::VarReference<mutils::String<'a', 
'n', 'o',
      'r', 'm', '\x04', '\x06'> > >, myria::mtl::split_phase::AST<myria::Label<top>
      >::Statement<myria::mtl::split_phase::AST<myria::Label<top>
      >::Sequence<myria::mtl::split_phase::AST<myria::Label<top>
      >::Statement<myria::mtl::split_phase::AST<myria::Label<top>
      >::Assignment<myria::mtl::split_phase::AST<myria::Label<top> >::Expression<int,
      myria::mtl::split_phase::AST<myria::Label<top> >::VarReference<mutils::String<'y'> > >,
      myria::mtl::split_phase::AST<myria::Label<top> >::Expression<int, 
myria::mtl::split_phase::AST<myria::Label<top>
      >::FieldReference<myria::mtl::split_phase::AST<myria::Label<top> >::Expression<test_struct,
      myria::mtl::split_phase::AST<myria::Label<top> >::VarReference<mutils::String<'t', 's', 't', 'r', 'u', 
'c', 't'>
      > >, mutils::String<'a'> > > > >, myria::mtl::split_phase::AST<myria::Label<top>
      >::Statement<myria::mtl::split_phase::AST<myria::Label<top>
      >::Assignment<myria::mtl::split_phase::AST<myria::Label<top> >::Expression<int,
      myria::mtl::split_phase::AST<myria::Label<top> 
>::FieldReference<myria::mtl::split_phase::AST<myria::Label<top>
      >::Expression<test_struct, myria::mtl::split_phase::AST<myria::Label<top> 
>::VarReference<mutils::String<'t',
      's', 't', 'r', 'u', 'c', 't'> > >, mutils::String<'b'> > >, 
myria::mtl::split_phase::AST<myria::Label<top>
      >::Expression<int, myria::mtl::split_phase::AST<myria::Label<top> >::VarReference<mutils::String<'x'> > 
> > > >
      >, myria::mtl::split_phase::AST<myria::Label<top> 
>::Statement<myria::mtl::split_phase::AST<myria::Label<top>
      >::Sequence<> > > > > > > > > > > > > > > > > > > > > > > >,
      mutils::typeset<myria::mtl::type_holder<test_struct, 't', 's', 't', 'r', 'u', 'c', 't', '2'>,
      myria::mtl::type_holder<test_struct, 't', 's', 't', 'r', 'u', 'c', 't'> >,
      mutils::typeset<myria::mtl::type_holder<bool, 'a', 'n', 'o', 'r', 'm', '\x03', '\x06'>,
      myria::mtl::type_holder<int, 'c', 'o', 'n', 'd', 'i', 't', 'i', 'o', 'n'>, myria::mtl::type_holder<int, 
'a',
      'n', 'o', 'r', 'm', '\x07', '\x11'>, myria::mtl::value_holder<unsigned long, '\x01', '\x05', '\x01'>,
      myria::mtl::type_holder<bool, 'a', 'n', 'o', 'r', 'm', '\x04', '\x06'> >,
      mutils::typeset<myria::mtl::type_holder<int, 'z', 'e', 'r', 'o', '\x00', '\x01'>, 
myria::mtl::type_holder<int,
      'o', 'n', 'e', '\x00', '\x01'>, myria::mtl::type_holder<bool, 't', 'r', 'u', 'e'>, 
myria::mtl::type_holder<bool,
      'f', 'a', 'l', 's', 'e'>, myria::mtl::type_holder<int, 'x'>, myria::mtl::type_holder<int, 'y'>,
      myria::mtl::type_holder<bool, 'w', 'h', 'i', 'l', 'e', '\x03', '\x04'> >,
      mutils::typeset<myria::mtl::type_holder<test_struct, 't', 's', 't', 'r', 'u', 'c', 't', '2'>,
      myria::mtl::type_holder<test_struct, 't', 's', 't', 'r', 'u', 'c', 't'>,
      myria::mtl::type_holder<myria::Handle<myria::Label<bottom>, int>, 'h', 'n', 'd', 'l'>,
      myria::mtl::type_holder<myria::Handle<myria::Label<bottom>, test_struct>, 'h', 'n', 'd', 'l', '2'> > >,
      myria::mtl::runnable_transaction::phase<myria::Label<bottom>, int,
      myria::mtl::split_phase::AST<myria::Label<bottom> 
>::Statement<myria::mtl::split_phase::AST<myria::Label<bottom>
      >::Sequence<myria::mtl::split_phase::AST<myria::Label<bottom>
      >::Statement<myria::mtl::split_phase::AST<myria::Label<bottom> >::IncrementOccurance<mutils::String<'c', 
'o',
      'n', 'd', 'i', 't', 'i', 'o', 'n'> > >, myria::mtl::split_phase::AST<myria::Label<bottom>
      >::Statement<myria::mtl::split_phase::AST<myria::Label<bottom>
      >::ForEach<myria::mtl::split_phase::AST<myria::Label<bottom>
      >::Statement<myria::mtl::split_phase::AST<myria::Label<bottom>
      >::Sequence<myria::mtl::split_phase::AST<myria::Label<bottom>
      >::Statement<myria::mtl::split_phase::AST<myria::Label<bottom> >::IncrementOccurance<mutils::String<'a', 
'n',
      'o', 'r', 'm', '\x03', '\x06'> > >, myria::mtl::split_phase::AST<myria::Label<bottom>
      >::Statement<myria::mtl::split_phase::AST<myria::Label<bottom>
      >::If<myria::mtl::split_phase::AST<myria::Label<bottom> >::Expression<bool,
      myria::mtl::split_phase::AST<myria::Label<bottom> >::VarReference<mutils::String<'a', 'n', 'o', 'r', 
'm',
      '\x03', '\x06'> > >, myria::mtl::split_phase::AST<myria::Label<bottom>
      >::Statement<myria::mtl::split_phase::AST<myria::Label<bottom>
      >::Sequence<myria::mtl::split_phase::AST<myria::Label<bottom>
      >::Statement<myria::mtl::split_phase::AST<myria::Label<bottom> >::IncrementOccurance<mutils::String<'c', 
'o',
      'n', 'd', 'i', 't', 'i', 'o', 'n'> > >, myria::mtl::split_phase::AST<myria::Label<bottom>
      >::Statement<myria::mtl::split_phase::AST<myria::Label<bottom>
      >::LetRemote<myria::mtl::split_phase::AST<myria::Label<bottom> >::Binding<myria::Label<bottom>, int,
      mutils::String<'z'>, myria::mtl::split_phase::AST<myria::Label<bottom>
      >::Expression<myria::Handle<myria::Label<bottom>, int>, 
myria::mtl::split_phase::AST<myria::Label<bottom>
      >::VarReference<mutils::String<'h', 'n', 'd', 'l'> > > >, 
myria::mtl::split_phase::AST<myria::Label<bottom>
      >::Statement<myria::mtl::split_phase::AST<myria::Label<bottom>
      >::Sequence<myria::mtl::split_phase::AST<myria::Label<bottom>
      >::Statement<myria::mtl::split_phase::AST<myria::Label<bottom>
      >::Assignment<myria::mtl::split_phase::AST<myria::Label<bottom> >::Expression<int,
      myria::mtl::split_phase::AST<myria::Label<bottom> >::VarReference<mutils::String<'z'> > >,
      myria::mtl::split_phase::AST<myria::Label<bottom> >::Expression<int,
      myria::mtl::split_phase::AST<myria::Label<bottom> >::VarReference<mutils::String<'c', 'o', 'n', 'd', 
'i', 't',
      'i', 'o', 'n'> > > > >, myria::mtl::split_phase::AST<myria::Label<bottom>
      >::Statement<myria::mtl::split_phase::AST<myria::Label<bottom>
      >::LetRemote<myria::mtl::split_phase::AST<myria::Label<bottom> >::Binding<myria::Label<bottom>, int,
      mutils::String<'r', 'e', 'm', 'o', 't', 'e', '_', 'b', 'o', 'u', 'n', 'd', '_', 't', 'm', 'p', '\x00', 
'\x07',
      '\n'>, myria::mtl::split_phase::AST<myria::Label<bottom> 
>::Expression<myria::Handle<myria::Label<bottom>, int>,
      myria::mtl::split_phase::AST<myria::Label<bottom> >::VarReference<mutils::String<'h', 'n', 'd', 'l'> > > 
>,
      myria::mtl::split_phase::AST<myria::Label<bottom> 
>::Statement<myria::mtl::split_phase::AST<myria::Label<bottom>
      >::Let<myria::mtl::split_phase::AST<myria::Label<bottom> >::Binding<myria::Label<bottom>, int,
      mutils::String<'u', 'n', 'u', 's', 'e', 'd'>, myria::mtl::split_phase::AST<myria::Label<bottom>
      >::Expression<int, myria::mtl::split_phase::AST<myria::Label<bottom> >::VarReference<mutils::String<'r', 
'e',
      'm', 'o', 't', 'e', '_', 'b', 'o', 'u', 'n', 'd', '_', 't', 'm', 'p', '\x00', '\x07', '\n'> > > >,
      myria::mtl::split_phase::AST<myria::Label<bottom> 
>::Statement<myria::mtl::split_phase::AST<myria::Label<bottom>
      >::Sequence<myria::mtl::split_phase::AST<myria::Label<bottom>
      >::Statement<myria::mtl::split_phase::AST<myria::Label<bottom>
      >::LetRemote<myria::mtl::split_phase::AST<myria::Label<bottom> >::Binding<myria::Label<bottom>, 
test_struct,
      mutils::String<'r', 'e', 'm', 'o', 't', 'e', '_', 'b', 'o', 'u', 'n', 'd', '_', 't', 'm', 'p', '\x00', 
'\x07',
      '\x0D'>, myria::mtl::split_phase::AST<myria::Label<bottom> 
>::Expression<myria::Handle<myria::Label<bottom>,
      test_struct>, myria::mtl::split_phase::AST<myria::Label<bottom> >::VarReference<mutils::String<'h', 'n', 
'd',
      'l', '2'> > > >, myria::mtl::split_phase::AST<myria::Label<bottom>
      >::Statement<myria::mtl::split_phase::AST<myria::Label<bottom>
      >::Sequence<myria::mtl::split_phase::AST<myria::Label<bottom>
      >::Statement<myria::mtl::split_phase::AST<myria::Label<bottom> >::IncrementOccurance<mutils::String<'a', 
'n',
      'o', 'r', 'm', '\x07', '\x11'> > >, myria::mtl::split_phase::AST<myria::Label<bottom>
      >::Statement<myria::mtl::split_phase::AST<myria::Label<bottom>
      >::Assignment<myria::mtl::split_phase::AST<myria::Label<bottom> >::Expression<int,
      myria::mtl::split_phase::AST<myria::Label<bottom> >::VarReference<mutils::String<'u', 'n', 'u', 's', 
'e', 'd'> >
      >, myria::mtl::split_phase::AST<myria::Label<bottom> >::Expression<int,
      myria::mtl::split_phase::AST<myria::Label<bottom> >::BinOp<'-',
      myria::mtl::split_phase::AST<myria::Label<bottom> >::Expression<int,
      myria::mtl::split_phase::AST<myria::Label<bottom>
      >::FieldReference<myria::mtl::split_phase::AST<myria::Label<bottom> >::Expression<test_struct,
      myria::mtl::split_phase::AST<myria::Label<bottom> >::VarReference<mutils::String<'r', 'e', 'm', 'o', 
't', 'e',
      '_', 'b', 'o', 'u', 'n', 'd', '_', 't', 'm', 'p', '\x00', '\x07', '\x0D'> > >, mutils::String<'a'> > >,
      myria::mtl::split_phase::AST<myria::Label<bottom> >::Expression<int,
      myria::mtl::split_phase::AST<myria::Label<bottom> >::VarReference<mutils::String<'a', 'n', 'o', 'r', 
'm',
      '\x07', '\x11'> > > > > > > > > > >, myria::mtl::split_phase::AST<myria::Label<bottom>
      >::Statement<myria::mtl::split_phase::AST<myria::Label<bottom>
      >::Assignment<myria::mtl::split_phase::AST<myria::Label<bottom> >::Expression<int,
      myria::mtl::split_phase::AST<myria::Label<bottom> >::VarReference<mutils::String<'z'> > >,
      myria::mtl::split_phase::AST<myria::Label<bottom> >::Expression<int,
      myria::mtl::split_phase::AST<myria::Label<bottom> >::VarReference<mutils::String<'u', 'n', 'u', 's', 
'e', 'd'> >
      > > >, myria::mtl::split_phase::AST<myria::Label<bottom>
      >::Statement<myria::mtl::split_phase::AST<myria::Label<bottom>
      >::Let<myria::mtl::split_phase::AST<myria::Label<bottom> >::Binding<myria::Label<bottom>, int,
      mutils::String<'a'>, myria::mtl::split_phase::AST<myria::Label<bottom> >::Expression<int,
      myria::mtl::split_phase::AST<myria::Label<bottom> >::VarReference<mutils::String<'z'> > > >,
      myria::mtl::split_phase::AST<myria::Label<bottom> 
>::Statement<myria::mtl::split_phase::AST<myria::Label<bottom>
      >::Sequence<myria::mtl::split_phase::AST<myria::Label<bottom>
      >::Statement<myria::mtl::split_phase::AST<myria::Label<bottom>
      >::Assignment<myria::mtl::split_phase::AST<myria::Label<bottom> >::Expression<int,
      myria::mtl::split_phase::AST<myria::Label<bottom> >::VarReference<mutils::String<'z'> > >,
      myria::mtl::split_phase::AST<myria::Label<bottom> >::Expression<int,
      myria::mtl::split_phase::AST<myria::Label<bottom> >::VarReference<mutils::String<'a'> > > > > > > > > > 
> > > >
      > > > > > > >, myria::mtl::split_phase::AST<myria::Label<bottom>
      >::Statement<myria::mtl::split_phase::AST<myria::Label<bottom> >::Sequence<> > > > > >, '\x01', '\x05', 
'\x01'>
      >, myria::mtl::split_phase::AST<myria::Label<bottom>
      >::Statement<myria::mtl::split_phase::AST<myria::Label<bottom> >::IncrementOccurance<mutils::String<'a', 
'n',
      'o', 'r', 'm', '\x04', '\x06'> > >, myria::mtl::split_phase::AST<myria::Label<bottom>
      >::Statement<myria::mtl::split_phase::AST<myria::Label<bottom>
      >::If<myria::mtl::split_phase::AST<myria::Label<bottom> >::Expression<bool,
      myria::mtl::split_phase::AST<myria::Label<bottom> >::VarReference<mutils::String<'a', 'n', 'o', 'r', 
'm',
      '\x04', '\x06'> > >, myria::mtl::split_phase::AST<myria::Label<bottom>
      >::Statement<myria::mtl::split_phase::AST<myria::Label<bottom> >::Sequence<> >,
      myria::mtl::split_phase::AST<myria::Label<bottom> 
>::Statement<myria::mtl::split_phase::AST<myria::Label<bottom>
      >::Sequence<myria::mtl::split_phase::AST<myria::Label<bottom>
      >::Statement<myria::mtl::split_phase::AST<myria::Label<bottom> >::LetIsValid<mutils::String<'i', 's', 
'V', 'a',
      'l', 'i', 'd', '_', 't', 'm', 'p', '\x00', '\x05', '\t'>, 
myria::mtl::split_phase::AST<myria::Label<bottom>
      >::Expression<myria::Handle<myria::Label<bottom>, int>, 
myria::mtl::split_phase::AST<myria::Label<bottom>
      >::VarReference<mutils::String<'h', 'n', 'd', 'l'> > >, 
myria::mtl::split_phase::AST<myria::Label<bottom>
      >::Statement<myria::mtl::split_phase::AST<myria::Label<bottom>
      >::If<myria::mtl::split_phase::AST<myria::Label<bottom> >::Expression<bool,
      myria::mtl::split_phase::AST<myria::Label<bottom> >::VarReference<mutils::String<'i', 's', 'V', 'a', 
'l', 'i',
      'd', '_', 't', 'm', 'p', '\x00', '\x05', '\t'> > >, myria::mtl::split_phase::AST<myria::Label<bottom>
      >::Statement<myria::mtl::split_phase::AST<myria::Label<bottom>
      >::Sequence<myria::mtl::split_phase::AST<myria::Label<bottom>
      >::Statement<myria::mtl::split_phase::AST<myria::Label<bottom>
      >::Return<myria::mtl::split_phase::AST<myria::Label<bottom> >::Expression<int,
      myria::mtl::split_phase::AST<myria::Label<bottom> >::Constant<7> > > > > >,
      myria::mtl::split_phase::AST<myria::Label<bottom> 
>::Statement<myria::mtl::split_phase::AST<myria::Label<bottom>
      >::Sequence<> > > > > > > > > > > >, mutils::typeset<myria::mtl::type_holder<bool, 'a', 'n', 'o', 'r', 
'm',
      '\x03', '\x06'>, myria::mtl::type_holder<myria::Handle<myria::Label<bottom>, int>, 'h', 'n', 'd', 'l'>,
      myria::mtl::type_holder<int, 'c', 'o', 'n', 'd', 'i', 't', 'i', 'o', 'n'>,
      myria::mtl::type_holder<myria::Handle<myria::Label<bottom>, test_struct>, 'h', 'n', 'd', 'l', '2'>,
      myria::mtl::type_holder<int, 'a', 'n', 'o', 'r', 'm', '\x07', '\x11'>, myria::mtl::value_holder<unsigned 
long,
      '\x01', '\x05', '\x01'>, myria::mtl::type_holder<bool, 'a', 'n', 'o', 'r', 'm', '\x04', '\x06'> >,
      mutils::typeset<>, mutils::typeset<myria::mtl::remote_holder<myria::Handle<myria::Label<bottom>, int>, 
int,
      'z'>, myria::mtl::remote_holder<myria::Handle<myria::Label<bottom>, int>, int, 'r', 'e', 'm', 'o', 't', 
'e',
      '_', 'b', 'o', 'u', 'n', 'd', '_', 't', 'm', 'p', '\x00', '\x07', '\n'>, myria::mtl::type_holder<int, 
'u', 'n',
      'u', 's', 'e', 'd'>, myria::mtl::remote_holder<myria::Handle<myria::Label<bottom>, test_struct>, 
test_struct,
      'r', 'e', 'm', 'o', 't', 'e', '_', 'b', 'o', 'u', 'n', 'd', '_', 't', 'm', 'p', '\x00', '\x07', '\x0D'>,
      myria::mtl::type_holder<int, 'a'>, myria::mtl::remote_isValid_holder<myria::Handle<myria::Label<bottom>, 
int>,
      'i', 's', 'V', 'a', 'l', 'i', 'd', '_', 't', 'm', 'p', '\x00', '\x05', '\t'> >,
      mutils::typeset<myria::mtl::type_holder<bool, 'a', 'n', 'o', 'r', 'm', '\x03', '\x06'>,
      myria::mtl::type_holder<myria::Handle<myria::Label<bottom>, int>, 'h', 'n', 'd', 'l'>,
      myria::mtl::type_holder<int, 'c', 'o', 'n', 'd', 'i', 't', 'i', 'o', 'n'>,
      myria::mtl::type_holder<myria::Handle<myria::Label<bottom>, test_struct>, 'h', 'n', 'd', 'l', '2'>,
      myria::mtl::type_holder<int, 'a', 'n', 'o', 'r', 'm', '\x07', '\x11'>, myria::mtl::value_holder<unsigned 
long,
      '\x01', '\x05', '\x01'>, myria::mtl::type_holder<bool, 'a', 'n', 'o', 'r', 'm', '\x04', '\x06'> > > >,
      myria::mtl::value_holder<test_struct, 't', 's', 't', 'r', 'u', 'c', 't'>, 
myria::mtl::value_holder<test_struct,
      't', 's', 't', 'r', 'u', 'c', 't', '2'>, myria::mtl::value_holder<myria::Handle<myria::Label<bottom>, 
int>, 'h',
      'n', 'd', 'l'>, myria::mtl::value_holder<myria::Handle<myria::Label<bottom>, test_struct>, 'h', 'n', 
'd', 'l',
      '2'> >
txn{};
  std::cout << txn << std::endl;
	

	ClientTracker<> trk;
		txn.run_local(trk,tstruct,tstruct2,hndl,hndl2);//*/
}