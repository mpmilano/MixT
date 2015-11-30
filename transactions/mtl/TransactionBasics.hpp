#pragma once

namespace myria {
	
	struct GDataStore;
	namespace mtl {



		struct TransactionContext {

			virtual bool commit() = 0;
			virtual GDataStore& store() = 0 ;
			virtual ~TransactionContext(){}
	
		};


	} }
