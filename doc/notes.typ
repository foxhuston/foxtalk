#import "@preview/lovelace:0.3.0": *
#import "@preview/curryst:0.3.0": rule, proof-tree

#show "FoxTalk": smallcaps

#set document(
  title: [#emoji.fox FoxTalk Notes],
  author: "Fox Huston"
)
#set heading(numbering: "1.1")

// `otf-stix` is in the AUR...
// #show math.equation: set text(font: "Stix Two Math")

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


= Syntax

#let Database   = {$upright(sans("D"))$}
#let Aggregator = {$angle.l q, a angle.r$}
#let Handler    = {$angle.l q, h angle.r$}
#let Boot       = {$upright(sans("boot"))$}

First I wrote this whole thing in terms of sets, but when I started to get to the actual evaluation of the thing, I couldn't figure out how to write it. Since I'm working on a runtime for a language, I realized _I might as well write it as a language._ When all you have is a hammer...

#let term(t) = {$upright(sans(#t))$}
#let claim = term("claim")
#let when = term("when")
#let sstep = {$-->^(Handler)$}
#let made = {$==>^"gen"$}

#table(
  columns: (auto, auto, auto),
  align: (right, center, left),
  stroke: none,
  [$e$], [$::=$], [$claim o$],
  []   , [$|$]  , [$e; e | o$],
  [$o$], [$::=$], [$circle.small | bullet | angle.l o, o angle.r | o made o$]
)

The $bullet$ is the unit value, $circle.small$ represents any other constant the system might encounter but not care about (i.e. things that the handlers can create that #Database stores, but doesn't use in evaluation), the angle brackets are pairs, and $o made o$ represents the _provenance_ of a tuple. In this case, the following rule uses it to store which handler generated an object:

#show "E-Claim": smallcaps
#show "E-When": smallcaps

#proof-tree(
  rule(
    name: [E-Claim],
    [$Database tack claim o sstep Database union {o, Handler made o} tack bullet$]
  ),
)

Huh... Ok, so... a hypothetical E-When would behave exactly the same as E-Claim, and there's nothing that actually drives the evaluation of this language forward... How odd. I'm writing the relation as $sstep$, but this also needs to work for $Aggregator$ pairs. Duplicating every rule just to have that seems silly.

== Tick

#box( // Prevents breaking across columns/pages
  pseudocode-list[
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

= Algorithm
<algorithm>

// #let Database = {$upright(sans("D"))$}
// #let handlers   = {$upright("handlers")$}

#figure(
  caption: [Initial Definitions],
table(
  columns: (auto, auto),
  align: (right, left), // Huh. Could also be a function of row/column.
  stroke: none,
  [$Database$], [is a database of objects.],
  [$T subset.eq Database$], [is a subset of objects in the database.],
  [$t in Database$], [is an object in the database],
  [$Aggregator$], [is an aggregator.],
  [$a : T -> T$], [is a function that takes in a set of objects, and returns a set of objects.],
  [$q$], [is a _query_, such that:],
  [$T = Database(q)$], [is the set of objects in $Database$ that match $q$.],
  [$Boot$], [is a user-defined bootstrap handler that matches itself.],
  [$q_a$], [is the query that matches all aggregators in #Database.],
))

We define the state of the system at step $i$ recursively as:

$ Database_0       & = { Boot } \
  Database_(i + 1) & = union.big_(Aggregator in Database_(i)(q_a)) a(Database_(i)(q)) $

This whole system has the feeling of "facts in resonance;" that is, something exists in the database only so long as _something_ is asserting its existence. Deletions fall out of this system naturally: if some handler $h$ is asserting some (unique #footnote[If one or more other handlers were also asserting $t$, then the fact that $h$ stopped wouldn't mean anything for $Database_(i+1)$, since it is the union of _all_ of the outputs of all of the handlers.]) object $t$ on step $i$, and then it does not on step $i + 1$, then $t in.not Database_(i+1)$. Furthermore, on timestep $i+1$, any other handler that was depending on $t$ will have a new query result (lacking $t$), and it will recompute its output, which will possibly result in fewer tuples still. In this way, deletions ripple across steps, until the system comes to a new equilibrium.

== Handler Invariants / Contract
+ Handlers are #strike[idempotent]. I don't know what word to use, here, but what we're going for is to be able to write an optimization (below) that will not call the handlers if their input doesn't change.

== Refinement I: Handlers

Say that $Handler$ is a _handler_, where $h : t -> T$ takes in a single database object, and returns a set of database objects. Also say that $q_h$ is the query that returns all of the handlers in #Database. Then the state of the database at step $i$ is

$ Database_0       & = { Boot } \
  Database_(i + 1) & = (union.big_(Aggregator in Database_(i)(q_a)) a(Database_(i)(q))) union \
  & (union.big_(Handler in Database_(i)(q_h)) union.big_(t in D_(i)(q)) h(t)). $

Any aggregator can act as a handler (by only considering each element of its input one at a time), but a handler can never act as an aggregator. I've made this distinction because this will allow us to significantly optimize the evaluation stage, and it will also abstract a lot of bookeeping that I want the FoxTalk runtime to do that would otherwise be duplicated in many of the handlers.


= Evaluation
#let Aggregator = $angle.l q, a, s, t angle.r$
#let Handler    = $angle.l q, h, s, t angle.r$

In order to integrate with existing programs / libraries, Foxtalk needs a strong FFI, as well as aggregator and handlers designed to support them. I don't want to lose the nice abstractness of the system we have so far, though, so let's see how we might make some minimal edits. Firstly, let's redefine our handlers:

#box[ // box prevents breaking across pages (and presumably columns).
  #table(
    columns: (auto, auto),
    align: (right, left), // Huh. Could also be a function of row/column.
    stroke: none,
    [$Aggregator$], [is an aggregator.],
    [$Handler$], [is a handler.],
    [$s$], [is the _setup_ function from $() -> ()$.],
    [$c$], [is the _cleanup_ function, from $t -> ()$.]
  )
]

