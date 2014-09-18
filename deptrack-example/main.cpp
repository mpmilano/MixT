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


	template<Tracking::TrackingSet s, TVparams>
	void applyInterest(TransValsA balance, IntermVal<double,s> v){
		balance.add(v.f(truncate));
	}
	
	template<TVparams>
	void calcInterest(TransValsA balance){
		applyInterest(balance, balance * 0.3);
	}

	//note - do we even need to touch intermediate values used in the computation?
	//we're going to touch any write-sources, and we're not allowing any values to 
	//escape the if-statement except through write-sources.

	template<TVparams>
	void withDraw(TransValsA balance){
		TIF((balance > 50),
		     {
			     auto tmp = balance - 50;
			     balance.put(tmp);
		     },
		     {
			     //compiler will warn if we don't
			     //use available handles in both branches
			     ignore(balance);
		     },
		     balance);
	}

};


int main(){
	
	auto tmp = banking();
	tmp.calcInterest(tmp.a_balance);

}
