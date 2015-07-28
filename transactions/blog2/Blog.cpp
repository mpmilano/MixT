#include "FileStore.hpp"
#include "Transaction.hpp"

struct Comment {
	const int timestamp;
	const std::string content;
};

struct BlogEntry {
	const std::string content;
	Handle<Level::causal,HandleAccess::all,std::set<Comment> > comments;

	auto postComment(const std::string &cmnt){
		Comment c{gensym(),cmnt};
		comments.op(Insert,c);
	}
};

template<typename Store>
struct Blog {
	Store s;
	Handle<Level::causal, HandleAccess::all,std::set<BlogEntry> > entries;

	auto postEntry(const std::string &entry){
		BlogEntry bo{entry,s.newCollection<HandleAccess::all,Comment>()};
		entries.op(Insert,bo);
	}
};

int main(){
	FileStore<Level::causal> store;
	Blog<FileStore<Level::causal> > b{store,
			causalStore.newCollection<HandleAccess::all,BlogEntry>()};
	b.postEntry("hi!");
}
