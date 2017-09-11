#define BEGIN_SPLIT_CONTEXT(name) template<typename l> struct name {\
																																		\
		template <typename l2, typename y, typename v, typename e>			\
		using Binding = typename AST<l>::template Binding<l2, y, v, e>;	\
		template<typename y, typename e>																\
		using Expression = typename AST<l>::template Expression<y, e>;			\
		template<typename s, typename f> using FieldReference = typename AST<l>::template FieldReference<s, f>;	\
		template <typename v>																								\
		using VarReference = typename AST<l>::template VarReference<v>;			\
		template <int i>																										\
		using Constant = typename AST<l>::template Constant<i>;							\
		using GenerateTombstone = typename AST<l>::template GenerateTombstone<>; \
		template <char op, typename L, typename R>													\
		using BinOp = typename AST<l>::template BinOp<op, L, R>;						\
		template <typename b>																								\
		using Statement = typename AST<l>::template Statement<b>;						\
		template <typename b, typename _body>																\
		using Let = typename AST<l>::template Let<b, _body>;								\
		template <typename b, typename _body>																\
		using LetRemote = typename AST<l>::template LetRemote<b, _body>;		\
		template <typename h>																								\
		using IsValid = typename AST<l>::template IsValid<h>;								\
		template <typename oper_name, typename Hndl, typename... args>			\
		using Operation = typename AST<l>::template Operation<oper_name,Hndl,args...>; \
		template <typename L, typename R>																		\
		using Assignment = typename AST<l>::template Assignment<L, R>;			\
		template <typename R>																								\
		using Return = typename AST<l>::template Return<R>;									\
		template <typename R>																								\
		using AccompanyWrite = typename AST<l>::template AccompanyWrite<R>;	\
		template <typename e>																								\
		using WriteTombstone = typename AST<l>::template WriteTombstone<e>;	\
		template <typename var>																							\
		using IncrementOccurance = typename AST<l>::template IncrementOccurance<var>; \
		template <typename var>																							\
		using IncrementRemoteOccurance = typename AST<l>::template IncrementRemoteOccurance<var>; \
		template <typename var>																							\
		using RefreshRemoteOccurance = typename AST<l>::template RefreshRemoteOccurance<var>;	\
		template <typename c, typename t, typename e>												\
		using If = typename AST<l>::template If<c, t, e>;										\
		template <typename c, typename t, char... name>											\
		using While = typename AST<l>::template While<c, t, name...>;				\
		template <typename t, char... name>																	\
		using ForEach = typename AST<l>::template ForEach<t, name...>;			\
		template <typename... Seq>																					\
		using Sequence = typename AST<l>::template Sequence<Seq...>;				\


#define END_SPLIT_CONTEXT };
