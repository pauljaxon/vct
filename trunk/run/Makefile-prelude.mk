#=============================================================================
#=============================================================================
# Makefile-prelude.mk
#=============================================================================
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
#
#=============================================================================

#=============================================================================
# Victor executable
#=============================================================================

VCT=vct

#=============================================================================
# Overridable options
#=============================================================================
# "ifdef <V>" used for bool variables.  <V> is considered `true' just when
# it is defined.
# 
#
#----------------------------------------------------------------------------
# INPUT OPTIONS
#----------------------------------------------------------------------------
#-----------------------------------------------------------------------------
# Selecting VCG or SIV file for VCs
#----------------------------------

ifdef SIV
  siv_flag=-siv
  siv_sfx = -siv
endif

#-----------------------------------------------------------------------------
# Focussing on single unit and goal
#----------------------------------

ifdef UNIT
  unit_option = $(UNIT)
endif

ifdef GOAL
  goal_option = -goal=$(GOAL) -ctick
endif

#-----------------------------------------------------------------------------
# Reading standard user-defined rules files
#------------------------------------------
# Default in std_options is
#  -read-directory-rlu-files\
#  -read-unit-rlu-files \
#  -expect-dir-user-rules-with-undeclared-ids

ifdef NRLU
  std_rlu_sfx=-nrlu
  std_rlu_option= -read-directory-rlu-files=false -read-unit-rlu-files=false

else ifdef RLUA
  std_rlu_sfx=-rlua
  std_rlu_option= -read-all-decl-files-in-dir

endif

#-----------------------------------------------------------------------------
# Type inference
#----------------
# Control of type inference for free vars in rules

ifeq ($(TI),0)
  ti_option=# (empty string)
  ti_sfx =-ti0
else ifeq ($(TI),1)
  ti_option=-assume-var-in-real-pos-is-real 
  ti_sfx =-ti1
else ifeq ($(TI),2)
  ti_option=-assume-int-or-real-var-is-real
  ti_sfx =-ti2
else ifeq ($(TI),3)
  ti_option=-assume-int-or-real-var-is-int
  ti_sfx =-ti3
else ifeq ($(TS),1)
  ti_option=-assume-var-in-real-pos-is-real\
         -suppress-warnings-of-var-type-assumptions
  ti_sfx =# (empty string)
else ifeq ($(TS),2)
  ti_option=-assume-int-or-real-var-is-real\
            -suppress-warnings-of-var-type-assumptions
  ti_sfx =-ts2
else ifeq ($(TS),3)
  ti_option=-assume-int-or-real-var-is-int\
         -suppress-warnings-of-var-type-assumptions
  ti_sfx =-ts3
else
  ti_option=-assume-var-in-real-pos-is-real\
            -suppress-warnings-of-var-type-assumptions
  ti_sfx =# (empty string)
endif


#----------------------------------------------------------------------------
# TRANSLATION OPTIONS
#----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# Arithmetic abstraction and simplification
#------------------------------------------

ifdef L
  lin_sfx=-lin
  lin_opt=-abstract-nonlin-times \
          -abstract-non-const-real-div
  smtlib_logic=AUFLIRA
else ifdef LEC
  lin_sfx=-lin-ec
  lin_opt=-abstract-nonlin-times \
          -abstract-non-const-real-div\
          -abstract-nonlin-times \
          -elim-consts 
  smtlib_logic=AUFLIRA
else ifdef LAE
  lin_sfx=-lin-ae
  lin_opt=-abstract-nonlin-times -abstract-non-const-real-div\
          -elim-consts \
          -ground-eval \
          -arith-eval 
  smtlib_logic=AUFLIRA
else ifdef EC
  lin_sfx=-ec
  lin_opt=-elim-consts
  smtlib_logic=AUFNIRA
else ifdef AE
  lin_sfx=-ae
  lin_opt=-elim-consts \
          -ground-eval \
          -arith-eval 
  smtlib_logic=AUFNIRA
else
  lin_sfx= # empty
  lin_opt= # empty
  smtlib_logic=AUFNIRA
endif

#-----------------------------------------------------------------------------
# Handling of div and mod operators
#----------------------------------

# Use Euclidean div and mod operators rather than the
# axiomatisation given in divmod.rul.

# See comment in arith.cc for a discussion of div & mod handling.

# Supported only by SMTLIB2.

ifdef EDM
   edm_sfx=-edm
   edm_options=\
     -rules=none\
     -rules=prelude.rul\
     -use-euclidean-divmod
