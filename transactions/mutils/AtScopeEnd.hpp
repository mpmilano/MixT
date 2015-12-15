#pragma once
#include <memory>
#include <functional>

namespace mutils{

struct AtScopeEnd {
private:
	std::unique_ptr<std::function<void ()> > doit;
public:
	AtScopeEnd(const AtScopeEnd&) = delete;
	
	AtScopeEnd(AtScopeEnd&& e):doit(std::move(e.doit)){}

	bool assert_this() const {return true;}
	
	AtScopeEnd(const std::function<void ()> &di):doit(new std::function<void ()>(di)){}
	virtual ~AtScopeEnd() {
		(*doit)();
	}
};

}
