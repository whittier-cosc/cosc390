# Top-level makefile that builds the static library (libwcpic32.a) and 
# all projects.

LIB_DIR=wcpic32lib
SKEL_DIR=skeleton
PROJ_DIRS=[0-9][0-9]-*

all:
	$(MAKE) -C $(LIB_DIR)
	$(MAKE) -C $(SKEL_DIR)
	for d in $(PROJ_DIRS); do $(MAKE) -C $$d; done

clean:
	$(MAKE) -C $(LIB_DIR) clean
	$(MAKE) -C $(SKEL_DIR) clean
	for d in $(PROJ_DIRS); do $(MAKE) -C $$d clean; done

# Copy skeleton Makefile to all project directories.
copy-makefile:
	for d in $(PROJ_DIRS); do cp $(SKEL_DIR)/Makefile $$d; done
	
.PHONY: all
.PHONY: clean
.PHONY: copy-makefile