endif

#-----------------------------------------------------------------------------
# Set threshold for symbolic numeric constants
#---------------------------------------------

SYM_CONSTS=100000


#-----------------------------------------------------------------------------
# Enum type options
#------------------

ifdef AXE
  enum_sfx = -axe
  enum_option = -axiomatise-enums
else ifdef ABE
  enum_sfx = -abe
  enum_option = -abstract-enums
else
  enum_option = -elim-enums
endif

#-----------------------------------------------------------------------------
# Translation options for SMTLIB, SMTLIB2 and Simplify
# ----------------------------------------------------
#
# Possible values:
#
#   A = 0,1
#   B = 0,1
#   C = 0,1,2
#   D = 0,1
#   E = 0,1
#   F = 0,1
#   G = 0,1,2,3

# First record whether any options provided or not.

ifneq ($(A)$(B)$(C)$(D)$(E)$(F)$(G),)
  smtlib_options_provided = yes
else
  smtlib_options_provided = no
endif

# A. Whether to treat bit type as subtype 0,1 of integers or quotient of
# integers where 1 is true and all other values false.
# SMTLIB and Simplify
#
A = 0
ifeq ($(A),0)
  A_opt = -refine-bit-type-as-int-subtype 
endif
ifeq ($(A),1)
  A_opt = -refine-bit-type-as-int-quotient 
endif

# B. Whether to introduce term-level bit ops and rels or use
# prop-to-bit coercions
# SMTLIB and Simplify
#
B = 0
ifeq ($(B),0)
  B_opt = -refine-bit-eq-equiv 
#          -trace-intro-bit-ops-and-rels
endif
ifeq ($(B),1)
  B_opt = -bit-type-with-ite 
#          -trace-prop-to-bit-insertion
endif

# C. Heuristics for how one chooses a bit-valued or prop-valued version for
# each QFOL uninterpreted function to bool.
# C = 2 is OK only if B = 1.
# SMTLIB and Simplify

C = 0
ifeq ($(C),0)
  C_opt = # (no option) use bit-valued ops iff is instance in term position
endif
ifeq ($(C),1)
  C_opt = -bit-type-prefer-bit-vals  
endif
ifeq ($(C),2)
  C_opt = -bit-type-prefer-props     
endif


# D. Approach to handling refinement of array types. Whether to 
# constrain array element values on extended indexes or not.
# (Assert option to not constrain)
#
D = 0
ifeq ($(D),0)
  D_opt = # (no option) out of bounds element values constrained
endif
ifeq ($(D),1)
  D_opt = -refine-array-types-with-quotient 
endif


# E. Whether to axiomatise arrays and records before type refinement or
#    after
#
E = 0
ifeq ($(E),0)
  E_opt = -abstract-arrays-records-late 
endif
ifeq ($(E),1)
  E_opt = -abstract-arrays-records-early
endif


# F. Strategy for axiomatising records
#    Whether to use axiomisation that uses updates or constructors 
#
F = 0
ifeq ($(F),0)
  F_opt = -elim-record-constructors \
          -add-record-select-update-axioms \
          -abstract-record-selects-updates
endif
ifeq ($(F),1)
  F_opt = -abstract-record-updates \
          -add-record-select-constructor-axioms \
          -abstract-record-selects-constructors
endif

# G. Approach for handling extensionality.  Whether axioms are added and
#    if they are added, whether aliases are used for array and record 
#    equalities to help with quantifier instantiation algorithms.
#
#    NB: using the record constructor extensionality axioms (G = 2) only make
#    sense if option F = 1.
#

G = 0
ifeq ($(G),0)
  G_opt = # (no option)
endif
ifeq ($(G),1)
  G_opt = -add-array-extensionality-axioms \
          -add-record-eq-elements-extensionality-axioms \
          -identify-eqs-at-arr-rec-types
endif
ifeq ($(G),2)
  G_opt = -add-array-extensionality-axioms \
          -add-record-constructor-extensionality-axioms \
          -identify-eqs-at-arr-rec-types
endif
ifeq ($(G),3)
  G_opt = -add-array-extensionality-axioms \
          -add-record-eq-elements-extensionality-axioms \
          -use-array-eq-aliases \
          -use-record-eq-aliases \
          -identify-eqs-at-arr-rec-types
endif


ifeq ($(smtlib_options_provided),yes)
  smtlib_option_suffix =-$(A)$(B)$(C)$(D)$(E)$(F)$(G)
