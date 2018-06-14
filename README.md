consistency-tester
==================

Fake data store interface with mutltiple levels of consistency + tester


Things clang *really* doesn't like:
 - concept-style `auto` 

Things that g++ *really* doesn't like: 
 - exceptions thrown from constexpr contexts unless guarded by explicit conditionals
 - anonymous structs used as template parameters
 - deeply-nested constexpr; you need a *lot* of RAM to build large examples under g++.  My largest example took 45GB on g++-8.1