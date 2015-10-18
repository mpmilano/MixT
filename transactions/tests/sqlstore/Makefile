all:
	clang++ -g $$TRANS_DIR/utils.o $$TRANS_DIR/dummy_tracker.o $$TRANS_DIR/SerializationSupport.o $$TRANS_DIR/SQLStore.o $$TRANS_DIR/Context.o $$TRANS_DIR/TempBuilder.o $$TRANS_DIR/ConExpr.o $$TRANS_DIR/WhileBuilder.o *cpp -o test -I$$TRANS_DIR $(clangFlags) $(linkFlags) && ./test

