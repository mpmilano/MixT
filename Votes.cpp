#include "Votes.h"
#include <iostream>

namespace election{

	using namespace backend;
//counts order: Andrew, Nate, Ross, Dexter, ConstaBob

	VoteTracker::VoteTracker(backend::DataStore& ds):
		ds(ds),
		votes({ds.newHandle<Level::causal>(0),
					ds.newHandle<Level::causal>(0),
					ds.newHandle<Level::causal>(0),
					ds.newHandle<Level::causal>(0),
					ds.newHandle<Level::causal>(0),})
	{}
	
	void VoteTracker::countVote(Candidate cnd){
		ds.incr_op(votes[(int) cnd]);
	}	
	int VoteTracker::getCount(Candidate cnd){
		return ds.get(votes[(int) cnd]);
	}
	VoteTracker::counts VoteTracker::currentTally(){
		return counts(
			ds.get(votes[0]),
			ds.get(votes[1]),
			ds.get(votes[2]),
			ds.get(votes[3]),
			ds.get(votes[4]));
	}

	VoteTracker::counts VoteTracker::FinalTally(){
		return counts(
			ds.get<Level::strong>(votes[0]),
			ds.get<Level::strong>(votes[1]),
			ds.get<Level::strong>(votes[2]),
			ds.get<Level::strong>(votes[3]),
			ds.get<Level::strong>(votes[4]));
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
	v.currentTally();
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
