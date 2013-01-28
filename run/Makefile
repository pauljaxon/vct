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
CVC4=cvc4
SIMPLIFY=simplify
ALT_ERGO=alt-ergo

# Locations of SPARK VC files
# ---------------------------

# Victor is run using Make targets of form
#
#   <vcset>-<translation-and-prover-options>
#
# Here <vcset> is a name given to a set of VCs.  The set can be
# anything from one or two VCs from a single unit to all the VCs from
# 1000s of units of a large project.

# The association between a particular <vcset> name and the
# corresponding VCs is made by setting the value of a Make variable
# "<vcset>_options" which supplies Victor options for selecting the
# VCs.

# * For multiple units, common options are -prefix and -units. See
#   autop and hilton VC set examples below.  See the user manual "Unit
#   listing input file" section for details on how to format the .lis
#   file provided as the -units value.

# * For a single unit, the unitname can be provided as a non-option
#   argument, along with a -prefix option, as needed.

# * The options list can also include flags that provide
#   prover-independent customisation of the VC translation.  For
#   prover-specific customisation, it is probably better to create
#   variations on the provided targets.

# For each VC set, a phony target "vcset-<vcset>" is also needed in
# order to ensure that the pattern targets for running Victor always
# correctly match a valid VC set anme.

# Make variables for VC sets distributed with Victor.

VCT_VCSETS=../vc

autop_options = \
    -prefix=$(VCT_VCSETS)/autop/code \
    -units=$(VCT_VCSETS)/autop/autop-units.lis
.PHONY: vcset-autop

hilton_options = \
    -prefix=$(VCT_VCSETS)/hilton/code \
    -units=$(VCT_VCSETS)/hilton/hilton-units.lis
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
# fire when make called.


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
# is overridden. 
#
#
# Format of documentation 
# <Title>
#
# <Varname> [=<value> : <type>]   [(<default>)]  <suffix>
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
#
#
#----------------------------------------------------------------------------
# INPUT OPTIONS
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
# siv file
#---------
#
# SIV=<val> : bool (false)  -siv
# 
# If <val> is true, read goals from .siv files output by Simplifier
# rather than .vcg files output by Examiner. 
#
#----------------------------------------------------------------------------
# Focus on single unit and possibly single goal
# ---------------------------------------------
#
# UNIT=<unitname> : string   ("")
# GOAL=<goalnumber> : int    (0)
#
# Setting UNIT overrides any units file listing of units to run on.
# A default GOAL number of 0 indicates to try all goals.
#
#----------------------------------------------------------------------------
# User rule file selection
#-------------------------
#
# NRLU=<val> : bool (false)
# RLUA=<val> : bool (false)
#
# The default is to read in .rlu user rule files and to expect
# directory-level user rule files to perhaps have rules which have
# constant, function and type ids that are undeclared in some of the
# unit .fdl files in the directory.  In a run on a particular unit,
# any such rules are ignored.
#
# * NRLU Do not read in any .rlu user files.
#
# * RLUA When processing each unit in a directory, read in all the
#        .fdl files in the directory and merge their declarations, and
#        read in the .rlu user rule files.  With this option, the
#        expectation is that all the user rules will have all their
#        ids properly declared.

#----------------------------------------------------------------------------
# TRANSLATION OPTIONS
#----------------------------------------------------------------------------

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

#-----------------------------------------------------------------------------
# Handling of div and mod operators
#----------------------------------

# EDM=<val> : bool (false) -edm
#
# Add axioms to relate the FDL div and mod operators to the Euclidean
# div and mod operators defined in the SMTLIB2 standard.
# 
# The Euclidean div and mod satisfy the formulas
# 
#   x = y * (x div y) + x mod y
#   0 <= x mod y < |y|
# 
# Without this option, Victor uses an axiomatic characterisation of the FDL
# div and mod provided in the file run/divmod.rul.

#----------------------------------------------------------------------------
# Set threshold for symbolic numeric constants
#---------------------------------------------
#
# SYM_CONSTS=<threshold> : int (100000)
#
# Use with Simplify solver to attempt to avoid overflows.


#-----------------------------------------------------------------------------
# Enumeration type handling
#--------------------------

# AXE=<val> : bool (false) -axe
# ABE=<val> : bool (false) -abe

# * AXE Make each enumeration type E an uninterpreted type and introduce 
#       axioms and functions describing how E is isomorphic to an integer 
#       subrange.
# * ABE Make each enumeration type E an uninterpreted type and leave in 
#       place the axioms generated by the Examiner.  These axioms give
#       a partial description of an isomorphism with an integer subrange.
#
# The default is to define each enumeration type as an integer subrange,
# and associated functions and relations as corresponding functions and
# relations on integers.

#----------------------------------------------------------------------------
# Options for SMTLIB and Simplify translations
#---------------------------------------------
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

#----------------------------------------------------------------------------
# USER RULE ANALYSIS
#----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# Finding redundant rules
#-------------------------
#
# FRR=<val> : bool (false) -frr
#
# Employ the user rule knockout capability to find minimal sets of
# user rules needed to prove each goal, and to identify which rules
# are not required.
#
# See the User Manual for details.

