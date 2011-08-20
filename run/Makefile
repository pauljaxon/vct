#=============================================================================
# Make rules for running Victor
#=============================================================================
# This file is part of Victor: a SPARK VC Translator and Prover Driver.

# Copyright (C) 2009, 2010 University of Edinburgh

# Author(s): Paul Jackson

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


#=============================================================================
# User customisation
#=============================================================================


# Stand-alone executables for SMT solvers
# ----------------------------------------

# The default assumption is that the solver executables are on the current
# path.  As desired, absolute paths can be set up for each here.

Z3=z3
YICES=yices
CVC3=cvc3
SIMPLIFY=simplify
ALT_ERGO=alt-ergo

# Locations of SPARK VC files
# ---------------------------

# Targets are set up below that can easily be used for different sets
# of VC units.  For a set of VC units named <vc-set>, the targets
# assume the definition here of:
#
# 1. A Make variable "<vc-set>_prefix" for the root of the directory tree 
#    for the set.

# 2. A Make variable "<vc-set>_units" that names the .lis file listing 
#    the unit names and, for each unit, specifies options.  See the
#    user manual "Unit listing input file" section for details on how to
#    format this file.
#
# 3. A phony target "vcset-<vc-set>" against which to check matches for
#    the pattern rules below for running Victor.

# Make variables for VC sets distributed with Victor.

VCT_VCSETS=../vc

autop_prefix =  $(VCT_VCSETS)/autop/code
autop_units =   $(VCT_VCSETS)/autop/autop-units.lis
.PHONY: vcset-autop

hilton_prefix = $(VCT_VCSETS)/hilton/code
hilton_units =  $(VCT_VCSETS)/hilton/hilton-units.lis
.PHONY: vcset-hilton


#=============================================================================
# Make features used
#=============================================================================
#
# Automatic variables commonly used in commands are:
# 
# $@ : Set to whole rule target
#
# $* : Set to the stem of the target (the stem is the string that the 
#      % matches in the target)

# Rules are made generic with respect to the VC set by using GNU Make
# pattern rules. (rules with % in target)

# Use of phony rather than real targets ensures that rules always
# fires when make called.


#=============================================================================
# Variables settable in make calls
#=============================================================================
# 
# The behaviour of Victor runs can be customised by setting make variables
# when invoking make.
#
#
# Syntax for setting variable names is:
#
#  make <target>  <vname1>=<val1> .. <vnamek>=<valk>
#
# When make variables are set in this way, any definition in the makefile
# is overridden.  Variable descriptions are given below in rough order
# of likely interest to users.
#
#
# Format of documentation 
# <Title>
#
# <Varname> [=<value> : <type>]   [<default>]  <suffix>
#
# <Description>
# 
# where
# <value>  : suggestive name for value.
# <type>   : One of `int' or `string' or `fixed'. or `bool'.
#            `fixed' means fixed point. E.g. 1.5  0.03
#            `bool' values are `true' and `false'.
#
# <suffix> : Suffix added to report name. Usually includes <value> if
#            significant.

#----------------------------------------------------------------------------
# Timeout for file-level interface
#
# T=<time> : int (10)  -t<time>
#
# Set timeout in seconds for solver with file-level interface.
# Uses process ulimit facility to set timeout.

#----------------------------------------------------------------------------
# CVC3 timeout
#
# CT=<time> : int (100)  -ct<time>
#
# Set timeout in units of 0.1 seconds for CVC3.  Works for both API and SMTLIB
# file-level interfaces.  Uses CVC3's internal timeout capabilities.
# If CT is set, T is ignored.

#----------------------------------------------------------------------------
# Use unfused conclusions 
#
# U=<val> : bool (false)  u- | f-
# 
# If <val> is true, run solvers on each goal slice at a time. If false,
# fuse conclusions together so there is one solver run per goal.
# If conclusions are unfused, add a u- prefix to report name.
# If conclusions are fused, add an f- prefix to report name.
# 
# Prefix rather than suffix added to report file name so that summary
# files for each kind of run are separated out when files listed in
# order.

#----------------------------------------------------------------------------
# Directory for reports
#
# OUTDIR=<dirname> : string  (out)
#
# Directory for .csv .log .sum report files

#----------------------------------------------------------------------------
# Arithmetic abstraction and simplification
#
# L=  <val> : bool (false)  -lin
# LEC=<val> : bool (false)  -lin-ec
# LAE=<val> : bool (false)  -lin-ae
# EC=<val>  : bool (false)  -ec
# AE=<val>  : bool (false)  -ae

# Various options for coping with solvers that don't support non-linear
# arithmetic (e.g. Yices).  At most one should be selected.

