#pragma once

enum class Level {weak, strong};

//handles and accompanying utilities.
//maybe needs a namespace.

namespace Access{
enum class Access {read, write, both, admin};

	constexpr bool read(Access a){
		return a != Access::write;
	}

	constexpr bool write(Access a){
		return a != Access::read;
	}

	constexpr bool noread(Access a){
		return a == Access::write;
	}

	constexpr bool nowrite(Access a){
		return a == Access::read;
	}

}
