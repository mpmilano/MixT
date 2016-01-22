#pragma once
#include "Operation.hpp"
#include "Transaction.hpp"

//The Eiger underlying causal Store



struct Eiger{

	template<typename T>
	class Obj : public RemoteObject<T>{
		T local_cached;
		bool cache_valid = false;
	public:
		const T& get() const {
			if (cache_valid) return local_cached;
			else if (current_transaction){
				throw CannotProceedError();
			}
			//do some underlying eiger-specific fetch.
		}

		void put(const T& t) {
			if (current_transaction){
				local_cached = t;
				cache_valid = true;
			}
			//do some underlying eiger-specific store.
		}
	};

	class Update {
		auto operator()(){
		}
	};

	class Trans{
		void addUpdate(Update up);
	};

	static Trans* current_transaction = nullptr;
	
};

//TODO: dynamic dispatch into operations. This is important.
template<typename T>
OPERATION(Insert, Eiger::Obj<T>* set, const T& obj){
	Update up; //do something
	if (Eiger::current_transaction){
		current_transaction->addUpdate(up);
		//try update local temporary with this operation;
		//if we cannot, next read from local-temporary view of this
		//object will cause the transaction to fail.
	}
	else up();
	return true;
}
