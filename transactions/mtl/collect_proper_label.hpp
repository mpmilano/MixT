#pragma once

namespace myria {
namespace mtl {
namespace typecheck_phase {

template <typename ast>
constexpr auto collect_labels_helper(ast);

// label_min_of
template <typename l, typename r, typename y, typename v, typename e>
constexpr auto _collect_labels_helper(Binding<Label<label_min_of<l, r>>, y, v, e>)
{
  return collect_labels_helper(e{});
}

template <typename l, typename r, typename y, typename s, typename f>
constexpr auto _collect_labels_helper(Expression<Label<label_min_of<l, r>>, y, FieldReference<s, f>>)
{
  return collect_labels_helper(s{});
}

template <typename l, typename r, typename y, typename v>
constexpr auto _collect_labels_helper(Expression<Label<label_min_of<l, r>>, y, VarReference<v>>)
{
  return mutils::typeset<>{};
}

template <typename l, typename r, typename y, char op, typename L, typename R>
constexpr auto _collect_labels_helper(Expression<Label<label_min_of<l, r>>, y, BinOp<op, L, R>>)
{
  return collect_labels_helper(L{}).combine(collect_labels_helper(R{}));
}

template <typename l, typename r>
constexpr auto _collect_labels_helper(Expression<Label<label_min_of<l, r>>, tracker::Tombstone, GenerateTombstone>)
{
  return mutils::typeset<>{};
}

template <typename l, typename r, typename v, typename e>
constexpr auto _collect_labels_helper(Statement<Label<label_min_of<l, r>>, Assignment<v, e>>)
{
  return collect_labels_helper(v{}).combine(collect_labels_helper(e{}));
}

template <typename l, typename r, typename b, typename e>
constexpr auto _collect_labels_helper(Statement<Label<label_min_of<l, r>>, Let<b, e>>)
{
  return collect_labels_helper(b{}).combine(collect_labels_helper(e{}));
}

template <typename l, typename r, typename b, typename e>
constexpr auto _collect_labels_helper(Statement<Label<label_min_of<l, r>>, LetRemote<b, e>>)
{
  return collect_labels_helper(b{}).combine(collect_labels_helper(e{}));
}

	template <typename l, typename r, typename y, typename h>
constexpr auto _collect_labels_helper(Expression<Label<label_min_of<l, r>>, y, IsValid<h>>)
{
	return collect_labels_helper(h{});
}

	template <typename l, typename r, typename y, typename oper_name, typename Hndl, typename... args>
	constexpr auto _collect_labels_helper(Expression<Label<label_min_of<l, r>>, y, Operation<oper_name, Hndl, args...>>)
{
	return collect_labels_helper(Hndl{}).combine(collect_labels_helper(args{})...);
}
	
template <typename l, typename r, typename oper_name, typename Hndl, typename... args>
constexpr auto _collect_labels_helper(Statement<Label<label_min_of<l, r>>, Operation<oper_name, Hndl, args...>>)
{
	return collect_labels_helper(Hndl{}).combine(collect_labels_helper(args{})...);
}

template <typename l, typename r, typename c, typename t, typename e>
constexpr auto _collect_labels_helper(Statement<Label<label_min_of<l, r>>, If<c, t, e>>)
{
  return collect_labels_helper(c{}).combine(collect_labels_helper(t{})).combine(collect_labels_helper(e{}));
}

template <typename l, typename r, typename c, typename e>
constexpr auto _collect_labels_helper(Statement<Label<label_min_of<l, r>>, While<c, e>>)
{
  return collect_labels_helper(c{}).combine(collect_labels_helper(e{}));
}

template <typename l, typename r>
constexpr auto _collect_labels_helper(Statement<Label<label_min_of<l, r>>, Sequence<>>)
{
  return mutils::typeset<>{};
}

template <typename l, typename r, typename s1, typename... seq>
constexpr auto _collect_labels_helper(Statement<Label<label_min_of<l, r>>, Sequence<s1, seq...>>)
{
  return collect_labels_helper(s1{}).combine(collect_labels_helper(seq{})...);
}

// temp
template <int l, int r, typename y, typename v, typename e>
constexpr auto _collect_labels_helper(Binding<Label<temp_label<l, r>>, y, v, e>)
{
  return collect_labels_helper(e{});
}

template <int l, int r, typename y, typename s, typename f>
constexpr auto _collect_labels_helper(Expression<Label<temp_label<l, r>>, y, FieldReference<s, f>>)
{
  return collect_labels_helper(s{});
}

template <int l, int r, typename y, typename v>
constexpr auto _collect_labels_helper(Expression<Label<temp_label<l, r>>, y, VarReference<v>>)
{
  return mutils::typeset<>{};
}

template <int l, int r, typename y, char op, typename L, typename R>
constexpr auto _collect_labels_helper(Expression<Label<temp_label<l, r>>, y, BinOp<op, L, R>>)
{
  return collect_labels_helper(L{}).combine(collect_labels_helper(R{}));
}
template <int l, int r>
constexpr auto _collect_labels_helper(Expression<Label<temp_label<l, r> >, tracker::Tombstone, GenerateTombstone>)
{
  return mutils::typeset<>{};
}

template <int l, int r, typename v, typename e>
constexpr auto _collect_labels_helper(Statement<Label<temp_label<l, r>>, Assignment<v, e>>)
{
  return collect_labels_helper(v{}).combine(collect_labels_helper(e{}));
}

template <int l, int r, typename b, typename e>
constexpr auto _collect_labels_helper(Statement<Label<temp_label<l, r>>, Let<b, e>>)
{
  return collect_labels_helper(b{}).combine(collect_labels_helper(e{}));
}

template <int l, int r, typename b, typename e>
constexpr auto _collect_labels_helper(Statement<Label<temp_label<l, r>>, LetRemote<b, e>>)
{
  return collect_labels_helper(b{}).combine(collect_labels_helper(e{}));
}

