ENGINE_DIR := nx-engine

#all: server client
all: client

client: engine client_only

server: engine server_only

engine:
	@$(MAKE) -C $(ENGINE_DIR) -f Makefile

client_only:
	@$(MAKE) -C client -f Makefile

server_only:
	@$(MAKE) -C server -f Makefile

run_client:
	@$(MAKE) -C client -f Makefile run

run_server:
	@$(MAKE) -C server -f Makefile run

.PHONY: all engine server client server_only client_only run_client run_server 
