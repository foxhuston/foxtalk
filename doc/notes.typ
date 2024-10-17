#import "@preview/lovelace:0.3.0": *
#import "@preview/curryst:0.3.0": rule, proof-tree

#show "FoxTalk": smallcaps

#set document(
  title: [#emoji.fox FoxTalk Notes],
  author: "Fox Huston"
)
#set heading(numbering: "1.1")
#set math.equation(numbering: "(1)")

// `otf-stix` is in the AUR...
// #show math.equation: set text(font: "Stix Two Math")

// Weird. Figures are the only referenceable objects, apparently, so you kind of
// have to jump through this hoop in order to be able to write @claim or
// whatever.
#let Claim(it) = figure(kind: "claim", supplement: "Claim")[
  #align(left, [*Claim #context counter(figure.where(kind: "claim")).display():* #it])
]

///// TITLE ////////////////////////////////////////////////////////////////////
#context text(24pt)[
  *#document.title*
]

#context for a in document.author [
  #a

]

#v(12pt)

///// ABSTRACT /////////////////////////////////////////////////////////////////

#text(10pt)[
  #smallcaps([Abstract:]) _blah blah_
]

#v(24pt)

///// DOCUMENT /////////////////////////////////////////////////////////////////

#show: rest => columns(2, rest)
#set par(justify: true)

#let Database   = {$upright(sans("D"))$}
#let Aggregator = {$angle.l q, a angle.r$}
#let Handler    = {$angle.l q, h angle.r$}
#let Boot       = {$upright(sans("boot"))$}

#let term(t)    = {$upright(sans(#t))$}
#let claim      = term("claim")
#let when       = term("when")
#let next       = term("next")
#let sstep      = {$-->^(Handler)$}
#let made       = {$==>^"gen"$}

= Terms & Definitions:

#table(
  columns: (auto, auto),
  align: (right, left), // Huh. Could also be a function of row/column.
  stroke: none,
  [$Database$], [is a database of objects.],
  [$Aggregator$], [is an aggregator.],
  [$Handler$], [is a handler.],
  [$q$], [is a _query_, such that:],
  [$T = Database(q)$], [is the set of objects in $Database$ that match $q$.],
  [$a : T -> T$], [is a function that takes in a set of objects, and returns a set of objects.],
  [$h : o -> T$], [is a function that takes in _a single_ object, and returns a set of objects.],
  [$q_a$], [is the query that matches all aggregators in #Database.],
  [$q_h$], [is the query that matches all handlers in #Database.],
)

== Denotationally

To find the state of the database at the $i$th step:

$ Database_0       & = { Boot } \
  Database_(i + 1) & = union.big_(Aggregator in Database_(i)(q_a)) a(Database_(i)(q)) $<set-theory-db>

This whole system has the feeling of "facts in resonance;" that is, something exists in the database only so long as _something_ is asserting its existence. Deletions fall out of this system naturally: if some handler $h$ is asserting some (unique #footnote[If one or more other handlers were also asserting $t$, then the fact that $h$ stopped wouldn't mean anything for $Database_(i+1)$, since it is the union of _all_ of the outputs of all of the handlers.]) object $t$ on step $i$, and then it does not on step $i + 1$, then $t in.not Database_(i+1)$. Furthermore, on timestep $i+1$, any other handler that was depending on $t$ will have a new query result (lacking $t$), and it will recompute its output, which will possibly result in fewer tuples still. In this way, deletions ripple across steps, until the system comes to a new equilibrium.

This is also the intended output-behavior of everything else that follows. That is, at the end of the $i$th step, whatever algorithm we come up with should produce a set equivalent to $Database_(i)$.

== Computing the Next DB State
In order to write algorithms to compute $Database_i$, we need to know the structure of certain objects in the database. We define

$ o ::= bullet | angle.l o, o angle.r | o made o, $

where $bullet$ represents any object in the database (and about which we can make no claims regarding its structure), the angle brackets represent pairs of objects, and $o_1 made o_2$ means that $o_2$ was generated because of (or by) $o_1$.

In the following pseudo-code, I used $=$ in the mathematical sense, and $:=$ as the "becomes" operator, which overwrites a (program) variable.

At its simplest, we have the following algorithm:

#box( // Prevents breaking across columns/pages
  pseudocode-list(booktabs: true, title: [$"Next"_0$])[
    + *function* $sans("next")(Database)$:
      + *let* $D'_a = emptyset$
      + *for each* $Aggregator in Database(q_a)$
        + $D'_(a) := D'_(a) union a(Database(q))$
      + *return* $D'_a$
  ]
)

which mirrors almost exactly @set-theory-db. However, we'll want to introduce some bookkeeping, as well as actually write in what to do with handlers:

#box( // Prevents breaking across columns/pages
  pseudocode-list(booktabs: true, title: [$"Next"_1$])[
    + *function* $sans("next")(Database)$:
      + *let* $D'_a = emptyset, D'_h = emptyset$
      + *for each* $Aggregator in Database(q_a)$
        + *for each* $r_a in a(Database(q))$
          + $D'_(a) := D'_(a) union {r_a, Aggregator made r_a}$
      + *for each* $Handler in Database(q_h)$
        + *for each* $r_q_h in Database(q)$
          + *for each* $r_h in h(r_q_h)$
            + $D'_(h) := D'_(h) union {r_h, Handler made r_h}$
      + *return* $D'_a union D'_h$
  ]
)

