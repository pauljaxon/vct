Verification conditions (VCs) are logical statements that, if proven,
formally guarantee the programs they are generated from satisfy
certain correctness conditions.  For example, the guarantees can be
for the absence of run-time errors such as overflows and array bounds
violations.

SPARK is an Ada subset which adds in an annotation language for
expressing program properties.

Victor reads in VCs generated from SPARK programs using the SPARK toolset
from Altran Praxis and AdaCore.  It translates these VCs into the input formats
of popular theorem provers, and can automatically run these provers on
the translated VCs.  Currently it translates to

  * the standard SMT-LIB language used by many SMT solvers,
  * API calls for the CVC3 and Yices SMT solvers,
  * theory files for the Isabelle/HOL interactive theorem prover.

The SPARK toolset includes tools for proving VCs.  Victor augments and
complements the capabilities of these tools.

For further information on Victor, visit

> http://homepages.inf.ed.ac.uk/pbj/spark/victor.html