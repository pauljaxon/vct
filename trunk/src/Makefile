#==========================================================================
# Makefile for Victor sources and CSV-file tools
#==========================================================================
# This file is part of Victor: a SPARK VC Translator and Prover Driver.

# Copyright (C) 2009, 2010 University of Edinburgh

# Author: Paul Jackson

# Victor is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or (at
# your option) any later version.

# Victor is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.

# A copy of the GNU General Public License V3 can be found in file
# LICENSE.txt and online at http://www.gnu.org/licenses/.
#==========================================================================

# Tested with GNU Make v3.81.  
# References below to a GNU Make manual are to the manual for this version.

# Tools used:

#   g++/gcc    V4.4.5
#   bison      V2.4.1
#   flex       V2.5.35

# External libraries used:

#   gmp  V4.3.1


#==========================================================================
# Use
#==========================================================================

# Normal build
#   make vct

# Build with alternate optimisation level
# 
#   make vct OPT=<level>
# 
# <level> should be one of "none", "1", "2" or "3".
# Default level is 2.

# Build with debugging version of CVC3
#   make DEBUG=on vct 

#==========================================================================
# User customisation
#==========================================================================


# Linking with Yices
# ------------------

# If linking with Yices, ensure following line is uncommented and value
# is set appropriately.  If not linking with Yices, ensure line is commented
# out. 
# 
# The named directory is expected to contain lib/ and include/ 
# directories, as set up by the Yices installation.

# YICESDIR=/home/pbj/smt/yices/yices-1.0.29


# Linking with CVC3
# -----------------

# If linking with CVC3, ensure following line is uncommented and value
# is set appropriately.  If not linking with CVC3, ensure line is commented
# out. 

# The named directory is expected to contain lib/ and include/ 
# directories, as set up by the cvc3 installation.

# If also a cvc3 library is created with debug options turned on, the named
# directory should also contain at least the directory debug/lib/

# CVC3DIR=/home/pbj/smt/cvc3/r2.2
# CVC3DIR=/home/pbj/smt/cvc3/r2011-08-24



#==========================================================================
# Main make code
#==========================================================================
# Nothing here should need modifying by user.

VCTEXEC=vct  # Name of Victor executable

DEBUG=false
# -Wall rather noisy.  E.g. size() method returns unsigned int and
#  then get complaint about signed/unsigned comparisons. 
#  Fixed for now but just adding (int) coercions to code.
#
#   Wanting from flags: 
#      detecting control reaching end of functions with implicit return
#      Is allowed by compiler when have T* return types.
# unused vars.

# Expected values of OPT set on make command line are "none", "1",
# "2" or "3".

OPT=2

ifeq ($(OPT),none)
  CXXFLAGS= -g -Wall  
else
  CXXFLAGS= -g -Wall -O$(OPT)
endif


CPPFLAGS=

LDFLAGS= -Wall 

ifdef STATIC_GMP
  LDLIBS= -Xlinker -Bstatic -lgmpxx -lgmp -Xlinker -Bdynamic
else
  LDLIBS= -lgmpxx -lgmp 
endif

OBJS= \
  node.o \
  node-utils.o \
  lex.yy.o \
  pdriver.o  \
  box.o \
  formatter.o \
  pprinter.o \
  typesort.o \
  utility.o \
  bignum.o \
  context.o \
  normalisation.o \
  processor.o \
  translation.o \
  main.o \
  smt-driver.o \
  rule-filter.o \
  smtlib-driver.o \
  smtlib2-driver.o \
  isab-driver.o


ifdef YICESDIR
  CPPFLAGS+=  -I $(YICESDIR)/include -D LINK_YICES
  LDFLAGS+= -L $(YICESDIR)/lib 

  # Yices documentation says to also use -lstdc++ and -lgmp with gcc.
  # However here
  # - assume with g++ that stdc++ is always linked in
  # - assume that since gmp is already statically linked in we don't need -lgmp

  LDLIBS+= -lyices 
  OBJS+= yices-driver.o
endif

