#pragma once
#include "Client.hpp"
#include <string>

namespace election{

	enum class Candidate : int 
	{Andrew, Nate, Ross, Dexter, ConstaBob, Count};


	class fake_int{
	public:
		int i = 0;
		operator int(){return i;}
		fake_int(int i):i(i){//std::cout << "building" << std::endl;
		}
			fake_int(const fake_int& fi):i(fi.i) {//std::cout << "copying!" << std::endl;
			}
		auto operator ++(int) {return i++;}

		~fake_int(){//std::cout << "oh no!" << std::endl;
		}

	};

	typedef fake_int dType;

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

	typedef backend::DataStore::Handle<0,backend::Level::strong, backend::HandleAccess::all, dType> VoteH_primary;
	typedef backend::DataStore::Handle<1,backend::Level::strong, backend::HandleAccess::all, dType> VoteH_secondary;


	class VoteTrackerServer;

	class VoteTrackerClient {

	private : 
		typedef VoteH_secondary VoteH;
		backend::Client<1> ds;
		VoteTrackerClient(VoteTrackerServer&);
		std::array<std::unique_ptr<VoteH>, (int)Candidate::Count > votes;
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
		typedef VoteH_primary VoteH;
		backend::DataStore& upstream_ds;
		backend::Client<0> ds;
		std::array<VoteH, (int)Candidate::Count > votes;

	public:

		VoteTrackerServer(backend::DataStore&);
		VoteTrackerClient spawnClient();

		friend class VoteTrackerClient;

	};
	

}
