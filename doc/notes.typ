#import "@preview/lovelace:0.3.0": *

= Algorithm

First, some definitions:

#let Database = {$upright(sans("D"))$}
#let Handler  = {$angle.l q, f angle.r$}
#let Boot     = {$upright(sans("boot"))$}
#let handlers = {$upright("handlers")$}

#table(
  columns: (auto, auto),
  align: (right, left), // Huh. Could also be a function of row/column.
  stroke: none,
  [$Database_i$], [is the database at timestep $i$.],
  [$T subset Database_i$], [is a subset of objects in the database.],
  [$Handler$], [is a handler.],
  [$q$], [is a _query_, such that:],
  [$T = Database(q)$], [is the set of objects in $Database$ that match $q$.],
  [$f$], [is a function from $T arrow.r T$.],
  [$Boot$], [is a user-defined bootstrap handler that matches itself.]
)

We define the state of the system at step $i$ recursively as:

$ Database_0       & = & { Boot } \
  Database_(i + 1) & = & union.big_(Handler in handlers Database_i) f(Database_(i)(q)) $

Where $handlers Database$ returns the subset of handler objects in $Database$.

= Implementation / Optimization

This is from a previous try at defining this thing. The algorithm above is, I
think, the minimal way to describe the intended semantics of the system, but it
doesn't necessarily lead to a straightforward, efficient Implementation. It also
doesn't capture side-effectful computation, which we will certainly have: this will
be used to process camera video streams, and render vulkan output.

...


We start by defining $angle.l T, T angle.r in cal(H)_(Handler)$ to be an
input-output pair for the handler #Handler. $R_i$ is the state of $R$ at
timestep $i$, written as pairs of $angle.l cal(H), Database angle.r$, then
the following algorithm computes $R_(i+1)$.


#pseudocode-list[
  + *for all* $Handler in handlers Database_(i)$
    + $angle.l I', O' angle.r = cal(H)_(Handler)$
    + $I = Database(q)$
    + *if* $I != I'$ *then*
      + $D' = D without O'$
      + $O = f(I)$
      + $cal(H)_(Handler) := angle.l I, O angle.r$
      + $Database := Database' union O$
    + *end if*
  + *end for*
  + *return* $angle.l cal(H), Database angle.r$
]
