#pragma once
#include "Backend.hpp"
#include <string>

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

		VoteTracker(backend::DataStore&);

		void countVote(Candidate);
		int getCount(Candidate);
		counts currentTally();
		counts FinalTally();
			
	};

}
