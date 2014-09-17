#include "deptrack.h"



namespace banking{

//banking example

	ReadVal(int) b_balance_R(100);
	ReadVal(int) a_balance_R(100);
	WriteVal<int,a_balance_R.id()> a_balance_W;

	int truncate(double d){ return d;}
	
	template<Tracking::TrackingSet s>
		void applyInterest(IntermVal<double,s> v){
		a_balance_W.add(v.f(truncate));
	}
	
	void calcInterest(){
		applyInterest(a_balance_R * 0.3);
	}

	void withDraw(){
		TIF2((a_balance_R > 50),
		     {
			     auto tmp = a_balance_R - 50;
			     //a_balance_W.put(tmp);
		     },
		     {},
		     a_balance_R,b_balance_R); //, a_balance_W);
	}

}

int main(){

	banking::calcInterest();

}
