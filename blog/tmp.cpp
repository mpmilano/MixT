#include <future>
#include "Blog.h"

using namespace std;
using namespace backend;

void commenter_1(Blog &&b){
	string init = "I really like this place!";
	b.cmnt.postComment(init);
	auto response = b.cmnt.currentComments().back();
	while (response.display() == init) response = b.cmnt.currentComments().back();
	b.cmnt.postComment("nope. not even a little bit.");
}

void commenter_2(Blog &&b){
	while (b.cmnt.currentComments().size() == 0) continue;
	b.cmnt.postComment("first post!");
}

void blogger(Blog &&b){
	b.post.postUpdate("Welcome to AMA!");
	while (b.cmnt.currentComments().size() == 0) continue;
	b.post.postUpdate("seriously guys?");
}

int main() {
	DataStore ds;
	Blog b(ds);
	std::async(std::launch::async,[&](){blogger(b.spawn());});
	std::async(std::launch::async,[&](){commenter_1(b.spawn());});
	std::async(std::launch::async,[&](){commenter_2(b.spawn());});
}