#-----------------------------------------------------------------------------
# Auditing user rules
#--------------------

# FURA=<val> : bool (false) -fura
# QURA=<val> : bool (false) -qura
# SURA=<name> : string ("") -sura
#
# Generate prover goals for auditing user rules rather than using information
# from .vcg and .siv files.
#
# Choose just one option.
#
# * FURA  Full user rule audit.  Generate audit goals of kinds A,B,C,D,E.
# * QURA  Quick user rule audit.  Only generate audit goals of kind B.
# * SURA  Single user rule audit. Generate audit goals of kinds C,D,E
#         just for the user rule with name <name>.
#
#
# See Makefile-prelude.mk and the User Manual for further information.


#----------------------------------------------------------------------------
# PROVER DRIVING OPTIONS
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
# Proof summaries
#----------------
#
# PS=<val> : bool (false) -ps
#
# If <val>=true, ask the prover to generate a proof summary for each goal.
#
# A proof summary is a list of the parts of a VC (rules, hypotheses
# and conclusion) needed to prove the VC.  Proof summaries are obtained
# from unsat cores output by SMT solvers.  Only works with the SMTLIB2 
# interface.
#
# Proof summaries get reported in the remarks field of .vct file records.

#----------------------------------------------------------------------------
# Timeouts
#---------
#
# T=<time>  : int (1)   -t<time>
# CT=<time> : int       -ct<time>
# WT=<time> : fixed     -wt<time>
# ZT=<time> : int       -zt<time>

# Unless otherwise specified, these timeouts only apply to prover runs
# via the file-level interface.  At most one of these options should
# be selected.
# 
# * T   Ulimit timeout.  Set timeout in seconds using process ulimit facility 
#
# * CT  CVC3-specific timeout. Set timeout in units of 0.1 second.  Works for
#       both API and file-level interfaces. 
#
# * WT  Watchdog timeout.   Uses auxiliary program run/watchdogrun that
#       is created by running "make watchdog" in the src/ directory.
#       The watchdog program times out a prover if there is no activity
#       on the prover's stdout within the specified time period.
#       This kind of timeout is particularly useful when the prover is
#       handling multiple queries in a single run, as then each query
#       gets the full timeout time budget.
#       This option also allows finer time resolution than T.
#
# * ZT  Z3-specific timeout.  Set timeout in units of ms.  Only works with
#       SMTLIB2 file interface.  This timeout applies separately to each
#       query in the SMTLIB2 script.

#----------------------------------------------------------------------------
# Exploit solver incrementality
#------------------------------

# INC=<val> : bool (false) -inc
#
# For a given unit, try passing declarations and rules to the solver
# just once, and then incrementally assert and check each set of
# hypotheses and conclusion of each goal.  This can give a significant
# performance improvement. 

# Feature only tested so far with z3 and smtlib2 interface.


# ----------------------------------------------------------------------------
# Use unfused conclusions
#------------------------
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
# Working directory
#------------------
# WKDIR=<dirname> : string  (/tmp or work)
#
# Directory for prover input and output files. 
# Default is work if FWF or HWF options selected below.  Otherwise, is /tmp.
#
#-----------------------------------------------------------------------------
# Generate and preserve prover files
#------------------------------------
#
# FWF=<val> : bool (false)  
# HWF=<val> : bool (false)  
#
# * FWF Flat working files: put all files into working directory and
#       merge pathnames of units into filenames.  
# * HWF Hiearchical working files: put all files into hieararchy under
#       working directory, mirroring hieararchy of unit files.
#
# The default working directory is changed from /tmp to the work/ subdirectory
# of the current directory, unless a directory is explicitly specified using
# the WKDIR option.
#
#-----------------------------------------------------------------------------
# Add comments to working files
#------------------------------
#
# CMT=<val> : bool (false) 
#
# Add comment giving name of each formula (each rule, hypothesis or
# conclusion) included in the prover input file.  Names used in Victor
# input files are preserved, and new unique names are created for each
# rule added internally by Victor.

#----------------------------------------------------------------------------
# REPORT FILE OPTIONS
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
# Directory for reports
#----------------------
# OUTDIR=<dirname> : string  (out)
#
# Directory for .vct, .vus, .vsm and .vlg report files

#----------------------------------------------------------------------------
# Trivial goal tracking
#----------------------
#
# NTG=<val> : bool (false) -ntg
#
# `Trivial goals' are goals in .vcg or .siv files with no hypotheses or
# conclusions and listed as simply "*** true".
#
# By default, Victor includes a line in the .vct report file for each
# of these goals.  If <val> is true, reports on trivial goals are omitted
# from the .vct report, and the .vus, .vsm and .vlg reports all omit counts
# of the trivial goals.

#----------------------------------------------------------------------------
# Report file suffix
#-------------------
#
# SFX=<suffix> : string  ""
#
# Add an extra suffix to report file name.  Use this option when some
# parameter affecting behaviour is changed in a way that doesn't
# otherwise add a suffix.

