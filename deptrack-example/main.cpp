#include <memory>
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

constexpr Tracking::TrackingId insert_id = gen_id();

template<typename T>
class ListNode {
public:
	TranVals(T) value;
	TranVals(std::unique_ptr<ListNode<T> >, IDof(value),insert_id ) next;
	
	ListNode(T value):value(std::move(value)),next(nullptr){}

};

template<typename T, typename... Args>
std::unique_ptr<T> unique_new (Args... a){ return std::unique_ptr<T>(new T(a...)); }

template<typename T>
class LinkedList {
public:
	TranVals(ListNode<T>, insert_id) hd;

	LinkedList(T value):hd(value){}

	template<Tracking::TrackingId s, Tracking::TrackingSet... set>
	void insert(T value, TransVals<ListNode<T>,s,set...> &prev){
		//local stuff
		TransVals<std::unique_ptr<ListNode<T> >, insert_id, IDof(prev)> curr(unique_new<ListNode<T> >(value));
		//transaction
		typedef typename std::decay<decltype(curr)>::type::t currt;
		typedef typename std::decay<decltype(prev)>::type::t prevt;
		curr.c([](currt& curr, prevt& prev){curr->next.put(std::move(prev.next));}, prev);
		prev.c([&](prevt& prev, currt&){prev.next.put(std::move(curr.i()));}, curr);
		} //*/

};


int main(){
	
	auto tmp = banking();
	tmp.calcInterest(tmp.a_balance);

	LinkedList<int> ll(22);
	ll.insert(12,ll.hd);
	//ll.hd.value.display();
	

}
