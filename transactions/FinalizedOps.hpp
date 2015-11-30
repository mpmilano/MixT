#pragma once

namespace myria{

	template<Level l, typename T> FINALIZE_OPERATION(Increment, 1, (RemoteObject<l, T>* a));
}
