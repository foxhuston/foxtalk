#import "@preview/lovelace:0.3.0": *
#import "@preview/curryst:0.3.0": rule, proof-tree
#import "@preview/quick-maths:0.1.0": shorthands

#show "FoxTalk": smallcaps

#set document(
  title: [#emoji.fox FoxTalk Notes],
  author: "Fox Huston"
)
#set page("us-letter", margin: ( x: 0.75in ))
#set heading(numbering: "1.1")
#set math.equation(numbering: "(1)")
#set enum(numbering: "1.a.")

#let fox(m) = {text(fill: orange)[#emoji.fox #m]}

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

#let matches = math.class("relation", math.accent(sym.tilde, "?"))

I'll write the relation $q matches o$ to mean that the query $q$ matches the object $o$. #text(fill: luma(50%))[(TODO: this is pretty rough, but I just needed something. I could also be using $sigma_(phi)$ from relational algebra, and maybe saying something like $o in sigma_(phi)(Database)$? $phi$ is the same as $q$ in this case.)]

Things we can take advantage of:
- If $q matches o$ in step $i$, then $q matches o$ in step $i + 1$.

== Algorithm

#show: shorthands.with(
  ($<<$, $angle.l$),
  ($>>$, $angle.r$),
)

#let object = $upright(sans(o))$
#let Objects = $upright(sans(O))$
#let Aggregator = {$<< q, a, S, I, O >>$}
#let Handler = text(fill: red)[HANDLER]
// #let Handler = {$<< q, a, I, M, O >>$}
#let dirty(h) = {$#h^(bullet)$}
// SDB Destructured
#let sdbd = {$angle.l A, R angle.r$}

/ Objects: are things that the database operates on. Individuals are written #object, and a set of objects are written #Objects.
/ Queries: match (or do not match) objects, written $q matches object$.
/ Aggregators: Written $Aggregator$, where $q$ is a query, $a$ is a function from $Objects times S -> Objects times S$, $S$ is a set of opaque _sideband_ objects, $I$ is the set of _input_ objects, and $O$ is the set of _output_ objects.
// / Handlers: Written $Handler$, where $h : object -> Objects$, and $M$ is a set of $angle.l object, object angle.r$ pairs, mapping objects in $I$ to objects in $O$.
/ Dirty: Handlers are written $dirty(angle.l - angle.r)$.
/ The Database: is written $sdb = sdbd$, where $A$ is the set of registered aggregators, $H$ is the set of registered handlers, and $R$ the _refcount_ map of $object times NN$.


#let insert = "insert"
#show "insert": r => $upright(sans(#r))$

#let swap = "swap"
#show "swap": r => $upright(sans(#r))$

#let remove = "remove"
#show "remove": r => $upright(sans(#r))$

#let tick = "tick"
#show "tick": r => $upright(sans(#r))$

#pseudocode-list(booktabs: true,title: [$insert: sdb -> o -> sdb$])[
  + *function* $insert(sdbd, o)$:
    + *let* $A' = nothing, R' = R$
    + *if* $<< o, n >> in R$ (where $n$ is any natural number):
      + $R' := (R' without {<< o, n >>}) union {<< o, n + 1 >>}$
      + *return* $<<A, H, R'>>$
    + *else*:
      + *for each* $Aggregator in A$:
        + *if* $o in.not I and q matches o$:
          + $A' := A' union dirty(<< q\, a\, S\, I union {o}\, O >>)$
        + *else*
          + $A' := A' union Aggregator$

      + *return* $<< A', R union {<<o, 1>>} >>$
]

Addition simply increments the refcount in $sdb$ and, for each handler that matches (and that doesn't already have $o$ in its input set), it adds $o$ to that handler's input set and marks it dirty. Removals are a bit more complex:

#pseudocode-list(booktabs: true, title: [$remove: sdb -> sdb$])[
  + *function* $remove(sdb, o)$:
    + *let* $R' = sdb_(R)$
    + *let* $sdb' = sdb$
    + *if* $<<o, n>> in R'$:
      + *if* $n > 1$:
        + $R' := (R' without {<<o, n>>}) union {<<o, n-1>>}$
      + *else*
        + $R' := R' without {<<o, n>>}$
        + *let* $sdb' := remove_(a)(sdb', o)$
    + *return* $<<sdb'_(A), sdb'_(H), R'>>$
]

Next, we write the removal function on a specific aggregator:

#pseudocode-list(booktabs: true, title: [$remove_(a): sdb -> o -> sdb$])[
  + *function* $remove_(a)(sdbd, o)$:
    + *let* $A' = nothing$
    + *for each* $Aggregator in A$:
      + *if* $o in I$:
        + $A' := A' union {dirty(<<q\, a\, S\, I without {o}\, O>>)}$
      + *else*:
        + $A' := A' union Aggregator$


    + *return* $<< A', H, R >>$
]

Next, we have to write the tick function, that actually processes everything that happened with adds and removes.

#pseudocode-list(booktabs: true, title: [$tick: sdb -> sdb$])[
    + *function* $swap_(a)(sdbd, a, a')$:
      + *return* $<< (A without {a}) union {a'}, R>>$
    - \
  + *function* $tick(sdbd)$:
    + *let* $sdb' = sdbd$
    + *for each* $dirty(Aggregator) in A$:
      + $O', S' = a(I, S)$
      + $"Ins" = O' without O$
      + $"Rem" = O without O'$
      + $sdb' := swap_(a)(sdb', dirty(Aggregator),$
      + $"                     " <<q, a, S', I, O' >>)$
      + *for each* $o in "Ins"$:
        + $sdb' := insert(sdbd, o)$
      + *for each* $o in "Rem"$:
        + $sdb' := remove(sdbd, o)$
    + *return* $sdb'$
]

