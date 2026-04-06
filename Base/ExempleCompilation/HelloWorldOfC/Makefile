SRC_DIR=src
BIN_DIR=bin
EXEC=helloworld

.PHONY: all install clean

all:
	$(MAKE) -C $(SRC_DIR)

install: all
	mkdir -p $(BIN_DIR)
	cp $(SRC_DIR)/$(EXEC) $(BIN_DIR)/

clean:
	$(MAKE) -C $(SRC_DIR) clean
	rm -f $(BIN_DIR)/$(EXEC)

