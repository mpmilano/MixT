#pragma once

/*
  Example: Chat server. Users, rooms.  Room lists are linearizable; Room membership is linearizable.  Each user has an inbox, and each room also maintains a list of posts.  Posts, and the delivery thereof, are causal. When a user posts to a room, it's copied to each room-member in a single transaction.  This is a hybrid transaction; you need to read-validate the room-list at the end, and you also need to make sure the post shows up in all or none of the inboxes.  Users can't delete posts they make (though they can delete their own local things from their inbox), and users can join and leave rooms whenever. 

*/


//other examples: wiki edites?
//chat bigger example: pin messages is endorsement. Maybe for moderator elections?

//endorsements: include an endorsement that does a quorum read, and another which doesn't, for data-dependant things (like computation warranties use case).

#include <set>
#include <sstream>
#include "MTL.hpp"
#include "SQLStore.hpp"
//#include "TrackerTestingStore.hpp"
#include "RemoteCons.hpp"
#include "Ostreams.hpp"
#include "Operations.hpp"
#include "FinalHeader.hpp"//*/
#include "SerializationMacros.hpp"
#include "Transaction_macros.hpp"

namespace myria{

template<typename p>
using newObject_f = std::function<p (const typename p::stored_type&)>;

#define default_build 	template<typename... T>					\
	static p mke( newObject_f<p> store_alloc, const T&... a){	\
		typename p::stored_type ret{a...};						\
		return store_alloc(ret);								\
	}

template<Level l, typename T2>
struct remote_set {
	using p = Handle<l,HandleAccess::all, std::set<T2>,
					 SupportedOperation<RegisteredOperations::Insert,void,SelfType,T2> >;
	default_build
};

	struct post : public mutils::ByteRepresentable{
	using p = Handle<Level::causal, HandleAccess::all, post>;
	std::string str;
	post(const std::string &s):str(s){}

	default_build
        DEFAULT_SERIALIZATION_SUPPORT(post,str);
};

struct user : public mutils::ByteRepresentable {
	using p = Handle<Level::causal, HandleAccess::all, user>;
    using inbox_p = typename remote_set<Level::causal, post::p>::p;
    using inbox_t = remote_set<Level::causal, post::p>;
        inbox_p inbox;
	user(const decltype(inbox) &i):inbox(i){}
	
	default_build
	DEFAULT_SERIALIZATION_SUPPORT(user,inbox);


/*        std::set<post::p> posts(std::unique_ptr<VMObjectLog> &log, tracker::Tracker &trk, Strong &strong, Causal &causal) {
                auto trans = start_transaction(log,trk,strong,causal);
                trans->commit_on_delete = true;
                return inbox.get(trans->trackignContext->trk,trans.get());
				}//*/

	static void posts_p(std::unique_ptr<VMObjectLog> &log, tracker::Tracker &trk, p hndl){
		TRANSACTION(log,trk,hndl,
					let_remote(usr) = hndl IN (
						let_remote(theset) = $(usr,inbox) IN (mtl_ignore(theset))
						)
			);
	}
	
};

/*std::ostream& operator<<(std::ostream &os, const user& u){
  return os << "user with inbox: " << u.inbox;
  }
*/

using MemberList = RemoteCons<user,Level::strong,Level::causal>;

struct room : public mutils::ByteRepresentable{

	using p = Handle<Level::strong, HandleAccess::all, room>;
	MemberList::p members;

	room(newObject_f<MemberList::p> mf)
		:members(MemberList::mke(mf)) {}
	
	room(const decltype(members) &m)
		:members(m){}
	
	default_build
	DEFAULT_SERIALIZATION_SUPPORT(room,members)
	
		void add_post(std::unique_ptr<VMObjectLog> &log, tracker::Tracker& trk, post::p pst){
		struct params {
			post::p pst; MemberList::p members;
		};
		params env_param{pst,members};
		TRANSACTION(log,trk,env_param,
					let(hd) = $(env_param,members) IN ( 
						WHILE (isValid(hd)) DO (
							let_remote(tmp) = hd IN (
								let_remote(tmp2) = $(tmp,val) IN (
									mtl_ignore(do_op(Insert,$(tmp2,inbox),$(env_param,pst)))),
								hd = $(tmp,next)
								)
							)
						)
			); //*/
	}
	
	void add_member(std::unique_ptr<VMObjectLog> &log, tracker::Tracker& trk, user::p usr){
		struct params_t {MemberList::p members; user::p usr;};
		params_t params{members,usr};
		TRANSACTION(log,trk,params,
					$(params,members) = $bld(MemberList, $(params,usr), do_op(Clone,$(params,members)))
			);
	}
};

struct groups {
	using p = typename remote_set<Level::causal, room::p>::p;
	default_build
};

}