	template <int l, int r, typename y, typename h>
	constexpr auto _collect_labels_helper(Expression<Label<temp_label<l, r>>, y, IsValid<h>>)
{
	return collect_labels_helper(h{});
}

template <int l, int r, typename y, typename oper_name, typename Hndl, typename... args>
constexpr auto _collect_labels_helper(Expression<Label<temp_label<l, r>>, y, Operation< oper_name,  Hndl,  args...>>)
{
	return collect_labels_helper(Hndl{}).combine(collect_labels_helper(args{})...);
}
	
template <int l, int r, typename oper_name, typename Hndl, typename... args>
constexpr auto _collect_labels_helper(Statement<Label<temp_label<l, r>>, Operation< oper_name,  Hndl,  args...>>)
{
	return collect_labels_helper(Hndl{}).combine(collect_labels_helper(args{})...);
}

template <int l, int r, typename c, typename t, typename e>
constexpr auto _collect_labels_helper(Statement<Label<temp_label<l, r>>, If<c, t, e>>)
{
  return collect_labels_helper(c{}).combine(collect_labels_helper(t{})).combine(collect_labels_helper(e{}));
}

template <int l, int r, typename c, typename e>
constexpr auto _collect_labels_helper(Statement<Label<temp_label<l, r>>, While<c, e>>)
{
  return collect_labels_helper(c{}).combine(collect_labels_helper(e{}));
}

template <int l, int r>
constexpr auto _collect_labels_helper(Statement<Label<temp_label<l, r>>, Sequence<>>)
{
  return mutils::typeset<>{};
}

template <int l, int r, typename s1, typename... seq>
constexpr auto _collect_labels_helper(Statement<Label<temp_label<l, r>>, Sequence<s1, seq...>>)
{
  return collect_labels_helper(s1{}).combine(collect_labels_helper(seq{})...);
}
// match

template <typename l, typename y, typename v, typename e>
constexpr auto _collect_labels_helper(Binding<l, y, v, e>)
{
  static_assert(l::is_label::value);
  return mutils::typeset<l>::combine(collect_labels_helper(e{}));
}

template <typename l, typename y, typename s, typename f>
constexpr auto _collect_labels_helper(Expression<l, y, FieldReference<s, f>>)
{
  static_assert(l::is_label::value);
  return mutils::typeset<l>::combine(collect_labels_helper(s{}));
}

template <typename l, typename y, typename v>
constexpr auto _collect_labels_helper(Expression<l, y, VarReference<v>>)
{
  static_assert(l::is_label::value);
  return mutils::typeset<l>{};
}

template <int i>
constexpr auto _collect_labels_helper(Expression<Label<top>, int, Constant<i>>)
{
  return mutils::typeset<Label<top>>{};
}

template <typename l, typename y, char op, typename L, typename R>
constexpr auto _collect_labels_helper(Expression<l, y, BinOp<op, L, R>>)
{
  using namespace mutils;
  static_assert(l::is_label::value);
  return typeset<l>::combine(collect_labels_helper(L{})).combine(collect_labels_helper(R{}));
}

	
template <typename l>
constexpr auto _collect_labels_helper(Expression<l, tracker::Tombstone, GenerateTombstone>)
{
  using namespace mutils;
  static_assert(l::is_label::value);
  return typeset<l>{};
}

template <typename l, typename v, typename e>
constexpr auto _collect_labels_helper(Statement<l, Assignment<v, e>>)
{
  using namespace mutils;
  static_assert(l::is_label::value);
  return typeset<l>::combine(collect_labels_helper(v{})).combine(collect_labels_helper(e{}));
}

template <typename l, typename e>
constexpr auto _collect_labels_helper(Statement<l, Return<e>>)
{
  using namespace mutils;
  static_assert(l::is_label::value);
  return typeset<l>::combine(collect_labels_helper(e{}));
}

template <typename l, typename e>
constexpr auto _collect_labels_helper(Statement<l, WriteTombstone<e>>)
{
  using namespace mutils;
  static_assert(l::is_label::value);
  return typeset<l>::combine(collect_labels_helper(e{}));
}

template <typename l, typename e>
constexpr auto _collect_labels_helper(Statement<l, AccompanyWrite<e>>)
{
  using namespace mutils;
  static_assert(l::is_label::value);
  return typeset<l>::combine(collect_labels_helper(e{}));
}

template <typename l, typename b, typename e>
constexpr auto _collect_labels_helper(Statement<l, Let<b, e>>)
{
  using namespace mutils;
  static_assert(l::is_label::value);
  return typeset<l>::combine(collect_labels_helper(b{})).combine(collect_labels_helper(e{}));
}

template <typename l, typename b, typename e>
constexpr auto _collect_labels_helper(Statement<l, LetRemote<b, e>>)
{
  using namespace mutils;
  static_assert(l::is_label::value);
  return typeset<l>::combine(collect_labels_helper(b{})).combine(collect_labels_helper(e{}));
}

