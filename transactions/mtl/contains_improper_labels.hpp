#pragma once

namespace myria {
namespace mtl {
namespace typecheck_phase {

template <typename ast>
constexpr auto contains_improper_labels_helper(ast);

// label_min_of
template <typename l, typename r, typename y, typename v, typename e>
constexpr auto _contains_improper_labels_helper(Binding<Label<label_min_of<l, r>>, y, v, e>)
{
	return true;
}

template <typename l, typename r, typename y, typename s, typename f>
constexpr auto _contains_improper_labels_helper(Expression<Label<label_min_of<l, r>>, y, FieldReference<s, f>>)
{
	return true;
}

template <typename l, typename r, typename y, typename v>
constexpr auto _contains_improper_labels_helper(Expression<Label<label_min_of<l, r>>, y, VarReference<v>>)
{
	return true;
}

template <typename l, typename r, typename y, char op, typename L, typename R>
constexpr auto _contains_improper_labels_helper(Expression<Label<label_min_of<l, r>>, y, BinOp<op, L, R>>)
{
	return true;
}

template <typename l, typename r>
constexpr auto _contains_improper_labels_helper(Expression<Label<label_min_of<l, r>>, tracker::Tombstone, GenerateTombstone>)
{
	return true;
}

template <typename l, typename r, typename v, typename e>
constexpr auto _contains_improper_labels_helper(Statement<Label<label_min_of<l, r>>, Assignment<v, e>>)
{
	return true;
}

template <typename l, typename r, typename b, typename e>
constexpr auto _contains_improper_labels_helper(Statement<Label<label_min_of<l, r>>, Let<b, e>>)
{
	return true;
}

template <typename l, typename r, typename b, typename e>
constexpr auto _contains_improper_labels_helper(Statement<Label<label_min_of<l, r>>, LetRemote<b, e>>)
{
	return true;
}

	template <typename l, typename r, typename y, typename h>
constexpr auto _contains_improper_labels_helper(Expression<Label<label_min_of<l, r>>, y, IsValid<h>>)
{
	return true;
}

	template <typename l, typename r, typename y, typename oper_name, typename Hndl, typename... args>
	constexpr auto _contains_improper_labels_helper(Expression<Label<label_min_of<l, r>>, y, Operation<oper_name, Hndl, args...>>)
{
	return true;
}
	
template <typename l, typename r, typename oper_name, typename Hndl, typename... args>
constexpr auto _contains_improper_labels_helper(Statement<Label<label_min_of<l, r>>, Operation<oper_name, Hndl, args...>>)
{
	return true;
}

template <typename l, typename r, typename c, typename t, typename e>
constexpr auto _contains_improper_labels_helper(Statement<Label<label_min_of<l, r>>, If<c, t, e>>)
{
	return true;
}

template <typename l, typename r, typename c, typename e>
constexpr auto _contains_improper_labels_helper(Statement<Label<label_min_of<l, r>>, While<c, e>>)
{
	return true;
}

template <typename l, typename r>
constexpr auto _contains_improper_labels_helper(Statement<Label<label_min_of<l, r>>, Sequence<>>)
{
  return true;
}

template <typename l, typename r, typename s1, typename... seq>
constexpr auto _contains_improper_labels_helper(Statement<Label<label_min_of<l, r>>, Sequence<s1, seq...>>)
{
  return true;
}

// temp
template <int l, int r, typename y, typename v, typename e>
constexpr auto _contains_improper_labels_helper(Binding<Label<temp_label<l, r>>, y, v, e>)
{
  return true;
}

template <int l, int r, typename y, typename s, typename f>
constexpr auto _contains_improper_labels_helper(Expression<Label<temp_label<l, r>>, y, FieldReference<s, f>>)
{
  return true;
}

template <int l, int r, typename y, typename v>
constexpr auto _contains_improper_labels_helper(Expression<Label<temp_label<l, r>>, y, VarReference<v>>)
{
  return true;
}

template <int l, int r, typename y, char op, typename L, typename R>
constexpr auto _contains_improper_labels_helper(Expression<Label<temp_label<l, r>>, y, BinOp<op, L, R>>)
{
  return true;
}
template <int l, int r>
constexpr auto _contains_improper_labels_helper(Expression<Label<temp_label<l, r> >, tracker::Tombstone, GenerateTombstone>)
{
  return true;
}

template <int l, int r, typename v, typename e>
constexpr auto _contains_improper_labels_helper(Statement<Label<temp_label<l, r>>, Assignment<v, e>>)
{
  return true;
}

template <int l, int r, typename b, typename e>
constexpr auto _contains_improper_labels_helper(Statement<Label<temp_label<l, r>>, Let<b, e>>)
{
  return true;
}

template <int l, int r, typename b, typename e>
constexpr auto _contains_improper_labels_helper(Statement<Label<temp_label<l, r>>, LetRemote<b, e>>)
{
  return true;
}