else
  smtlib_option_suffix =# (empty string)
endif

#----------------------------------------------------------------------------
# USER RULE ANALYSIS
#----------------------------------------------------------------------------
#-----------------------------------------------------------------------------
# Finding redundant rules
#-------------------------
#
ifdef FRR 
  frr_sfx=-frr
  frr_option=-find-redundant-rules
endif

#-----------------------------------------------------------------------------
# Auditing user rules
#--------------------

ifdef FURA  # Full User Rule Audit

  urule_audit_sfx=-fura
  urule_audit_options=\
     -do-rule-audit\
     -rule-audit-a\
     -rule-audit-b\
     -rule-audit-c\
     -rule-audit-d\
     -rule-audit-e\
     -csv-reports-include-goal-origins

else ifdef QURA # Quick User Rule Audit

  urule_audit_sfx=-qura
  urule_audit_options=\
     -do-rule-audit\
     -rule-audit-b\
     -csv-reports-include-goal-origins

else ifdef SURA # Single User Rule Audit
  urule_audit_sfx=-sura
  urule_audit_options=\
     -do-rule-audit\
     -rule-audit-c\
     -rule-audit-d\
     -rule-audit-e\
     -rule-audit-rule=$(SURA)\
     -csv-reports-include-goal-origins
endif

#----------------------------------------------------------------------------
# PROVER DRIVING OPTIONS
#----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# Getting proof summaries
#------------------------
# i.e. Unsat cores

ifdef PS
  pf_sum_sfx = -ps
  pf_sum_option = -smtlib2-unsat-cores
endif

#-----------------------------------------------------------------------------
# Enabling timeouts
#------------------

timeout_option = -ulimit-timeout=$(T)

ifdef T # Ulimit timeout (integer sec): applies to any file-level solver
  timeout_sfx = -t$(T)

else ifdef CT  # CVC timeout for API & file-level interfaces. 
               # CVC3: Units of 0.1sec.
               # CVC4: Units of 1 msec.
  timeout_sfx = -ct$(CT)
  timeout_option = -timeout=$(CT) 
  cvc3_timeout_flag = -timeout $(CT)   # Option for cvc3 command
  cvc4_timeout_flag = -tlimit=$(CT)    # Option for cvc4 command

# 14/12/09: timeout using shell timout.sh is currently flakey.
# Use with caution!
#
# else ifdef ST # Shell timeout (fixed pt sec): applies to any file-level solver
#  timeout_sfx = -st$(ST)
#  timeout_option = -shell-timeout=$(ST)

else ifdef WT  # Watchdog timeout (fixed pt sec): 
              # applies to any file-level solver
  timeout_sfx = -w$(WT)
  timeout_option = -watchdog-timeout=$(WT)

else ifdef ZT  # Z3 soft timeout.  Units of ms.
  timeout_sfx = -zt$(ZT)
  timeout_option = -smtlib2-soft-timeout=$(ZT)

endif

T=1# Delay setting default T to here so do not get suffix for default time.

#-----------------------------------------------------------------------------
# Exploit solver incrementality
#------------------------------

ifdef INC
  inc_sfx=-inc
  inc_option=-exploit-solver-incrementality
endif

#-----------------------------------------------------------------------------
# Fusing conclusions
#-------------------


ifdef U
  fuse_c_pfx = u
  fuse_concl_options =  -ctick
else
  fuse_c_pfx = f
  fuse_concl_options = -fuse-concls
endif



#-----------------------------------------------------------------------------
# Working directory
#-----------------
WKDIR=/tmp

#-----------------------------------------------------------------------------
# Generate and preserve prover files
#-----------------------------------

ifdef FWF
  WKDIR=work
  wkfiles_option = \
    -unique-working-files=false\
    -delete-working-files=false\
    -flat-working-files
else ifdef HWF
  WKDIR=work
  wkfiles_option = \
    -unique-working-files=false\
    -delete-working-files=false\
    -hier-working-files
endif

#-----------------------------------------------------------------------------
# Adding comments to working files
#---------------------------------
ifdef CMT
  cmt_option = -add-formula-descriptions
endif 

#----------------------------------------------------------------------------
# REPORT FILE OPTIONS
#----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# Output directory
#-----------------
OUTDIR=out


#-----------------------------------------------------------------------------
# Track trivial goals
#---------------------

ifdef NTG
  tg_sfx = -ntg
else
  tg_flag=-count-trivial-goals