ifdef CVC3DIR

  ifeq ($(DEBUG),false)
      CPPFLAGS+= -I $(CVC3DIR)/include/cvc3 -D LINK_CVC3
      LDFLAGS+= -L $(CVC3DIR)/lib
  else
      CPPFLAGS+= -I $(CVC3DIR)/debug/include/cvc3 -D LINK_CVC3 -D_CVC3_DEBUG_MODE 
      LDFLAGS+= -L $(CVC3DIR)/debug/lib
  endif

  # CVC3 needs to be linked with gmp library.  This library is already linked
  # in by default, so no need to include it here.
  LDLIBS+= -lcvc3
  OBJS+= cvc-driver.o 
endif

# Putting parser.tab.o last in OBJS list ensures that `include'
# directive below will force running of bison before trying to extract
# dependency information for .cc files that include .hh files
# generated by bison: if these auto generated files are missing, the
# g++ -MM dependency extraction doesn't work properly.  Couldn't find
# any explanation in Make manual as to why Make tries to regenerate
# last included makefile first.

OBJS+= parser.tab.o 


# All sources
SRCS= $(OBJS:.o=.cc)

# All dependency files
DFILES= $(SRCS:.cc=.d)

# $^ = all prerequisites
# $@ = rule target

vct: $(OBJS)
	$(CXX) -o ../bin/$(VCTEXEC) $(LDFLAGS) $^ $(LDLIBS)


lex.yy.cc: lexer.ll
	flex -olex.yy.cc lexer.ll

# Option "-r state" writes file parser.output describing parser.
# Good for debugging parser.

parser.tab.hh parser.tab.cc: parser.yy
	bison -r state parser.yy



# Rules for compiling individual object files.
# 
# For each <name>.cc, the rule below creates a <name>.d file containing
# a single commandless rule of form
# 
# <name>.o <name>.d : <name>.cc <f1>.hh ... <fn>.hh
# 
# where <f1>.hh ... <fn>.hh are all the header files included by <name>.cc.
# 
# Such rules provide additional prerequisites to the implicit rules for
# generating .o and .d files.
# 
# See the Make manual
# 
#   Writing Rules/Generating Prerequisites Automatically, p40
#   Using Implicit Rules, p101,
# 
# for details. 

# The "@" suppresses echoing of the commands. See 
# 
#   Writing Commands / Command Echoing, p51.

# Once consequence of this approach is that make can force bison/lex to
# run when checking dependencies, even if one tries make -n!

# This is problem if building on platform with wrong version of bison/flex
# and only supplying .cc and .hh files.

%.d: %.cc
	@echo Making dependency file $@
	@set -e; rm -f $@; \
	 $(CXX) -MM $(CPPFLAGS) $< > $@.$$$$; \
	 sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	 rm -f $@.$$$$

# The include directive not only includes the named .d files, but also
# attempts to generate missing .d files using the above rule.

include $(DFILES)

# Implicit rule (from make -p) used for compiling .cc to .o is:
# 
# %.o: %.cc
# 	$(COMPILE.cc) $(OUTPUT_OPTION) $<
# where
#  COMPILE.cc = $(CXX) $(CXXFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c
#  OUTPUT_OPTION = -o $@
#  CXX = g++


.PHONY: clean save-build test-bignum

basicclean:
	-rm -f $(OBJS) *.d


clean:
	-rm -f $(OBJS) \
            lex.yy.cc \
            parser.tab.cc parser.tab.hh \
            location.hh position.hh stack.hh parser.output \
            *.d

save-build:
	-mkdir build
	cp parser.tab.cc parser.tab.hh location.hh position.hh stack.hh \
           lex.yy.cc parser.output  ../bin/* build/


test-bignum: test-bignum.cc bignum.o utility.o node.o \
             pprinter.o box.o formatter.o
	$(CXX) -g -o test-bignum -lgmpxx -lgmp $^



# Utility functions for manipulating CSV files.


csvproj: csvproj.cc utility.o
	$(CXX) -o ../bin/csvproj $^

csvfilt: csvfilt.cc utility.o
	$(CXX) -o ../bin/csvfilt $^

csvmerge: csvmerge.cc utility.o
	$(CXX) -o ../bin/csvmerge $^

csvisect: csvisect.cc utility.o
	$(CXX) -o ../bin/csvisect $^

.PHONY: csvutils
csvutils: csvproj csvfilt csvmerge csvisect

# Run command for solver, with watchdog killing solver if no activity on
# stdout for a specified period.

watchdog: watchdogrun.c
	gcc -o ../run/watchdogrun $^


# END OF FILE