	template <int l, int r, typename y, typename h>
	constexpr auto _contains_improper_labels_helper(Expression<Label<temp_label<l, r>>, y, IsValid<h>>)
{
	return true;
}

template <int l, int r, typename y, typename oper_name, typename Hndl, typename... args>
constexpr auto _contains_improper_labels_helper(Expression<Label<temp_label<l, r>>, y, Operation< oper_name,  Hndl,  args...>>)
{
	return true;
}
	
template <int l, int r, typename oper_name, typename Hndl, typename... args>
constexpr auto _contains_improper_labels_helper(Statement<Label<temp_label<l, r>>, Operation< oper_name,  Hndl,  args...>>)
{
	return true;
}

template <int l, int r, typename c, typename t, typename e>
constexpr auto _contains_improper_labels_helper(Statement<Label<temp_label<l, r>>, If<c, t, e>>)
{
  return true;
}

template <int l, int r, typename c, typename e>
constexpr auto _contains_improper_labels_helper(Statement<Label<temp_label<l, r>>, While<c, e>>)
{
  return true;
}

template <int l, int r>
constexpr auto _contains_improper_labels_helper(Statement<Label<temp_label<l, r>>, Sequence<>>)
{
  return true;
}

template <int l, int r, typename s1, typename... seq>
constexpr auto _contains_improper_labels_helper(Statement<Label<temp_label<l, r>>, Sequence<s1, seq...>>)
{
  return true;
}
// match

template <typename l, typename y, typename v, typename e>
constexpr auto _contains_improper_labels_helper(Binding<l, y, v, e>)
{
  return contains_improper_labels_helper(e{});
}

template <typename l, typename y, typename s, typename f>
constexpr auto _contains_improper_labels_helper(Expression<l, y, FieldReference<s, f>>)
{
  return (contains_improper_labels_helper(s{}));
}

template <typename l, typename y, typename v>
constexpr auto _contains_improper_labels_helper(Expression<l, y, VarReference<v>>)
{
	return false;
}

template <int i>
constexpr auto _contains_improper_labels_helper(Expression<Label<top>, int, Constant<i>>)
{
	return false;
}

template <typename l, typename y, char op, typename L, typename R>
constexpr auto _contains_improper_labels_helper(Expression<l, y, BinOp<op, L, R>>)
{
  return (contains_improper_labels_helper(L{}))||(contains_improper_labels_helper(R{}));
}

	
template <typename l>
constexpr auto _contains_improper_labels_helper(Expression<l, tracker::Tombstone, GenerateTombstone>)
{
	return false;
}

template <typename l, typename v, typename e>
constexpr auto _contains_improper_labels_helper(Statement<l, Assignment<v, e>>)
{
	return (contains_improper_labels_helper(v{}))||(contains_improper_labels_helper(e{}));
}

template <typename l, typename e>
constexpr auto _contains_improper_labels_helper(Statement<l, Return<e>>)
{
	return (contains_improper_labels_helper(e{}));
}

template <typename l, typename e>
constexpr auto _contains_improper_labels_helper(Statement<l, WriteTombstone<e>>)
{
	return (contains_improper_labels_helper(e{}));
}

template <typename l, typename e>
constexpr auto _contains_improper_labels_helper(Statement<l, AccompanyWrite<e>>)
{
	return (contains_improper_labels_helper(e{}));
}

template <typename l, typename b, typename e>
constexpr auto _contains_improper_labels_helper(Statement<l, Let<b, e>>)
{
	return (contains_improper_labels_helper(b{}))||(contains_improper_labels_helper(e{}));
}

template <typename l, typename b, typename e>
constexpr auto _contains_improper_labels_helper(Statement<l, LetRemote<b, e>>)
{
	return (contains_improper_labels_helper(b{}))||(contains_improper_labels_helper(e{}));
}

	template <typename l, typename y, typename h>
	constexpr auto _contains_improper_labels_helper(Expression<l, y, IsValid<h>>)
{
	return (contains_improper_labels_helper(h{}));
}

	template <typename l, typename y, typename h>
	constexpr auto _contains_improper_labels_helper(Expression<l, y, Endorse<l,h>>)
{
	return (contains_improper_labels_helper(h{}));
}

template <typename l, typename y, typename oper_name, typename Hndl, typename... args>
constexpr auto _contains_improper_labels_helper(Expression<l, y, Operation< oper_name,  Hndl,  args...>>)
{
	return (contains_improper_labels_helper(Hndl{}) || ... || contains_improper_labels_helper(args{})) ;
}
	
template <typename l, typename oper_name, typename Hndl, typename... args>
constexpr auto _contains_improper_labels_helper(Statement<l, Operation< oper_name,  Hndl,  args...>>)
{
	return (contains_improper_labels_helper(Hndl{}) || ... || contains_improper_labels_helper(args{}));
}

template <typename l, typename c, typename t, typename e>
constexpr auto _contains_improper_labels_helper(Statement<l, If<c, t, e>>)
{
	return (contains_improper_labels_helper(c{}))||(contains_improper_labels_helper(t{}))||(contains_improper_labels_helper(e{}));
}

template <typename l, typename c, typename e>
constexpr auto _contains_improper_labels_helper(Statement<l, While<c, e>>)
{
	return (contains_improper_labels_helper(c{}))||(contains_improper_labels_helper(e{}));
}

template <typename l, typename... seq>
constexpr auto _contains_improper_labels_helper(Statement<l, Sequence<seq...>>)
{
	return (contains_improper_labels_helper(seq{}) || ...);
}

template <typename ast>
constexpr auto contains_improper_labels_helper(ast)
{
  return _contains_improper_labels_helper(ast{});
}

template <typename ast>
constexpr auto contains_improper_labels(ast)
{
  return contains_improper_labels_helper(ast{});
}
}
}
}