// == Accounting for Tuple Deletion

// #let free = term("free")

// #let Aggregator = {$angle.l q, a, free angle.r$}
// #let Handler    = {$angle.l q, h, free angle.r$}

// Let $q_"gen"$ be the query that finds objects like $o made o$, and let aggregators be written $Aggregator$ and handlers $Handler$. Then before returning the next version of #Database, we can run the following code:

// #box( // Prevents breaking across columns/pages
//   pseudocode-list[
//     + *function* $next(Database)$:
//       - $dots.v$
//       + *let* $Database' = Database'_a union Database'_h$
//       + *let* $R = Database without D'$
//       + *for each* $angle.l -, -, free angle.r made o in R(q_"gen")$
//         + $free(o)$
//       + *return* $D'$
//   ]
// )

= An Incremental #next

I think our actual goals for a performant algorithm are as follows:

1. Be able to know if $o$ matches $q$ as quickly as possible.
2. Run the handlers as little as possible.
3. Preserve the set-union semantics from @set-theory-db.

Given those goals, I think that we would want an algorithm that did some kind of "forward-querying"---that is, when some object $o$ is inserted, figure out what handlers will need to run on the next tick.

#Claim[The only time a handler needs to rerun is when its input changes.]<when-changed>

Let's set up an example: we can write a simple clock handler:

#let timeobj(t) = {$mono("time" = #t)$}

$ angle.l timeobj(\_), \ lambda(r) -> claim(timeobj(r + 1)) angle.r $

I have made _many_ leaps, here, but the gist is that this handler wants to find all objects in the db that look like $timeobj(\_)$, where the underscore matches any value, and the handler-function of that tuple claims there is a new object that says that "time is $r + 1$."

Looking back at @set-theory-db, we should expect that if, say, $timeobj(10) in Database_(i)$, then $timeobj(10) in.not Database_(i+1) and timeobj(11) in Database_(i+1)$, since the time-handler would no longer be asserting the previous object, and would have generated a new one. From a practical standpoint, if #Database is some kind of storage system, we need to _remove_ #timeobj(10) and _insert_ #timeobj(11) during the processing of this particular handler.

Anyways, back to @when-changed, it would be good if we had a way to---on insertion---know what handlers will need to run on the _next_ tick of the system. That is, if $o$ is new in $D_(i)$, then we know that in $D_(i+1)$, any handler that has a query $q$ that matches $o$ will need to be re-run#footnote[And just for that specific $o$ in the case of non-aggregating handlers.].

Changes are just a remove followed by an insert (as in the example), and we need to know what to do on removal: if $o in Database_(i) and o in.not Database_(i+1)$, then the output of any handler that matched on $o$ is invalid, and needs to be run again.

// "Stateful Database"
#let sdb = {$frak(D)$}

#let allAggs = {$cal(A)$}
#let aggtriple = {$angle.l q, a, I, O angle.r$}
#let aggtriplen(n) = {$angle.l q_(#n), a_(#n), I_(#n), O_(#n) angle.r$}

Let's take a first-pass at this, only using aggregators. First, let $sdb = angle.l allAggs, A angle.r$ where $allAggs$ is the set of all aggregators with their current inputs and outputs---that is, tuples of $aggtriple$, where $I$ is the set of values that $q$ matches, and $O$ is the set of values that $a(Database(q))$ would match. $A$ is the set of aggregators that need to run. (Note that $sdb_0 = angle.l {Boot}, {Boot} angle.r$).

#let matches = math.class("relation", "matches")

