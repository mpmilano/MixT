#include <iostream>

namespace mutils{

	struct Frequency
	{
		void Print() const { std::cout << hertz << "Hz\n"; }
		const unsigned long long hertz;
		constexpr auto operator+(const Frequency& fr) const {
			return Frequency{fr.hertz + hertz};
		}
		constexpr auto operator*(const Frequency& fr) const {
			return Frequency{fr.hertz * hertz};
		}
	};
	constexpr Frequency operator"" _Hz(unsigned long long hz)
	{
		return Frequency{hz};
	}
	constexpr Frequency operator"" _kHz(long double khz)
	{
		unsigned long long coerce = khz * 1000;
		return Frequency{coerce};
	}
	constexpr Frequency operator"" _kHz(unsigned long long khz)
	{
		unsigned long long coerce = khz * 1000;
		return Frequency{coerce};
	}

	constexpr Frequency as_hertz(unsigned long long hz){
		return Frequency{hz};
	}

}
