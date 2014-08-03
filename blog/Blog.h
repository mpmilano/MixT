#pragma once
#include <string>
#include "../Client.hpp"
#include "../Backend.hpp"

class Comments;
class Blog;

class Comment{
private:
	std::string s;
public:
	Comment(std::string s):s(std::move(s)){}
	std::string display(){return s;}
};

class Comments{
private:
	static constexpr backend::Client_Id cid = 0;
	static constexpr backend::Level level = backend::Level::causal;
	static constexpr backend::HandleAccess ha=backend::HandleAccess::all;
	typedef backend::DataStore::Handle <cid,level,ha,std::list<Comment> >
	CHandle;
	typedef backend::Client<cid> Client;
	Client &c;
	CHandle comments;
public:
	Comments(Client &c, CHandle h):
		c(c),
		comments(std::move(h)){}
	void postComment(Comment c);
	const std::list<Comment>& currentComments();
	friend class Blog;
};


class Post{
private:
	static constexpr backend::Client_Id cid = 0;
	static constexpr backend::Level level = backend::Level::strong;
	static constexpr backend::HandleAccess ha=backend::HandleAccess::all;
	typedef backend::DataStore::Handle <cid,level,ha,std::string > 
	PHandle;
	typedef backend::Client<cid> Client;
	Client &c;
	PHandle post;
public:
	Post(Client &c, PHandle p):c(c),post(std::move(p)){}
	void postUpdate(std::string);
	void replaceContent(std::string);
	friend class Blog;
};

class Blog{
private:
	static constexpr backend::Client_Id cid = 0;
	typedef backend::Client<cid> Client;
	Client c;
	Comments cmnt;
	Post post;
public:
	Blog(backend::DataStore &ds):
		c(ds),
		cmnt(c,c.newHandle<Comments::level>(std::list<Comment>())),
		post(c,c.newHandle<Post::level>(std::string())){}
	void spawn();
};
