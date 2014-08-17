#include "Votes.h"
#include <iostream>

namespace election{

	using namespace backend;

//counts order: Andrew, Nate, Ross, Dexter, ConstaBob

	VoteTrackerServer::VoteTrackerServer(backend::DataStore& _ds):
		upstream_ds(_ds),
		ds(_ds),
		votes(make_array_u(ds.newHandle<Level::strong>(new fake_int(8)),
				   ds.newHandle<Level::strong>(new fake_int(0)),
				   ds.newHandle<Level::strong>(new fake_int(0)),
				   ds.newHandle<Level::strong>(new fake_int(0)),
				   ds.newHandle<Level::strong>(new fake_int(0)))) {}

	
	void VoteTrackerClient::countVote(Candidate cnd){
		ds.incr_op(*votes[(int) cnd]);
	}

	void VoteTrackerClient::voteForTwo(Candidate cnd1, Candidate cnd2){
		std::cout << "danger!" << std::endl;

		typedef DataStore::Handle<1,VoteH::level, HandleAccess::write, dType> hndl;

		assert (cnd1 != cnd2);
		auto transaction = [](Client<1>& ds, hndl cnd1, hndl cnd2){
			ds.incr_op(cnd1);
			ds.incr_op(cnd2);
		};
		ds.transaction().wo(transaction, ds.wo_hndl(*votes[(int) cnd1]),ds.wo_hndl( *votes[(int) cnd2]));
		std::cout << "passed." << std::endl;
	}

	int VoteTrackerClient::getCount(Candidate cnd){
		return ds.get(*votes[(int) cnd]);
	}
	counts VoteTrackerClient::currentTally(){

		typedef DataStore::Handle<1,Level::causal, HandleAccess::read, dType> hndl;
		
		auto transaction = [](Client<1> &ds, hndl v0, hndl v1, hndl v2, hndl v3, hndl v4) {
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

		return ds.transaction().ro(transaction,
					 ds.newConsistency<Level::causal> (*votes[0]),
					 ds.newConsistency<Level::causal> (*votes[1]),
					 ds.newConsistency<Level::causal> (*votes[2]),
					 ds.newConsistency<Level::causal> (*votes[3]),
					 ds.newConsistency<Level::causal> (*votes[4])); 
//*/
	}

	counts VoteTrackerClient::FinalTally(){
		typedef DataStore::Handle<1, VoteH::level, HandleAccess::read, dType> hndl;
		auto transaction = [](Client<1> &ds, hndl v0, hndl v1, hndl v2, hndl v3, hndl v4) {
			return counts(
				ds.get(v0),
				ds.get(v1),
				ds.get(v2),
				ds.get(v3),
				ds.get(v4));
		};
		return ds.transaction().ro(transaction,
					 ds.ro_hndl(*votes[0]),
					 ds.ro_hndl(*votes[1]),
					 ds.ro_hndl(*votes[2]),
					 ds.ro_hndl(*votes[3]),
					 ds.ro_hndl(*votes[4]));
	}

	VoteTrackerClient VoteTrackerServer::spawnClient(){
		return VoteTrackerClient(*this);
	}

	VoteTrackerClient::VoteTrackerClient(VoteTrackerServer& s):ds(s.upstream_ds),votes(
		(make_array_u(ds.get_access(*s.votes[0],s.ds),
			      ds.get_access(*s.votes[1],s.ds),
			      ds.get_access(*s.votes[2],s.ds),
			      ds.get_access(*s.votes[3],s.ds),
			      ds.get_access(*s.votes[4],s.ds)))){}

	VoteTrackerClient::VoteTrackerClient(VoteTrackerClient&& s):ds(std::move(s.ds)),votes(std::move(s.votes)){}

}

int main (){

	{
		backend::DataStore ds;
		election::VoteTrackerServer vs(ds);
		auto v = vs.spawnClient();
		auto v2 = vs.spawnClient();
		v.FinalTally();
		v.countVote(election::Candidate::Ross);
		v2.countVote(election::Candidate::Dexter);
		v.countVote(election::Candidate::Nate);
		v2.countVote(election::Candidate::Ross);
		v.countVote(election::Candidate::Andrew);
		std:: cout << v.currentTally() << std::endl;
		v.countVote(election::Candidate::Ross);
		v.countVote(election::Candidate::ConstaBob);
		v.countVote(election::Candidate::Ross);
		v.getCount(election::Candidate::Ross);
		v.voteForTwo(election::Candidate::Ross, election::Candidate::ConstaBob);
		v.getCount(election::Candidate::Ross);
		v.countVote(election::Candidate::Andrew);
		v.countVote(election::Candidate::Ross);
		v.countVote(election::Candidate::Nate);
		v.countVote(election::Candidate::Ross);
		v2.FinalTally();
		std::cout << v.FinalTally() << std::endl;
		std::cout << v2.FinalTally() << std::endl;
		std::cout << "done" << std::endl;
		assert(v.getCount(election::Candidate::Ross) == 7);
		assert(v.getCount(election::Candidate::Nate) == 2);
		assert(v.getCount(election::Candidate::Andrew) == 10);
		assert(v.getCount(election::Candidate::Dexter) == 1);
		assert(v.getCount(election::Candidate::ConstaBob) == 2);
	}
}
