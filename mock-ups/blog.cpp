
//consistency of class induces minimal consistency on objects in class.
//is a compiler error if the composite consistency is not the class's 
//consistency.  This does not cause consistency downgrades on the objects
//in the class.
remote<consistency::causal> class BlogPost {

	//objects in this class can be remote or ephemeral.  
	//remote objects correspond to datastore handles and must
	//be initiated with them.  Transient handles correspond 
	//to local state.

	remote<consistency::causal> List<Comment> cs;
	remote<consistency::linable> Article a;

};

DataStore s;

//the reference is stored with strong consistency.  The composite 
//consistency of its fields is causal, as annotated on the class.
//each underlying field has whichever consistency it wants.
BlogPost bp = new remote<s, consistency::strong> BlogPost();


//this desugars to the following:

class BlogPost {
	Handle<List<Comment>, consistency::causal, HandleAccess::all> cs;
	Handle<Article, consistency::linable, HandleAccess::all> a;
	Store s;
	BlogPost(Store s):s(s) { ... }
	static_assert(a.consis + b.consis >= consistency::causal);
};

DataStore s;
Handle<BlogPost, consistency::strong, HandleAccess::all> bp = 
	s.newObject<BlogPost, consistency::strong>();

//okay, so we really do need a way of carrying out operations transparently.  It's just too cumbersome to constantly ship objects back and forth. 


//it's possible to explicitly call datastore operations on objects which are 

//need a way to provide access to "primitive types" on the underlying datastores.