	template <typename l, typename y, typename h>
	constexpr auto _collect_labels_helper(Expression<l, y, IsValid<h>>)
{
  using namespace mutils;
  static_assert(l::is_label::value);
  return typeset<l>::combine(collect_labels_helper(h{}));
}

template <typename l, typename y, typename oper_name, typename Hndl, typename... args>
constexpr auto _collect_labels_helper(Expression<l, y, Operation< oper_name,  Hndl,  args...>>)
{
  using namespace mutils;
  static_assert(l::is_label::value);
  return typeset<l>::combine(collect_labels_helper(Hndl{})).combine(collect_labels_helper(args{})...);
}
	
template <typename l, typename oper_name, typename Hndl, typename... args>
constexpr auto _collect_labels_helper(Statement<l, Operation< oper_name,  Hndl,  args...>>)
{
  using namespace mutils;
  static_assert(l::is_label::value);
  return typeset<l>::combine(collect_labels_helper(Hndl{})).combine(collect_labels_helper(args{})...);
}

template <typename l, typename c, typename t, typename e>
constexpr auto _collect_labels_helper(Statement<l, If<c, t, e>>)
{
  using namespace mutils;
  static_assert(l::is_label::value);
  return typeset<l>::combine(collect_labels_helper(c{})).combine(collect_labels_helper(t{})).combine(collect_labels_helper(e{}));
}

template <typename l, typename c, typename e>
constexpr auto _collect_labels_helper(Statement<l, While<c, e>>)
{
  using namespace mutils;
  static_assert(l::is_label::value);
  return typeset<l>::combine(collect_labels_helper(c{})).combine(collect_labels_helper(e{}));
}

template <typename l, typename... seq>
constexpr auto _collect_labels_helper(Statement<l, Sequence<seq...>>)
{
  using namespace mutils;
  static_assert(l::is_label::value);
  return typeset<l>::combine(collect_labels_helper(seq{})...);
}

template <typename ast>
constexpr auto collect_labels_helper(ast)
{
  return _collect_labels_helper(ast{});
}

template <typename... rst>
constexpr auto sort_labels(mutils::typeset<rst...> a)
{
  return a.template as_sorted_list<label_lte>();
}

template <typename ast>
constexpr auto collect_proper_labels(ast)
{
  return sort_labels(collect_labels_helper(ast{}));
}
}
}
}