# * EC substitutes out symbolic integer constants, constants introduced
#      with definition c = k where c is a constant, k an integer literal.
# * AE Applies EC first and then does some partial evaluation and 
#      simplification, trying to make more multiplications linear.
# * L abstracts non-linear multiplications.  Applied after EC or AE in LEC
#      and LAE.


#----------------------------------------------------------------------------
# siv file
#
# SIV=<val> : bool (false)  -siv
# 
# If <val> is true, read goals from .siv files output by Simplifier
# rather than .vcg files output by Examiner, and add -siv suffix to report
# names.

#----------------------------------------------------------------------------
# Track trivial goals
#
# TG=<val> : bool (false) -tg
#
# `Trivial goals' are goals in .vcg or .siv files with no hypotheses or
# conclusions and listed as simply "*** true".
#
# If <val> is true, Victor includes a line in the .csv report file for each
# of these goals.


#----------------------------------------------------------------------------
# Report file suffix
#
# SFX=<suffix> : string  ""
#
# Add an extra suffix to report file name.  Use this option when some
# parameter affecting behaviour is changed in a way that doesn't
# otherwise add a suffix.


#----------------------------------------------------------------------------
# Iterate calls of solver to increase precision of runtime measurements.
#
# RD=<count> : int (1)   -rd<count>
# RC=<count> : int (1)   -rc<count>
#
# RD ("Repeat Drive") for use with API interface mode, RC ("Repeat
# Check") for use with smtlib/simplify interface mode.

#----------------------------------------------------------------------------
# Focus on single unit and possibly single goal
#
# UNIT=<unitname> : string   ("")
# GOAL=<goalnumber> : int    (0)
#
# Setting UNIT overrides any units file listing of units to run on.
# A default GOAL number of 0 indicates to try all goals.

#----------------------------------------------------------------------------
# Set threshold for symbolic numeric constants
#
# SYM_CONSTS=<threshold> : int (100000)
#
# Use with Simplify solver to attempt to avoid overflows.

#----------------------------------------------------------------------------
# Options for SMTLIB and Simplify translations
# 
# A=<Aoption> : int (0)
# B=<Boption> : int (0)
# C=<Coption> : int (0)
# D=<Doption> : int (0)
# E=<Eoption> : int (0)
# F=<Foption> : int (0)
# G=<Goption> : int (0)
#
# Options A-F select translation alternatives and should not affect
# completeness.  Option G affects how array and record extensionality is
# dealt with.  The default for G is not to add in any support for
# extensionality. 
#
# See Makefile-prelude.mk for details on these options.

#=============================================================================
# Include prelude
#=============================================================================
# The prelude defines make variables that assemble together sets of
# Victor options for different solvers and different customisation
# options.

include Makefile-prelude.mk


#============================================================================
# Rules for running Victor
#============================================================================


# Naming conventions

# Make targets:
#   <unit-set>-<interface-m><prover>
#
# Output files:
# <prefix><unit-set>-<interface-m><prover><suffix>.[vlg|vct|vsm]

# <prefix> = 
# f-: conclusions fused
# u-: conclusions unfused

# <interface-m> =
# a: API
# s: SMT-Lib and file-level 
# t: SMT-Lib2 and file-level 
# p: Simplify and file-level
# i: Isabelle/HOL and file-level
# d: dummy

# <prover>=
# a: Alt-Ergo
# c: cvc3
# y: yices
# p: simplify
# z: z3
# n: none

# <suffix> = suffices added by setting make variables that customise
#            Victor behaviour

# e.g. 
#
#   make autop-ac
#
# Runs CVC3 via API on autop VC set and creates report files
#
#  f-autop-ac.vlg
#  f-autop-ac.vct
#  f-autop-ac.vsm
#
# in OUTDIR directory (by default ./out).

# Also below we use <prefix><unit-set>-xx when analysing Simplifier 
# behaviour.

#----------------------------------------------------------------------------
# API interface mode
#-------------------

%-ac: vcset-%
	$(VCT) \
           $(std_options) \
           $(api_cvc3_options)\


%-ay: vcset-%
	$(VCT) \
           $(std_options)\
           $(api_yices_options) \

#----------------------------------------------------------------------------
# SMTLIB file interface mode
#---------------------------

%-sa: vcset-%
	$(VCT) \
            $(std_options)\
            $(smtlib_options) \
            -prover-command=$(ALT_ERGO) \


%-sc: vcset-%
	$(VCT) \
            $(std_options)\
            $(smtlib_options) \
            -prover-command='$(CVC3) -lang smt $(cvc3_timeout_flag)'\

%-sy: vcset-%
	$(VCT) \
            $(std_options)\
            $(smtlib_options) \
            -abstract-nonlin-times\
            -logic=AUFLIA\
            -prover-command='$(YICES) -smt'\


