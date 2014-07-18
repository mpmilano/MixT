#pragma once
#include "Backend.hpp"

namespace election{

	enum class Candidate : int 
	{Andrew, Nate, Ross, Dexter, ConstaBob, Count};

	class VoteTracker{
	public:
		typedef backend::DataStore::Handle
			<backend::Level::causal,int> VoteH;

	private:
		backend::DataStore& ds;
		VoteH votes[(int)Candidate::Count];
		
		

	public:
		class counts{
		public:
			const int andrew;
			const int nate;
			const int ross; 
			const int dexter; 
			const int constabob;
			friend std::ostream& operator<<
				(std::ostream& s, const counts& c){
				s << "Andrew: " << c.andrew
				  << " Nate: " <<c.nate 
				  << " Ross: " << c.ross
				  << " Dexter: " << c.dexter 
				  << " ConstaBob: " << c.constabob;
				return s;
			}
		counts(int andrew, 
		       int nate, 
		       int ross, 
		       int dexter, 
		       int constabob):
			andrew(andrew),
				nate(nate),
				ross(ross),
				dexter(dexter),
				constabob(constabob)
				{}
		};

		VoteTracker(backend::DataStore&);

		void countVote(Candidate);
		int getCount(Candidate);
		counts currentTally();
		counts FinalTally();
			
	};

}
