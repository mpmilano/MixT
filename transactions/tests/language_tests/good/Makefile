all:
	for foo in *cpp; do clang++ -g $$TRANS_DIR/utils.o $$TRANS_DIR/Ends.o $$TRANS_DIR/Tracker.o $$TRANS_DIR/Ostreams.o $$TRANS_DIR/ConExpr.o $$TRANS_DIR/Context.o $$TRANS_DIR/TempBuilder.o $$TRANS_DIR/SerializationSupport.o $$TRANS_DIR/WhileBuilder.o $$foo -I$$TRANS_DIR $(clangFlags) $(linkFlags); done 
