#pragma once

namespace backend{

	typedef std::function<void ()> upfun;

	class pending{
	private:
		bool locked = false;
		std::list<upfun> pending_updates;
		void push(upfun up){
			if (!locked) pending_updates.push_back(std::move(up));
		}
		std::function<void (upfun&) > push_ = [&](upfun &f){ push(f);};
	
	public:
		pending(const pending&) = delete;
		pending(pending &&p):locked(p.locked),
				     pending_updates(std::move(p.pending_updates)),
				     push_(std::move(p.push_)){}
		pending(){}
		void runAndClear(){
			if (!locked){
				auto l = lock();
				for (auto &up : pending_updates) up();
				pending_updates.clear();
			}
		}
		
		bool isClear(){
			return pending_updates.size() == 0;
		}
		
		typedef std::function<void (upfun&)> push_f;
		
		void run(std::function<void (push_f&) > &&f){
			if (!locked) f(push_);
		}
		
		class pending_lock{
			
		private:
			pending& cl;
			pending_lock(pending& cl):cl(cl){
				cl.locked = true;
			}
		public:
			~pending_lock(){
				cl.locked = false;
			}
			friend class pending;
		};
		friend class pending_lock;
		pending_lock lock(){return pending_lock(*this);}
	};
}