#box[
  #pseudocode-list[
    + *function* $next(sdb):$
      + *let* $Database'_a = emptyset, allAggs' = emptyset, A' = emptyset$
      + *for each* $aggtriple in sdb. A:$
        + *let* $O' = a(T)$
        - \
        -  _Additions:_
        + *for each* $o in O' without O:$
          + *for each* $aggtriplen(1) in allAggs$:
            + *if* $o in.not I_1 and q_1 matches o$:
              + $A' := A' union {angle.l q_1, a_1, I_1 union {o}, O_1 angle.r}$
        - \
        - _Removals:_
        + *for each* $o in O without O':$
          + *for each* $aggtriplen(2) in allAggs$:
            + *if* $o in.not I_2 and q_2 matches o$:
              + $A' := A' union {angle.l q_1, a, I_2 without {o}, O_2 angle.r}$
        - \
        + $Database'_a := Database'_a union O$
  ]
]


This is like, very broken (since it will insert multiple copies of some $Aggregator$ if it has a removal and an output...)







// = Algorithm
// <algorithm>

// // #let Database = {$upright(sans("D"))$}
// // #let handlers   = {$upright("handlers")$}

// #figure(
//   caption: [Initial Definitions],
// table(
//   columns: (auto, auto),
//   align: (right, left), // Huh. Could also be a function of row/column.
//   stroke: none,
//   [$Database$], [is a database of objects.],
//   [$T subset.eq Database$], [is a subset of objects in the database.],
//   [$t in Database$], [is an object in the database],
//   [$Aggregator$], [is an aggregator.],
//   [$a : T -> T$], [is a function that takes in a set of objects, and returns a set of objects.],
//   [$q$], [is a _query_, such that:],
//   [$T = Database(q)$], [is the set of objects in $Database$ that match $q$.],
//   [$Boot$], [is a user-defined bootstrap handler that matches itself.],
//   [$q_a$], [is the query that matches all aggregators in #Database.],
// ))

// We define the state of the system at step $i$ recursively as:

// $ Database_0       & = { Boot } \
//   Database_(i + 1) & = union.big_(Aggregator in Database_(i)(q_a)) a(Database_(i)(q)) $

// This whole system has the feeling of "facts in resonance;" that is, something exists in the database only so long as _something_ is asserting its existence. Deletions fall out of this system naturally: if some handler $h$ is asserting some (unique #footnote[If one or more other handlers were also asserting $t$, then the fact that $h$ stopped wouldn't mean anything for $Database_(i+1)$, since it is the union of _all_ of the outputs of all of the handlers.]) object $t$ on step $i$, and then it does not on step $i + 1$, then $t in.not Database_(i+1)$. Furthermore, on timestep $i+1$, any other handler that was depending on $t$ will have a new query result (lacking $t$), and it will recompute its output, which will possibly result in fewer tuples still. In this way, deletions ripple across steps, until the system comes to a new equilibrium.

// == Handler Invariants / Contract
// + Handlers are #strike[idempotent]. I don't know what word to use, here, but what we're going for is to be able to write an optimization (below) that will not call the handlers if their input doesn't change.

// == Refinement I: Handlers

// Say that $Handler$ is a _handler_, where $h : t -> T$ takes in a single database object, and returns a set of database objects. Also say that $q_h$ is the query that returns all of the handlers in #Database. Then the state of the database at step $i$ is

// $ Database_0       & = { Boot } \
//   Database_(i + 1) & = (union.big_(Aggregator in Database_(i)(q_a)) a(Database_(i)(q))) union \
//   & (union.big_(Handler in Database_(i)(q_h)) union.big_(t in D_(i)(q)) h(t)). $

// Any aggregator can act as a handler (by only considering each element of its input one at a time), but a handler can never act as an aggregator. I've made this distinction because this will allow us to significantly optimize the evaluation stage, and it will also abstract a lot of bookeeping that I want the FoxTalk runtime to do that would otherwise be duplicated in many of the handlers.


// = Evaluation
// #let Aggregator = $angle.l q, a, s, t angle.r$
// #let Handler    = $angle.l q, h, s, t angle.r$

// In order to integrate with existing programs / libraries, Foxtalk needs a strong FFI, as well as aggregator and handlers designed to support them. I don't want to lose the nice abstractness of the system we have so far, though, so let's see how we might make some minimal edits. Firstly, let's redefine our handlers:

