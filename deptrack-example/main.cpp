#include "deptrack.h"

template<typename T>
void ignore(T){}

int truncate(double d){ return d;}

class banking {

//banking example

public:
	
	TranVals(int,b_balance);

	TranVals(int,a_balance, IDof(b_balance_R) );


	template<Tracking::TrackingSet s>
		void applyInterest(IntermVal<double,s> v){
		a_balance_W.add(v.f(truncate));
	}
	
	void calcInterest(){
		applyInterest(a_balance_R * 0.3);
	}

	//note - do we even need to touch intermediate values used in the computation?
	//we're going to touch any write-sources, and we're not allowing any values to 
	//escape the if-statement except through write-sources.

	void withDraw(){
		TIF2((a_balance_R > 50),
		     {
			     auto tmp = a_balance_R - 50;
			     a_balance_W.put(tmp);
		     },
		     {
			     //compiler will warn if we don't
			     //use available handles in both branches
			     ignore(a_balance_W);
		     },
		     a_balance_R, a_balance_W);
	}

};

int main(){

	banking().calcInterest();

}
