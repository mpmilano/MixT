#pragma once

namespace backend {
	enum class Level { causal, strong, fastest};

	template <Level L>
	class DataStore {
	public:
		int return_one () {return 1;}
		DataStore () {}
		DataStore (const DataStore<L> &) = delete;
	};


};
