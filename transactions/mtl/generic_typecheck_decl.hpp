#pragma once
#if 0

namespace myria { namespace mtl  { namespace typecheck_phase {
		template<typename l, typename y, typename v, typename e>
		constexpr auto _label_inference(Binding<l,y,v,e>){
		}

		template<typename l, typename y, typename s, typename f>
		constexpr auto _label_inference(Expression<l,y,FieldReference<s,f> >){
		}

		template<typename l, typename y, typename v>
		constexpr auto _label_inference(Expression<l,y,VarReference<v> >){
		}

		template<typename l, int i>
		constexpr auto _label_inference(Expression<int ,y,Constant<i> >){
		}

		template<typename l, typename y, char op, typename L, typename R>
		constexpr auto _label_inference(Expression<l,y,BinOp<op,L,R> >){
		}
		
		template<typename l, typename b, typename e>
		constexpr auto _label_inference(Statement<l,Let<b,e> >){
		}

		template<typename l, typename b, typename e>
		constexpr auto _label_inference(Statement<l,LetRemote<b,e> >){
		}

		template<typename l, typename c, typename t, typename e>
		constexpr auto _label_inference(Statement<l,If<c,t,e> >){
		}

		template<typename l, typename c, typename e>
		constexpr auto _label_inference(Statement<l,While<c,e> >){
		}

		template<typename l, typename... seq>
		constexpr auto _label_inference(Statement<l,Sequence<seq...> >){
		}

		}}}

#endif