// #box[ // box prevents breaking across pages (and presumably columns).
//   #table(
//     columns: (auto, auto),
//     align: (right, left), // Huh. Could also be a function of row/column.
//     stroke: none,
//     [$Aggregator$], [is an aggregator.],
//     [$Handler$], [is a handler.],
//     [$s$], [is the _setup_ function from $() -> ()$.],
//     [$c$], [is the _cleanup_ function, from $t -> ()$.]
//   )
// ]

// Intuitively, $s$ should perform any initialization that the handler needs in order to run itself.#footnote[While $s$ could be handled in a similar way to #(Boot)---by matching on itself and being run once---that would imply that the handler needed to dynamically regenerate its query in order to match on what it actually needed to match on. I think that would complicate the evaluation of the model without any other benefits. Perhaps I'll change my tune in the future.] The cleanup function $c$ is responsible for any kind of cleanup/resource freeing that needs to happen on a _per object_ basis.

// #show "E-Step": smallcaps
// #show "E-AggregatorStep": smallcaps
// #show "E-HandlerStep": smallcaps
// #show "E-NewAggregator": smallcaps
// #show "E-NewHandler": smallcaps
// #show "E-AggregatorFree": smallcaps
// #show "E-HandlerFree": smallcaps

// #figure(
//   placement: auto,
//   scope: "parent",
//   // float: true,
//   grid(
//     columns: 2,
//     gutter: 0.25in,
//     proof-tree(
//       rule(
//         name: [E-AggregatorStep],
//         [$Database_i tack.r bold(a) = a(Database_(i)(q))$],
//         [$Aggregator in Database_i$]
//       )
//     ),
//     proof-tree(
//       rule(
//         name: [E-HandlerStep],
//         [$Database_i tack.r bold(h) = h(Database_(i)(q))$],
//         [$Handler in Database_i$]
//       )
//     ),
//     grid.cell(
//       colspan: 2,
//       proof-tree(
//         rule(
//           name: [E-Step],
//           [$Database_i tack.r Database_(i+1)$],
//           [$A = Database_(i)(q_a)$],
//           [$bold(A) = union.big_(a in A) Database_i tack.r bold(a)$],
//           [$bold(A)' = bold(A) without A$],
//           [$forall Aggregator in bold(A'). s() = ()$],
//           // linebreak(), // Come on, curryst...
//           [$H = Database_(i)(q_h)$],
//           [$bold(H) = union.big_(h in H) Database_i tack.r bold(h)$],
//           [$bold(H)' = bold(H) without H$],
//           [$forall Handler in bold(H'). s() = ()$],
//           [$t_("removed") = Database_(i) without Database_(i+1)$],
//           [$forall t in t_("removed"). "Broken!! Which handler do we call?"$],
//           linebreak(),
//           [$D_(i+1) = bold(a) union bold(h)$],
//         )
//       )
//     )
//   ), caption: [Evaluation Rules]
// )
// <evaluation-rules>

// Symbols in @evaluation-rules:

// - $A, bold(A), bold(A')$ are sets of aggregators. Specifically, $bold(A')$ is the set of _newly added_ aggregators at a given step.

// // = Implementation / Optimization

// // This is from a previous try at defining this thing. The algorithm above is, I
// // think, the minimal way to describe the intended semantics of the system, but it
// // doesn't necessarily lead to a straightforward, efficient Implementation. It also
// // doesn't capture side-effectful computation, which we will certainly have: this will
// // be used to process camera video streams, and render vulkan output.

// // ...


// // We start by defining $angle.l T, T angle.r in cal(H)_(Handler)$ to be an
// // input-output pair for the handler #Handler. $R_i$ is the state of $R$ at
// // timestep $i$, written as pairs of $angle.l cal(H), Database angle.r$, then
// // the following algorithm computes $R_(i+1)$.


// // #pseudocode-list[
// //   + *for all* $Database_(i)(q_a)$
// //     + $angle.l I', O' angle.r = cal(H)_(Handler)$
// //     + $I = Database(q)$
// //     + *if* $I != I'$ *then*
// //       + $D' = D without O'$
// //       + $O = f(I)$
// //       + $cal(H)_(Handler) := angle.l I, O angle.r$
// //       + $Database := Database' union O$
// //     + *end if*
// //   + *end for*
// //   + *return* $angle.l cal(H), Database angle.r$
// // ]

= Questions

- How do we respond to events from the OS?
- How do we do negative matches? That is, I want to be able to express "`/dev/video0` is a camera $and not$(`/dev/video0` is calibrated.)" (That is, that second triple does not yet exist in the database.)