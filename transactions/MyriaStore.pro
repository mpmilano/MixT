CONFIG += c++14

HEADERS = args-finder.hpp Assignmnet.hpp BaseCS.hpp Basics.hpp CommonExprs.hpp compile-time-lambda.hpp compile-time-tuple.hpp ConExpr.hpp ConStatement.hpp DataStore.hpp Eiger.hpp FileStore.hpp filter-varargs.hpp FreeExpr.hpp Fun.hpp Handle.hpp IfBuilder.hpp If.hpp macro_utils.hpp Operate.hpp Operate_macros.hpp Operation.hpp RemoteObject.hpp restrict.hpp Store.hpp TempBuilder.hpp Temporary.hpp TransactionAst.hpp TransactionBuilder.hpp Transaction.hpp Transaction_macros.hpp trans_seq_generated.hpp tuple_extras.hpp TypeMap.hpp type_utils.hpp utils.hpp WhileBuilder.hpp While.hpp

SOURCES = new_test.cpp

QMAKE_CXX = clang++
QMAKE_CXXFLAGS += -stdlib=libc++
QMAKE_LINK = clang++
QMAKE_LFLAGS = -stdlib=libc++

LIBS += -lboost_serialization -lboost_serialization-mt
