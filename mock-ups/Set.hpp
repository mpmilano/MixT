
template<consistency LO, consistency L, typename T>
remote<L> class RemoteSet<T>{

private:
	
	remote<L> class Node {
		remote<LO> T obj;
		remote<LO> Option<T> obj2;
		remote<L> Option<Node> l;
		remote<L> Option<Node> m;
		remote<L> Option<Node> r;
		remote<L> Node p;
	};

	//non-empty only.
	remote<L> Node n;
	
	//transactions only allow terminating code.
	//loops are allowed, but must come with a 
	//budget (or be checkably terminating).
	remote<L> int maxdepth;
public:


	//true indicates added
	//false indicates already in heap.
	bool insert(T t){
		transaction {
			
			function split = [](Node parent, Node current, item i){
				if (i < current.obj) {
					insert_up(parent, current.obj);
					current.obj = i;
				}
				else if (i < current.obj2){
					insert_up(parent,i);
				}
				else {
					insert_up(parent,current.obj2);
					current.obj2 = i;
				}
			};

			function insert_up = [](Node n, item i){
				if (None == n.obj2){
					n.obj2 = i;
				}
				else split(n.parent, n, i);
			};

			Node n = this-> n; //this will correspond to retrieving a local copy, I think
			int counter = 0;
			while (counter <= maxdepth){
				if (t == n.obj || n.obj2 == t) return false;
				else {
					if (t < n.obj){
						if (n.left == None){
							
						}
						n = n.left;
						counter++;
						continue;
					}
					else if (t < n.obj2){
						
					}
					//insert right, then swap.
					
				}
			}
		}
		
	}

};
