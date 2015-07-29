#include "FileStore.hpp"
#include "Transaction.hpp"

struct Comment {
	const int timestamp;
	const std::string content;

	bool operator<(const Comment& be) const {
		return timestamp < be.timestamp;
	}
};


struct BlogEntry {
	const std::string content;
	Handle<Level::causal,HandleAccess::all,std::set<Comment> > comments;
	const int timestamp = gensym();

	auto postComment(const std::string &cmnt){
		Comment c{gensym(),cmnt};
		comments.op(Insert,c);
	}

	bool operator<(const BlogEntry& be) const {
		return timestamp < be.timestamp;
	}
};

template<typename Store>
struct Blog {
	Store s;
	Handle<Level::strong, HandleAccess::all,std::set<BlogEntry> > entries;

	auto postEntry(const std::string &entry){
		auto col = s.template newCollection<HandleAccess::all,Comment>();
		BlogEntry bo{entry,col};
		entries.op(Insert,bo);
	}
};

int main(){
	FileStore<Level::causal> store;
	FileStore<Level::strong> sstore;
	Blog<FileStore<Level::causal> > b{store,
			sstore.template newCollection<HandleAccess::all,BlogEntry>()};
	b.postEntry("hi!");
}
