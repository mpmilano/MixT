//an efficient, functional, tree-base set implementation.  I hope all of those words apply, at least.
#include<memory>

template<typename T>
class Set {

private:
	typedef const T Elem;
	typedef std::shared_ptr<Elem> Elem_p;

	//invariant; left-tree is always at same height or one shorter than right-tree.
	struct SetNode {
		typedef std::shared_ptr<const SetNode> SetNode_p;
		
		const Elem_p member;
		const SetNode_p left;
		const SetNode_p right;
		const SetNode_p& this_p;
		const bool _balanced;

		static bool balanced(const SetNode_p &sn){
			if (sn) return sn->_balanced;
			else return true;
		}
		
		static SetNode_p create(Elem_p &e, SetNode_p &l, SetNode_p &r){
			SetNode_p ret;
			assert(!l ? r && (!r->left) && !r->right : true);
			ret.reset(new SetNode{e,l,r,ret,(l ? l->balanced : false)});
			return ret;
		}

		static SetNode_p insert(SetNode_p here, Elem_p new_member) {
			if (here) return here->insert(new_member);
			else return create(new_member, nullptr, nullptr, true);
		}

		
		SetNode_p insert(Elem_p new_member) const {
			if (new_member == *this_p) return this_p;
			else if (new_member < *this)
				return create(new_member,right,insert(left,member));
			else if (*this < new_member)
				return create(member,right,insert(left,new_member));
		}

		bool is_here(){
		}
		
		auto operator==(const Elem_p &n) const {
			return (*member) == (*n);
		}
		
		auto operator<(const Elem_p &n) const {
			return (*member) < *(n);
		}
	};

	const SetNode sn;
public:
	
	Set<T> insert 
};