Intuitively, $s$ should perform any initialization that the handler needs in order to run itself.#footnote[While $s$ could be handled in a similar way to #(Boot)---by matching on itself and being run once---that would imply that the handler needed to dynamically regenerate its query in order to match on what it actually needed to match on. I think that would complicate the evaluation of the model without any other benefits. Perhaps I'll change my tune in the future.] The cleanup function $c$ is responsible for any kind of cleanup/resource freeing that needs to happen on a _per object_ basis.

#show "E-Step": smallcaps
#show "E-AggregatorStep": smallcaps
#show "E-HandlerStep": smallcaps
#show "E-NewAggregator": smallcaps
#show "E-NewHandler": smallcaps
#show "E-AggregatorFree": smallcaps
#show "E-HandlerFree": smallcaps

#figure(
  placement: auto,
  scope: "parent",
  // float: true,
  grid(
    columns: 2,
    gutter: 0.25in,
    proof-tree(
      rule(
        name: [E-AggregatorStep],
        [$Database_i tack.r bold(a) = a(Database_(i)(q))$],
        [$Aggregator in Database_i$]
      )
    ),
    proof-tree(
      rule(
        name: [E-HandlerStep],
        [$Database_i tack.r bold(h) = h(Database_(i)(q))$],
        [$Handler in Database_i$]
      )
    ),
    grid.cell(
      colspan: 2,
      proof-tree(
        rule(
          name: [E-Step],
          [$Database_i tack.r Database_(i+1)$],
          [$A = Database_(i)(q_a)$],
          [$bold(A) = union.big_(a in A) Database_i tack.r bold(a)$],
          [$bold(A)' = bold(A) without A$],
          [$forall Aggregator in bold(A'). s() = ()$],
          // linebreak(), // Come on, curryst...
          [$H = Database_(i)(q_h)$],
          [$bold(H) = union.big_(h in H) Database_i tack.r bold(h)$],
          [$bold(H)' = bold(H) without H$],
          [$forall Handler in bold(H'). s() = ()$],
          [$t_("removed") = Database_(i) without Database_(i+1)$],
          [$forall t in t_("removed"). "Broken!! Which handler do we call?"$],
          linebreak(),
          [$D_(i+1) = bold(a) union bold(h)$],
        )
      )
    )
  ), caption: [Evaluation Rules]
)
<evaluation-rules>

Symbols in @evaluation-rules:

- $A, bold(A), bold(A')$ are sets of aggregators. Specifically, $bold(A')$ is the set of _newly added_ aggregators at a given step.

// = Implementation / Optimization

// This is from a previous try at defining this thing. The algorithm above is, I
// think, the minimal way to describe the intended semantics of the system, but it
// doesn't necessarily lead to a straightforward, efficient Implementation. It also
// doesn't capture side-effectful computation, which we will certainly have: this will
// be used to process camera video streams, and render vulkan output.

// ...


// We start by defining $angle.l T, T angle.r in cal(H)_(Handler)$ to be an
// input-output pair for the handler #Handler. $R_i$ is the state of $R$ at
// timestep $i$, written as pairs of $angle.l cal(H), Database angle.r$, then
// the following algorithm computes $R_(i+1)$.


// #pseudocode-list[
//   + *for all* $Database_(i)(q_a)$
//     + $angle.l I', O' angle.r = cal(H)_(Handler)$
//     + $I = Database(q)$
//     + *if* $I != I'$ *then*
//       + $D' = D without O'$
//       + $O = f(I)$
//       + $cal(H)_(Handler) := angle.l I, O angle.r$
//       + $Database := Database' union O$
//     + *end if*
//   + *end for*
//   + *return* $angle.l cal(H), Database angle.r$
// ]

= Questions

- How do we respond to events from the OS?
- How do we do negative matches? That is, I want to be able to express "`/dev/video0` is a camera $and not$(`/dev/video0` is calibrated.)" (That is, that second triple does not yet exist in the database.)