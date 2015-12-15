#include "FutureFreePool.hpp"
#include <list>
#include "AtScopeEnd.hpp"

using namespace std;

namespace mutils{

	namespace{
		template<typename T> void irritate(const T&){}
	}

	int FutureFreePool_impl::get_pos(){
		int pos = -1;
		{
			FutureFreePool_impl::lock{m};
				pos = available_ids.front();
				available_ids.pop_front();
		}
		assert(pos > -1);
		return pos;
	}
	
	void FutureFreePool_impl::launch (function<void (int)> fun){
		auto pos = get_pos();
		auto this_p = this->this_p;
		AtScopeEnd ase{[pos,this_p](){
				lock{this_p->m};
				this_p->available_ids.push_back(pos);
			}};
		pending[pos] = internal_pool.push([fun,ase2 = std::move(ase)](int i){
				fun(i);
				assert(ase2.assert_this());
			});
	}
}
