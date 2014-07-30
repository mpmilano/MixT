#pragma once
#include "Backend.hpp"
#include <string>

namespace election{

	enum class Candidate : int 
	{Andrew, Nate, Ross, Dexter, ConstaBob, Count};

	class counts{
	public:
		const int andrew;
		const int nate;
		const int ross; 
		const int dexter; 
		const int constabob;
		
	private: 
		bool percents = false;
		std::string annot() const 
		{ return percents ? "%" : ""; } 
		
	public:
		friend std::ostream& operator<<
			(std::ostream& s, const counts& c){
			s << "Andrew: " << c.andrew<< c.annot() 
			  << " Nate: " << c.nate <<c.annot() 
			  << " Ross: " << c.ross<< c.annot() 
			  << " Dexter: " << c.dexter << c.annot() 
			  << " ConstaBob: " << c.constabob<< c.annot();
			return s;
		}

	counts(int andrew, 
	       int nate, 
	       int ross, 
	       int dexter, 
	       int constabob,
	       bool percents = false):
		andrew(andrew),
			nate(nate),
			ross(ross),
			dexter(dexter),
			constabob(constabob),
			percents(percents)
			{}
	};

	typedef backend::DataStore::Handle<backend::Level::strong, backend::HandleAccess::all, int> VoteH;


	class VoteTrackerServer;

	class VoteTrackerClient {

	private : 
		backend::Client ds;
		VoteTrackerClient(VoteTrackerServer&);
		std::array<VoteH, (int)Candidate::Count > votes;
	public :

		void countVote(Candidate);
		void voteForTwo(Candidate, Candidate);
		int getCount(Candidate);
		counts currentTally();
		counts FinalTally();

		VoteTrackerClient(VoteTrackerClient&&);

		friend class VoteTrackerServer;
			
	};


	class VoteTrackerServer{
	public:

	private:
		backend::DataStore& upstream_ds;
		backend::Client ds;
		std::array<VoteH, (int)Candidate::Count > votes;

	public:

		VoteTrackerServer(backend::DataStore&);
		VoteTrackerClient spawnClient();

		friend class VoteTrackerClient;

	};
	

}