endif

#----------------------------------------------------------------------------
# DEPRECATED OPTIONS
#----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# Solver call iteration
#----------------------

# RD only for use with api interface mode
# RC only for use with smtlib/simplify interface mode
# 
# api interface modes must also assert  -gstime-inc-setup in order
# for cumulative times to be reported correctly

# (Victor eventually should take care of these dependencies internally)

ifdef RD # Repeat Drive goal / goal slice
  repeat_sfx=-rd$(RD)
  repeat_option = -drive-goal-repeats=$(RD)
else ifdef RC # Repeat Check goal / goal slice
  repeat_sfx=-rc$(RC)
  repeat_option = -check-goal-repeats=$(RC)
else
  repeat_sfx=# Empty
  repeat_option=# Empty
endif






#=============================================================================
# Assembly of option lists
#=============================================================================
# SMTLIB2: Need to address abs abstraction
# By using -expand-exp-const, are assuming solver can handle non-lin arith.

report_root = $(fuse_c_pfx)-$@$(siv_sfx)$(tg_sfx)$(std_rlu_sfx)$(ti_sfx)$(lin_sfx)$(edm_sfx)$(enum_sfx)$(pf_sum_sfx)$(frr_sfx)$(urule_audit_sfx)$(inc_sfx)$(timeout_sfx)$(repeat_sfx)$(smtlib_option_suffix)$(SFX)

std_options = \
            $(unit_option) \
            $(goal_option) \
            $(fuse_concl_options)\
            $(timeout_option)\
            $(inc_option)\
            $(siv_flag)\
            $(tg_flag)\
            $(lin_opt)\
            $(enum_option)\
            $(pf_sum_option)\
            $(repeat_option)\
            $(std_rlu_option)\
            $(ti_option)\
            $(frr_option)\
            $(urule_audit_options)\
            -report=$(report_root)\
            -report-dir=$(OUTDIR)\
            -working-dir=$(WKDIR)\
            -decls=prelude.fdl\
            -unique-working-files\
            -delete-working-files\
            -rules=divmod.rul\
            -rules=prelude.rul\
            -ground-eval-exp\
            -expand-exp-const\
            -abstract-exp\
            -abstract-divmod\
            -gstime\
            -utick\
            -gtick\
            -longtick\
            -delete-rules-failing-typecheck\
            -read-directory-rlu-files\
            -read-unit-rlu-files \
            -expect-dir-user-rules-with-undeclared-ids\
            -use-alt-solver-driver \
            -echo-final-stats\
            -level=warning\
            $(edm_options)\
            $(wkfiles_option)\
            $(cmt_option)\
            $($*_options)

#----------------------------------------------------------------------------
# API CVC3 options
#----------------------------------------------------------------------------
# CVC3 r2.2 segfaults on a few tokeneer goals, so we need to exclude them.
# The relevant goals are tagged with "cvc3?" prefix in tokeneer-units.lis
#
# CVC3 r2.4.1 (Sep 11) and 2012-05-22 do not respect -stimeout flag and
# show divergent behaviour with Tokeneer (3rd release) VCs.
# e.g. enclave/enrolop 1
#
# Bad behaviour goes away with -quant-new flag (+quant-new by default).
# From cvc help: 
#
#    If this option is false, only naive instantiation is called
#
# -quant-new asserted by vct -cvc-old-quant-inst flag.
# A related cvc option is -quant-naive-num, set using vct option
# -cvc-quant-naive-num.  From cvc help, this is:
#
#   maximum number to call naive instantiation, 
#
# Default is 1000.

api_cvc3_options =  \
  -exclude-selected-goals\
  -active-unit-tags=cvc3\
  -ground-eval-exp=false\
  -abstract-exp=false\
  -bit-type \
  -bit-type-bool-eq-to-iff\
  -abstract-bit-valued-int-real-le\
  -abstract-arrays-records-late\
  -elim-array-constructors \
  -add-array-select-box-update-axioms\
  -abstract-array-box-updates\
  -gstime-inc-setup \
  -interface-mode=api \
  -strip-quantifier-patterns\
  -prover=cvc3\
  -cvc-old-quant-inst\
   $(EXTRAS)


#----------------------------------------------------------------------------
# API Yices options
#----------------------------------------------------------------------------

api_yices_options = \
  -abstract-arrays-records-late\
  -elim-record-constructors \
  -elim-array-constructors \
  -add-array-select-box-update-axioms\
  -abstract-array-box-updates\
  -strip-quantifier-patterns\
  \
  -gstime-inc-setup \
  -interface-mode=api \
  -prover=yices\
   $(EXTRAS)

