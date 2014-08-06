#pragma once
		template<Level L, typename T, HandleAccess HA>
		typename std::enable_if<canRead(HA), T&>::type
		get(DataStore::Handle<cid,L, HA, T> &hndl) {
			waitForSync<L>(); return hndl.hi();
		}

		
		template<Level L, typename T, HandleAccess HA>
		typename std::enable_if<canWrite(HA), void>::type
		give(DataStore::Handle<cid, L, HA, T> &hndl, std::unique_ptr<T> obj) {
			pending_updates.run([&] (typename pending::push_f &push) {
					std::shared_ptr<T> cpy(new T(*obj),release_deleter<T>());
					push([hndl,cpy](){
							auto &dltr = std::get_deleter<release_deleter<T> >(cpy);
							assert(!dltr->released());
							dltr->release();
							hndl.hi() = std::unique_ptr<T>(cpy.get());});	
				});
			hndl.hi() = std::move(obj);
			waitForSync<L>();
		}
		
		template<Level L, typename T, HandleAccess HA>
		typename std::enable_if<canWrite(HA), void>::type
		give(DataStore::Handle<cid, L, HA, T> &hndl, T* obj) {
			pending_updates.run([&] (typename pending::push_f &push ){
					std::shared_ptr<T> cpy(new T(*obj),release_deleter<T>());
					upfun &&tmp = [hndl,cpy](){
						std::get_deleter<release_deleter<T> >(cpy)->release();
						hndl.hi() = std::unique_ptr<T>(cpy.get());};
					push(tmp);
				});
			hndl.hi() = std::unique_ptr<T>(obj);
			waitForSync<L>();
		}
		
		template<Level L, typename T>
		std::unique_ptr<T> take(DataStore::Handle<cid, L,HandleAccess::all,T>& hndl) {
			waitForSync<L>();
			pending_updates.run([&hndl](typename pending::push_f &push) {
					push([hndl](){hndl.hi().reset();});});
			return hndl.hi();
		}
