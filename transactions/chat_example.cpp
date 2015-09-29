/*
Example: Chat server. Users, rooms.  Room lists are linearizable; Room membership is linearizable.  Each user has an inbox, and each room also maintains a list of posts.  Posts, and the delivery thereof, are causal. When a user posts to a room, it's copied to each room-member in a single transaction.  This is a hybrid transaction; you need to read-validate the room-list at the end, and you also need to make sure the post shows up in all or none of the inboxes.  Users can't delete posts they make (though they can delete their own local things from their inbox), and users can join and leave rooms whenever. 

 */


//other examples: wiki edites?
//chat bigger example: pin messages is endorsement. Maybe for moderator elections?

//endorsements: include an endorsement that does a quorum read, and another which doesn't, for data-dependant things (like computation warranties use case).
#include <set>
#include "Transaction.hpp"
#include "Handle.hpp"
#include "SQLStore.hpp"
#include "RemoteCons.hpp"
#include "Operate_macros.hpp"
#include "FreeExpr_macros.hpp"

using namespace std;

template<typename p>
using newObject_f = p (*) (const typename p::stored_type&);

#define default_build 	template<typename... T>							\
	static p mke( newObject_f<p> store_alloc, const T&... a){			\
		typename p::stored_type ret{a...};								\
		return store_alloc(ret);										\
	}

template<Level l, typename T2>
struct remote_set {
	using p = Handle<l,HandleAccess::all, std::set<T2> >;
	default_build
};

struct post{
	using p = Handle<Level::causal, HandleAccess::all, post>;
	std::string str;
	default_build
};

struct user {
	using p = Handle<Level::strong, HandleAccess::all, user>;
	typename remote_set<Level::causal, post::p>::p inbox;
	default_build
};

using MemberList = RemoteCons<user,Level::strong,Level::causal>;

struct room{
	using p = Handle<Level::strong, HandleAccess::all, room>;
	MemberList::p members;
	typename remote_set<Level::causal, post::p>::p posts;

	room(newObject_f<MemberList::p> mf, newObject_f<decltype(posts)> pf)
		:members(MemberList::mke(mf)),
		 posts(remote_set<Level::causal, post::p>::mke(pf)) {}
	
	default_build
	
	void add_post(post::p pst){
		TRANSACTION(
			do_op(Insert,posts,$$(pst)),
			let_mutable(hd) = members IN ( 
				WHILE (isValid(hd)) DO (
					let_ifValid(tmp) = hd IN (
						do_op(Insert,$($(tmp,val),inbox),$$(pst)),
						hd = $(tmp,next)
						),
					) 
				)
			);
	}
	
	void add_member(user::p usr){
		TRANSACTION(
			members.put(MemberList{usr,members})
			);
	}
};

struct groups {
	using p = typename remote_set<Level::causal, room::p>::p;
	default_build
};

int main() {
	auto &strong = SQLStore::inst();
	auto &weak = FileStore<Level::causal>::filestore_instance();
	auto strong_f = [&](auto &e){return strong.template newObject<HandleAccess::all>(e);};
	auto weak_f = [&](auto &e){return weak.template newObject<HandleAccess::all>(e);};
	groups::p g = groups::mke(weak_f);
	room::p initial_room = room::mke(strong_f,strong_f,weak_f);
	
	TRANSACTION(do_op(Insert,g,initial_room));	
	
}
