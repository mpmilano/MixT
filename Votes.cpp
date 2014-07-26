#include "Votes.h"
#include <iostream>

namespace election{

	using namespace backend;

//counts order: Andrew, Nate, Ross, Dexter, ConstaBob

	VoteTracker::VoteTracker(backend::DataStore& ds):
		ds(ds),
		votes({ds.newHandle<Level::strong>(8),
					ds.newHandle<Level::strong>(0),
					ds.newHandle<Level::strong>(0),
					ds.newHandle<Level::strong>(0),
					ds.newHandle<Level::strong>(0),})
	{}
	
	void VoteTracker::countVote(Candidate cnd){
		ds.incr_op(votes[(int) cnd]);
	}

	void VoteTracker::voteForTwo(Candidate cnd1, Candidate cnd2){

		typedef DataStore::Handle<votes[0].level, HandleAccess::write, int> hndl;

		assert (cnd1 != cnd2);
		auto transaction = [](DataStore& ds, hndl cnd1, hndl cnd2){
			ds.incr_op(cnd1);
			ds.incr_op(cnd2);
		};
		ds.wo_transaction(transaction, ds.wo_hndl(votes[(int) cnd1]),ds.wo_hndl( votes[(int) cnd2]));
	}

	int VoteTracker::getCount(Candidate cnd){
		return ds.get(votes[(int) cnd]);
	}
	VoteTracker::counts VoteTracker::currentTally(){

		typedef DataStore::Handle<Level::causal, HandleAccess::read, int> hndl;
		
		auto transaction = [](DataStore &ds, hndl v0, hndl v1, hndl v2, hndl v3, hndl v4) {
			auto interim =  counts(
				ds.get(v0),
				ds.get(v1),
				ds.get(v2),
				ds.get(v3),
				ds.get(v4));
			auto total = 
			interim.andrew + 
			interim.ross + 
			interim.nate + 
			interim.dexter + 
			interim.constabob;
			return counts((interim.andrew * 100) / total,
				      (interim.nate * 100) / total,
				      (interim.ross * 100) / total,
				      (interim.dexter * 100) / total,
				      (interim.constabob * 100) / total,
				      true);};

		return ds.ro_transaction(transaction,
					 ds.newConsistency<Level::causal> (votes[0]),
					 ds.newConsistency<Level::causal> (votes[1]),
					 ds.newConsistency<Level::causal> (votes[2]),
					 ds.newConsistency<Level::causal> (votes[3]),
					 ds.newConsistency<Level::causal> (votes[4])); 
//*/
	}

	VoteTracker::counts VoteTracker::FinalTally(){
		typedef DataStore::Handle<votes[0].level, HandleAccess::read, int> hndl;
		auto transaction = [](DataStore &ds, hndl v0, hndl v1, hndl v2, hndl v3, hndl v4) {
			return counts(
				ds.get(v0),
				ds.get(v1),
				ds.get(v2),
				ds.get(v3),
				ds.get(v4));
		};
		return ds.ro_transaction(transaction,
					 ds.ro_hndl(votes[0]),
					 ds.ro_hndl(votes[1]),
					 ds.ro_hndl(votes[2]),
					 ds.ro_hndl(votes[3]),
					 ds.ro_hndl(votes[4]));
	}

}

int main (){
	backend::DataStore ds;
	election::VoteTracker v(ds);
	v.FinalTally();
	v.countVote(election::Candidate::Ross);
	v.countVote(election::Candidate::Dexter);
	v.countVote(election::Candidate::Nate);
	v.countVote(election::Candidate::Ross);
	v.countVote(election::Candidate::Andrew);
	std:: cout << v.currentTally() << std::endl;
	v.countVote(election::Candidate::Ross);
	v.countVote(election::Candidate::ConstaBob);
	v.countVote(election::Candidate::Ross);
	v.voteForTwo(election::Candidate::Ross, election::Candidate::ConstaBob);
	v.getCount(election::Candidate::Ross);
	v.countVote(election::Candidate::Andrew);
	v.countVote(election::Candidate::Ross);
	v.countVote(election::Candidate::Nate);
	v.countVote(election::Candidate::Ross);
	std::cout << v.FinalTally() << std::endl;
}