#----------------------------------------------------------------------------
# SMTLIB Interface options
#----------------------------------------------------------------------------

smtlib_base_options = \
  -bit-type \
  -bit-type-bool-eq-to-iff\
  \
  -refine-types\
  -refine-int-subrange-type \
  \
  -elim-array-constructors\
  -add-array-select-box-update-axioms\
  -abstract-array-box-updates\
  -add-array-select-update-axioms\
  -abstract-array-select-updates\
  -abstract-array-types\
  \
  -abstract-record-types\
  \
  -abstract-bit-ops\
  -abstract-bit-valued-eqs\
  -abstract-bit-valued-int-real-le\
  -elim-bit-type-and-consts\
  \
  -abstract-reals\
  \
  -lift-quants \
  -strip-quantifier-patterns\
  -elim-type-aliases\
  \
  -interface-mode=smtlib \
  -smtlib-hyps-as-assums 


smtlib_options = \
  $(smtlib_base_options)\
  $(A_opt)\
  $(B_opt)\
  $(C_opt)\
  $(D_opt)\
  $(E_opt)\
  $(F_opt)\
  $(G_opt)\
  -logic=$(smtlib_logic)\
  $(EXTRAS)

#----------------------------------------------------------------------------
# SMTLIB2 Interface options
#----------------------------------------------------------------------------

smtlib2_base_options = \
  -refine-types\
  -refine-int-subrange-type \
  \
  -elim-array-constructors\
  -add-array-select-box-update-axioms\
  -abstract-array-box-updates\
  -add-array-select-update-axioms\
  -abstract-array-select-updates\
  -abstract-array-types\
  \
  -abstract-record-types\
  \
  -lift-quants \
  -strip-quantifier-patterns\
  \
  -interface-mode=smtlib2


smtlib2_aux_options = \
  $(smtlib2_base_options)\
  $(D_opt)\
  $(E_opt)\
  $(F_opt)\
  $(G_opt)\
  -logic=$(smtlib_logic)

smtlib2_options = \
  $(smtlib2_aux_options)\
  $(EXTRAS)

# Assume solver internally cannot handle complete merge of bool terms and
# formulas.  Assume only that it implements a Bool type.
# Needed for Alt-Ergo <= 0.95 and cvc3 <= 2.4.1.

smtlib2_strict_fol_options = \
  $(smtlib2_aux_options)\
  -bit-type-prefer-bit-vals\
  -bit-type\
  $(B_opt)\
  -abstract-bit-ops\
  -abstract-bit-valued-eqs\
  -abstract-bit-valued-int-real-le\
  $(EXTRAS)

#----------------------------------------------------------------------------
# Simplify interface options
#----------------------------------------------------------------------------
# Setting of symbolic constant threshold omitted.  Should be added later when
# needed

simplify_interface_options= \
  $(smtlib_options) \
  -refine-uninterpreted-types \
  -switch-types-to-int \
  -interface-mode=simplify \
  $(EXTRAS)


#----------------------------------------------------------------------------
# Isabelle options
#----------------------------------------------------------------------------
#
# These include a revised set of standard options.
#
# std_options removed:
#  $(lin_opt)\
#  $(repeat_option)\
#  -ground-eval-exp\
#  -abstract-exp\
#  -abstract-divmod\
#
#
# std options modified:
#  -rules=divmod.rul\
#  -rules=prelude.rul\
#
#
#
isabelle_options=\
  $(unit_option) \
  $(goal_option) \
  $(fuse_concl_options)\
  -fuse-unary-concls \
  $(timeout_option)\
  $(siv_flag)\
  $(tg_flag)\
  -units=$($*_units)\
  -report=$(report_root)\
  -report-dir=$(OUTDIR)\
  -prefix=$($*_prefix)\
  -decls=prelude.fdl\
  -unique-working-files\
  -rules=none\
  -rules=prelude.rul\
  -elim-enums\
  -gstime\
  -utick\
  -gtick\
  -longtick\
  -echo-final-stats\
  -level=warning\
\
  -abstract-arrays-records-late\
  -elim-record-constructors\
  -elim-array-constructors\
  -add-array-select-box-update-axioms\
  -abstract-array-box-updates\
  -refine-types\
  -refine-int-subrange-type\
  -interface-mode=isabelle\


# End of file

