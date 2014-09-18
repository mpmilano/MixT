#include "deptrack.h"

template<typename T>
void ignore(T){}

int truncate(double d){ return d;}

class banking {

//banking example

public:
	
	TranVals(int) b_balance;

	TranVals(int, IDof(b_balance) ) a_balance;

	banking():b_balance(100),a_balance(100){}


	template<Tracking::TrackingSet s>
		void applyInterest(IntermVal<double,s> v){
		a_balance.w.add(v.f(truncate));
	}
	
	void calcInterest(){
		applyInterest(a_balance.r * 0.3);
	}

	//note - do we even need to touch intermediate values used in the computation?
	//we're going to touch any write-sources, and we're not allowing any values to 
	//escape the if-statement except through write-sources.

	void withDraw(){
		TIF((a_balance.r > 50),
		     {
			     auto tmp = a_balance.r - 50;
			     a_balance.w.put(tmp);
		     },
		     {
			     //compiler will warn if we don't
			     //use available handles in both branches
			     ignore(a_balance.w);
		     },
		     a_balance);
	}

};


int main(){

	banking().calcInterest();

}
