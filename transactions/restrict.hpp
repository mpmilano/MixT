#pragma once

#define restrict(x) typename ignore = typename std::enable_if<(x)>::type
#define restrict2(x) typename ignore = void, typename ignore2 = typename std::enable_if<(x)>::type

#define CMA ,
