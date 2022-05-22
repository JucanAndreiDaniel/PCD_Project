include config.mk

all:
	$(MAKE) -C Server
	$(MAKE) -C Client

clean:
	$(MAKE) -C Server clean
	$(MAKE) -C Client clean
	$(RM) *~