== Specialization: Non-Aggregating Handlers

Using the above abstractions, we can define a non-aggregating handler function that selectively calls its inner function $h$ based on the state of individual input objects. This will allow us to efficiently write operations on _system-level resources_, such as Vulkan instances or cameras.


#let nonAgg = "nonAgg"
#show "nonAgg": r => $upright(sans(#r))$


#box[
  #pseudocode-list(booktabs: true, label: <nonagg>, title: [$nonAgg: (o -> Objects) -> Objects times S -> Objects times S$])[
    + *function* $nonAgg(h)$:
      + *return function* $(I, S)$:
          + *let* $"Ins" = { o | o in I and exists.not <<o', p>> in S. o = o'}$
          + *let* $"Rem" = { <<o, p>> | <<o, p>> in S and o in.not I }$
          + *let* $S' = S without "Rem"$
          - \
          + *for each* $o in "Ins"$:
            + *for each* $p in h(o)$:
              + $S' := S' union {<<o, p>>}$
          - \
          + $O' = { p | <<o, p>> in S'}$
        + *return* $<< O', S'>>$
  ]
]

== Examples

#set math.equation(numbering: none)

Let's work through an example with for the non-aggregating handler. Say we start with the empty database with one handler, $sdb_0 = << { H }, {} >>$, and say that $H = << q, nonAgg(h), {}, {}, {}>>$. Let's also say that $q$ matches $o_1, o_2,$ and $o_3$. Let's also define
$ h(o) = cases( {o_2, o_3} &"if" o = o_1,
                {o_5}      &"if" o = o_2,
                nothing    &"otherwise") $


=== Adding an object
So if we call: $insert(sdb_0, o_1)$, then:

#let New(what) = {text(fill: red)[#what]}

#let dbstate(a, r) = {$lr(angle.l
    #grid(
      columns: 1,
      rows: 20pt,
      align: horizon,
      $lr(\{, size:#150%) #a lr(\}, size: #150%)$,
      $lr(\{, size: #150%) #r lr(\}, size: #150%)$
    ) angle.r)$}

+ $R$ is empty, so we take the *else* case on line 6 of #insert. We only have one handler in $A$, which is $H$. By definition, the predicate on line 8 holds, so we change the state of the databse to be: $
sdb_1 = dbstate(dirty(<< q\, nonAgg(h)\, {}\, {o_1}\, {}>>), << o_1\, 1 >>) $

+ Next, we call tick, which will find our #dirty([H]), and call its handler function with the parameters $({o_1}, {})$. The handler function is the anonymous function defined in nonAgg starting on line 2, and we can take a look at what it does:

  + Ins is every $o$ in the input set where $o$ does not appear in the left-hand side of any tuple in $S$. Since $S$ is empty on this first step, $"Ins" = {o_1}$.

  + Rem looks for any pair in $S$ where the left-hand side of the pair _does not_ appear in $I$, but again $S$ is empty, and so $"Rem" = {}$.

  + Our new sideband state, $S' = {} without {} = {}$.

  + Next, we loop through each new input object $o$ in Ins. We call $h$ on this object, and expect a set of objects back. We store each individual output together with its triggering input in $S'$, so that by line 9, $S' = {<<o_1, o_2>>, <<o_1, o_3>>}$. (We defined $h(o_1)$ above.)

  + Finally, we compute the new output set of this handler, which is the union of all of the right-hand sides in $S'$, in this case the set ${ o_2, o_3 }$.

  + This function finally returns, and we're back on line 6 of tick.

+ We now have $O'$ and $S'$ as above, and we compute new insert sets and removal sets based on the previous output set of this handler (lines 7 and 8). In this case, $"Ins" = O'$ and $"Rem" = {}$.

+ On line 9, We unmark $H$ as dirty, and store its new sideband and output sets. At this point, say that $
    S &= {<<o_1, o_2>>, <<o_1, o_3>>}, \
    sdb_2 &= dbstate(<< q\, nonAgg(h)\, S\,  {o_1}\, {o_2, o_3}, << o_1\, 1 >>)
$

+ For each inserted object, we call insert: $
    sdb_3 &= insert(sdb_2, o_2) \
          &= dbstate(dirty(<< q\, nonAgg(h)\, S\, {o_1, New(o_2)}\, {o_2, o_3}), << o_1\, 1 >>\, New(<<o_2\, 1>>))
$

+ On the next tick, we go through the same process again. Inside of nonAgg, $"Ins" = { o_2 }$ and $"Rem" = {}$. Our new $S = {<<o_1, o_2>>, <<o_1, o_3>>, New(<<o_2\, o_5>>)}$, and $O = { o_2, o_3, New(o_5) }$. Our current DB state after this tick is: $
    sdb_4 &= dbstate(<< q\, nonAgg(h)\, S\, {o_1, o_2}\, {o_2, o_3, New(o_5)}, << o_1\, 1 >>\, <<o_2\, 1>>)
$

=== Removing an object.


== Adding Handlers

To add a handler $h'$, we have to set its initial input set to: $ { o | o in union.big_{a in A} a. O and h'. q(o)}. $ This is an expensive operation, but I believe that the insertion and deletion of handlers will be far less frequent than the actual tick operations, so that's what I've optimized for.









= Questions

- How do we respond to events from the OS?
- How do we do negative matches? That is, I want to be able to express "`/dev/video0` is a camera $and not$(`/dev/video0` is calibrated.)" (That is, that second triple does not yet exist in the database.)