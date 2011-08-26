#                                        -*- Automake -*-
# Process this file with automake to produce Makefile.in.

AM_V_LN = $(am__v_LN_$(V))
am__v_LN_ = $(am__v_LN_$(AM_DEFAULT_VERBOSITY))
am__v_LN_0 = @echo "  LN    " $@ "->" .libs/$@;

%.so:
	$(AM_V_LN)$(LN_S) .libs/$@ $@

all-local: $(MODULES:%.la=%.so)

clean-local: module-clean
module-clean:
	rm -f $(MODULES:%.la=%.so)