%-sz: vcset-%
	$(VCT) \
            $(std_options)\
            $(smtlib_options) \
            -prover-command='$(Z3) -smt' \


#----------------------------------------------------------------------------
# SMTLIB2 file interface mode
#----------------------------
# Observed incompletenesses of SMTLIB2 support noted below.  
# (Partial) workaround options provided for some of these.

# Alt-Ergo. 0.93
# - define-type not supported.
# - to_real function not recognised.
# - Instances of quantified Bool-typed variables not recognised.

%-ta: vcset-%
	$(VCT) \
            $(std_options)\
            $(smtlib2_options) \
            -elim-type-aliases\
            -smtlib2-add-to_real-decl\
            -prover-command=$(ALT_ERGO) \

# CVC3 2.4 (SMT-COMP 2011 version)
#  - define-type not supported.
#  - to_real function not recognised.
#  - (set-option :print-success false)  flagged as syntax error.
#  - Bool cannot be used as function argument type.

%-tc: vcset-%
	$(VCT) \
            $(std_options)\
            $(smtlib2_options) \
            -elim-type-aliases\
            -smtlib2-omit-set-option-command\
            -smtlib2-add-to_real-decl\
            -prover-command='$(CVC3) -lang smt2 $(cvc3_timeout_flag)'\

# Z3 2.20/3.0 preview (As submitted to SMT-COMP 2011). 
# No significant incompletenesses observed yet.

%-tz: vcset-%
	$(VCT) \
            $(std_options)\
            $(smtlib2_options) \
            -prover-command=$(Z3) \


#----------------------------------------------------------------------------
# Simplify file interface mode
#-----------------------------

%-pp: vcset-%
	$(VCT) \
            $(std_options)\
            $(simplify_interface_options) \
            -sym-consts=$(SYM_CONSTS) \
            -prover-command='$(SIMPLIFY) -nosc' \


# As %-pp, but use z3

%-pz: vcset-%
	$(VCT) \
            $(std_options)\
            $(simplify_interface_options) \
            -sym-consts=$(SYM_CONSTS) \
            -prover-command='$(Z3) -s' \


# As %-pz but no threshold

%-pz-nt: vcset-%
	$(VCT) \
            $(std_options)\
            $(simplify_interface_options) \
            -prover-command='$(Z3) -s' \


#----------------------------------------------------------------------------
# Isabelle file interface mode
#-----------------------------

%-in: vcset-%
	$(VCT) \
            $(isabelle_options)\
            -prover=none\
            -working-dir=iwork \
            -flat-working-files\

%-ii: vcset-%
	$(VCT) \
            $(isabelle_options)\
            -prover-command='isabelle tty <' \
            -working-dir=iwork \
            -flat-working-files\


#----------------------------------------------------------------------------
# Dummy interface mode
#---------------------
# Uses include checking Victor's internal runtime and generating
# reports on Praxis Simplifier performance

%-dn: vcset-%
	$(VCT) \
            $(std_options)\
            -interface-mode=dummy \
            -prover=none\


%-sn: vcset-%
	$(VCT) \
            $(std_options)\
            $(smtlib_options) \
            -prover=none\


%-se: vcset-%
	$(VCT) \
            $(std_options)\
            $(smtlib_options) \
            -prover-command='./echo.sh' \


#============================================================================
# Simplifier analysis
#============================================================================
# Create .vct report for Simplifier results, excluding trivial goals.
# For now, .vsm reports must be generated by hand.

%-xx: vcset-%
	make $*-dn TG=true
	make $*-dn SIV=true TG=true
	csvmerge $(OUTDIR)/f-$*-dn-siv-tg.vct 1 2 3 4 5 6 7 8 \
          $(OUTDIR)/f-$*-dn-tg.vct 8 | \
          csvfilt 9 unproven | csvproj 1 2 3 4 5 6 7 8 > $(OUTDIR)/f-$*-xx.vct


#============================================================================
# Gathering summary files
#============================================================================

sum:
	cat vsm-file-header.txt $(OUTDIR)/*.vsm > $(OUTDIR)/all-sum.csv

#============================================================================
# Sorting VCT file entries by run-time
#============================================================================

%-sorted.vct:  %.vct
	sort -s -n -t ',' -k 9,9 $*.vct | csvproj 9 8 1 6 > $@

#============================================================================
# Sets of runs
#============================================================================
# e.g. make autop-all T=1

%-all:
	make $*-ac CT=$(T)0 
	make $*-ay EC=true
	make $*-sa
	make $*-sc
	make $*-sy LAE=true
	make $*-sz
	make $*-pp

all-all: 
	make autop-all
	make hilton-all


#============================================================================
# Development Make rules
#============================================================================


# include Makefile-dev.mk

# End of file.
