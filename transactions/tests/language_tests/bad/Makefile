all:
	for foo in *cpp; do if clang++ -g $$TRANS_DIR/utils.o $$TRANS_DIR/Tracker.o $$TRANS_DIR/ConExpr.o $$TRANS_DIR/Context.o $$TRANS_DIR/TempBuilder.o $$TRANS_DIR/SerializationSupport.o $$TRANS_DIR/WhileBuilder.o $$foo -I$$TRANS_DIR $(clangFlags) $(linkFlags); then false; else echo "good"; fi; done 
