#pragma once

template<std::size_t size, typename T> struct SingleAllocator;

template<typename T>
struct allocated_ref{
private:
	std::size_t indx;
public:
	
	template<std::size_t size>
	constexpr T& get(SingleAllocator<size,T>& new_parent){
		return new_parent.data[indx-1];
	}

	template<std::size_t size>
	constexpr const T& get(SingleAllocator<size,T>& new_parent) const {
		return new_parent.data[indx-1];
	}

	template<std::size_t size>
	constexpr const T& get(const SingleAllocator<size,T>& new_parent) const {
		assert(indx > 0);
		return new_parent.data[indx-1];
	}

	constexpr allocated_ref(const allocated_ref&) = delete;
	constexpr allocated_ref(allocated_ref&& o):indx(o.indx){}
	struct really_set_index{constexpr really_set_index() = default;};
	constexpr allocated_ref(const really_set_index&, std::size_t index):indx{index}{}
	constexpr std::size_t get_index() const {return indx;}

	constexpr allocated_ref():indx{0}{}

	template<std::size_t s>
	constexpr allocated_ref(SingleAllocator<s,T>&);

	template<std::size_t s>
	constexpr void free(SingleAllocator<s,T>&);
	
	constexpr allocated_ref& operator=(allocated_ref&& o){
		indx = o.indx;
		return *this;
	}

	constexpr operator bool() const {return indx > 0;}
	
};