#----------------------------------------------------------------------------
# MISCELLANEOUS OPTIONS
#----------------------------------------------------------------------------
#
#----------------------------------------------------------------------------
# Extra Victor options
#---------------------
#
# EXTRAS=<options> : string ""
#
# Feed extra options to Victor, perhaps overriding previous options.
# This provides a quick way to customise a run.  It can be useful to
# also use the SFX option to differentiate the report files for such a
# run.

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
#   <vc-set>-<interface-m><prover>
#
# Output files:
# <prefix><vc-set>-<interface-m><prover><suffix>.[vlg|vct|vsm]

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
# c4: cvc4
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

# Also below we use <prefix><vc-set>-xx when analysing Simplifier 
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


%-sc4: vcset-%
	$(VCT) \
            $(std_options)\
            $(smtlib_options) \
            -prover-command='$(CVC4) --lang smtlib1 $(cvc4_timeout_flag)'\


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

# Alt-Ergo. 0.94 and 0.95

# Have observed several incompletenesses with SMTLIB2 support:
#
# 1. define-type not supported.
# 2. to_real function not recognised. 
# 3. Instances of quantified Bool-typed variables in formula positions
#    not recognised. 
# 4. Boolean operators (and, or etc) not recognised in term positions
# 5. Sometimes returns multiple unsat responses to single (check-sat).
# 6. Sometimes returns "unknown (sat)" response to (check-sat).
#
# Target with workarounds for 1,2,5 and 6, but not 3 and 4.
# 
%-ta: vcset-%
	$(VCT) \
            $(std_options)\
            $(smtlib2_options) \
            -elim-type-aliases\
            -smtlib2-add-to_real-decl\
            -ignore-extra-query-results\
            -prover-command=$(ALT_ERGO) \

# Target with workarounds for 1-6.

%-ua: vcset-%
	$(VCT) \
            $(std_options)\
            $(smtlib2_strict_fol_options) \
            -elim-type-aliases\
            -smtlib2-add-to_real-decl\
            -ignore-extra-query-results\
            -prover-command=$(ALT_ERGO) \

# In both cases above, the to_real function is declared, but no axioms
# are provided to characterise it.

# CVC3 2.4.1
#  1. define-type not supported.
#  2. to_real function not recognised.
#  3. (set-option :print-success false)  flagged as syntax error.
#  4. Bool cannot be used as function argument type.
#  5. Bool cannot be type of quantified var.
#
# Workarounds provided for 1-3, but not 4 and 5.

%-tc: vcset-%
	$(VCT) \
            $(std_options)\
            $(smtlib2_options) \
            -elim-type-aliases\
            -smtlib2-omit-set-option-command\
            -smtlib2-add-to_real-decl\
            -prover-command='$(CVC3) -lang smt2 $(cvc3_timeout_flag)'\

# CVC4 1.0 

# Documentation says 
# o nonlinear-arithmetic is not yet adequately supported,
# o to_real from Reals_Ints theory not yet supported.
#   However, does seem to handle implicit Int to Real conversions as required
#   by SMTLIB 2, so instances of to_real suppressed.

%-tc4: vcset-%
	$(VCT) \
            $(std_options)\
            $(smtlib2_options) \
            -smtlib2-implicit-to_real\
            -prover-command='$(CVC4) --lang smtlib2 --smtlib-strict $(cvc4_timeout_flag)'\

# Z3 3.2 - 4.3.1
# No significant incompletenesses observed yet.

%-tz: vcset-%
	$(VCT) \
            $(std_options)\
            $(smtlib2_options) \
            -prover-command='$(Z3)' \

%-uz: vcset-%
	$(VCT) \
            $(std_options)\
            $(smtlib2_strict_fol_options) \
            -prover-command='$(Z3)' \

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
            $(EXTRAS)


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
# Create report for Simplifier results

%-xx: vcset-%
	$(VCT) \
            $(std_options)\
            -interface-mode=dummy \
            -prover=none\
            -siv

# Previous version, when default was not to include trivial goals in runs
# %-xx: vcset-%
#	make $*-dn TG=true
#	make $*-dn SIV=true TG=true
#	csvmerge $(OUTDIR)/f-$*-dn-siv-tg.vct 1 2 3 4 5 6 7 8 \
#          $(OUTDIR)/f-$*-dn-tg.vct 8 | \
#          csvfilt 9 unproven | csvproj 1 2 3 4 5 6 7 8 > $(OUTDIR)/f-$*-xx.vct


#============================================================================
# Gathering summary files
#============================================================================

sum:
	cat vsm-file-header.txt $(OUTDIR)/*.vsm > $(OUTDIR)/all-sum.csv

#============================================================================
# Sorting VCT file entries by run-time
#============================================================================
# Output columns:
# Time status path unit goal# concl#

%-sorted.vct:  %.vct
	sort -s -n -t ',' -k 10,10 $*.vct | csvproj 10 9 2 3 7 8 > $@

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
