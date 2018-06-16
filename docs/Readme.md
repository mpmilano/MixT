MixT: An embedded, domain-specific language for Mixed-Consistency Transactions
==============================================================================

Welcome to MixT!  Whether you're here because you happened to see my recent [PLDI Talk](https://pldi18.sigplan.org/event/pldi-2018-papers-mixt-a-language-for-mixing-consistency-in-geodistributed-transactions), [paper](https://dl.acm.org/citation.cfm?id=3192375), or happened to find it in some other way, Welcome! 

So what's this MixT thing all about, anyway?
--------------------------------------------

MixT is a domain-specific programming langauges for writing transactions, embedded into C++.  What makes MixT unique is its approach to consistency and transaction isolation: rather than associate consistency with operations, MixT __associates consistency with data__ and expects programmers to use __multiple consistency models__ in the same application.  MixT transactions compile down to a sequence of **standard, single-consistency transactions** appropriate for execution on any number of SQL/NoSQL/NewSQL datastores.  To use MixT, [define an interface](https://github.com/mpmilano/MixT/blob/master/transactions/testing_store/TestingStore.hpp) for your datastore, [write a few transactions](https://github.com/mpmilano/MixT/blob/master/transactions/logging_example.cpp),  and you're off to the races! 

Shouldn't I really just pick one consistency model?
---------------------------------------------

If picking one consistency model works for your application, then great!  There's a dizzying array of consistency models out there; every third NoSQL/NewSQL database is oh-so-excited to tell you about their new twist on their new eventually, causally, semi-snapshot, absurdly-complex consistency model.  If you're thinking about using something like MixT, chances are you're already a user of one of these databases—and already familiar with what they do well, and (perhaps more importantly) what they really don't do well.  Rather than continuing to migrate all your data to ever-newer single databases, MixT lets you keep using your existing databases for the cases they handle well, mixing access among multiple different databases in the same application.  Using multiple consistency models is an error-prone process; MixT helps you avoid the errors that can happen when you mix models.

Where can I learn more?
-----------------------
The best source of information about the ideas behind MixT is [the paper](https://dl.acm.org/citation.cfm?id=3192375); if you're looking for information specifically about its prototype implementation, [the readme](https://github.com/mpmilano/MixT/blob/master/README.md) has basic instructions for getting started.  [The code](https://github.com/mpmilano/MixT) is first and foremost a research artifact; some difficulties building and using this artifact are to be expected, and documentation beyond setup and "hello world" is spare.  I am happy to provide any help required; send me a message via e-mail, on github, or by filing an issue against this repository.
