OBJS=*.o
ODIR=obj

$(ROOT_DIR)/$(BIN_DIR)/ipc_daemon: $(ROOT_DIR)/$(BIN_DIR)/$(BIN)
	cp $< $@
$(ROOT_DIR)/$(BIN_DIR)/$(BIN):$(ODIR)/$(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)