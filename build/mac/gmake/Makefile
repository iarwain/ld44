# GNU Make solution makefile autogenerated by Premake
# Type "make help" for usage help

ifndef config
  config=debug64
endif
export config

PROJECTS := ld44

.PHONY: all clean help $(PROJECTS)

all: $(PROJECTS)

ld44: 
	@echo "==== Building ld44 ($(config)) ===="
	@${MAKE} --no-print-directory -C . -f ld44.make

clean:
	@${MAKE} --no-print-directory -C . -f ld44.make clean

help:
	@echo "Usage: make [config=name] [target]"
	@echo ""
	@echo "CONFIGURATIONS:"
	@echo "   debug64"
	@echo "   profile64"
	@echo "   release64"
	@echo ""
	@echo "TARGETS:"
	@echo "   all (default)"
	@echo "   clean"
	@echo "   ld44"
	@echo ""
	@echo "For more information, see http://industriousone.com/premake/quick-start"
