MixT Prototype: Code
==================

DSL for mixed-consistency transactions: [website](https://mpmilano.github.io/MixT/), [paper](https://dl.acm.org/citation.cfm?id=3192375)

Hello! This is the prototype for MixT.  This is not intended for distribution or serious end-user experience; there are some very platform-specific assumptions in this code.  If you are brave or curious, welcome! 

Building MixT
---------
 * The transactions directory is the code's proper home. Build from there.
 * the pg_env.sh script will configure your environment for MixT.  `source pg_env.sh` before attempting to build!
 * clang++-5.0 is the Officially Supported Compiler™; ≥g++-7.2 should also work.
 * 64-bit Gentoo Linux is the only tested build and runtime environment for MixT.  
 * dev-libs/libpqxx is a required dependency for building MixT with postgres (as is default)
 * the build does not produce a library; link the object files explicitly please.
 * there is no `configure`; just `source pg_env.sh; make <target>`

Using MixT
------------
MixT's transaction compiler is implemented entirely in compile-time C++ through the use of `constexpr` and, yes, some templates.  To write a `MixT` transaction, just `#include mtl/mtl.hpp`.  There are some example transactions in the root directory; look for the `TRANSACTION(...)` invocations.  MixT includes bindings for a postgres backend and a simple in-memory backend; use the [in-memory backend](https://github.com/mpmilano/MixT/tree/master/transactions/testing_store) if you're just trying out some transaction code.  The in-memory backend is also a good thing to copy when writing your own bindings.

Interpreting errors
----------------
C++ is famously bad at giving reasonable error messages, especially when those errors involve templates.  First, I must strongly recommend clang here; g++ is not quite there yet with error messages.  If you are building under clang and MixT gives you an error when compiling your transaction, there are a few common cases to look for:
 * Always build with `-ferror-limit=1` set.  clang and gcc both tend to treat type errors as "pretend it's an integer and move on", which makes errors after the first one likely to be spurious or misleading.
 * `constexpr variable 'prev' must be initialized by a constant expression` indicates an exception has been thrown in constexpr code.  Scroll down until you see `subexpression not valid`, which will tell you the exact exception the code tried to throw. This is usually enough to understand the error. 
 * if you're getting an error in constexpr code, you can explicitly call the constexpr function that's throwing the error.  This will now execute at runtime, and give you normal exception behavior. 
 * `static_assert failed:` errors usually pertain to typechecking failures or flow violations.  In either case, you'll get an error message directly.  

Notes on compilers with MixT
--------------------------
This project is actively pushing the boundaries of compiler support for modern C++.  Some things to watch out for:

Things clang *really* doesn't like:
 - concept-style `auto`, which means the mixt_method syntax only builds on g++ until clang adds support.

Things that g++ *really* doesn't like: 
 - exceptions thrown from constexpr contexts unless guarded by explicit conditionals
 - anonymous structs used as template parameters
 - deeply-nested constexpr; you need a *lot* of RAM to build large examples under g++.  My largest example took 45GB on g++-8.1
