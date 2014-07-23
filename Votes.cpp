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
	int VoteTracker::getCount(Candidate cnd){
		return ds.get(votes[(int) cnd]);
	}
	VoteTracker::counts VoteTracker::currentTally(){
		return counts(0,0,0,0,0);
		/*
		DataStore::Handle<Level::causal,
				  DataStore::HandleAccess::read,
				  int> 
			votes_[(int)Candidate::Count] =
			{ds.newConsistency<Level::causal> (votes[0]),
			 ds.newConsistency<Level::causal> (votes[1]),
			 ds.newConsistency<Level::causal> (votes[2]),
			 ds.newConsistency<Level::causal> (votes[3]),
			 ds.newConsistency<Level::causal> (votes[4]),
			};
		
		auto transaction = [&](DataStore::Handle<votes_[0].level, votes_[0].ha,int> ){
			auto interim =  counts(
				ds.get(votes_[0]),
				ds.get(votes_[1]),
				ds.get(votes_[2]),
				ds.get(votes_[3]),
				ds.get(votes_[4]));
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

		return ds.ro_transaction(transaction,votes_[0]); 
//*/
	}

	VoteTracker::counts VoteTracker::FinalTally(){
		typedef DataStore::Handle<votes[0].level, votes[0].ha, int> hndl;
		auto transaction = [](DataStore &ds, hndl v0, hndl v1, hndl v2, hndl v3, hndl v4) {
			return counts(
				ds.get(v0),
				ds.get(v1),
				ds.get(v2),
				ds.get(v3),
				ds.get(v4));
		};
			return ds.ro_transaction(transaction,ds,votes[0],votes[1],votes[2],votes[3],votes[4]);
		
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
	v.getCount(election::Candidate::Ross);
	v.countVote(election::Candidate::Andrew);
	v.countVote(election::Candidate::Ross);
	v.countVote(election::Candidate::Nate);
	v.countVote(election::Candidate::Ross);
	std::cout << v.FinalTally() << std::endl;
}
