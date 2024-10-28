#import "@preview/lovelace:0.3.0": *
#import "@preview/curryst:0.3.0": rule, proof-tree
#import "@preview/quick-maths:0.1.0": shorthands
#import "@preview/fletcher:0.5.2" as fletcher: diagram, node, edge

#show "FoxTalk": smallcaps
#show "Reactor": smallcaps

// There *must* be a way to set the default emoji font...
#show emoji.face.meh: set text(font: "JoyPixels")
#show emoji.fox: set text(font: "JoyPixels")

#set document(
  title: [#emoji.fox FoxTalk Notes],
  author: ("Fox Huston", "lexi Huston")
)
// #set page("us-letter", margin: ( x: 0.75in ))
#set page("us-letter")
#set text(size: 10pt)
#set heading(numbering: "1.1")
// #set math.equation(numbering: "(1)")
#set enum(numbering: "1.a.")

#let fox(m) = {text(fill: orange)[#emoji.fox #m]}

// `otf-stix` is in the AUR...
#show math.equation: set text(font: "Stix Two Math")

// Weird. Figures are the only referenceable objects, apparently, so you kind of
// have to jump through this hoop in order to be able to write @claim or
// whatever.
#let Claim(it) = figure(kind: "claim", supplement: "Claim")[
  #align(left, [*Claim #context counter(figure.where(kind: "claim")).display():* #it])
]

#let Def(it) = figure(kind: "definition", supplement: "Definition")[#box(width: 100%, [
  #align(left, [
    *Definition #context counter(figure.where(kind: "definition")).display():*])
    #it
])]

///// TITLE ////////////////////////////////////////////////////////////////////
#context text(24pt)[
  *#document.title*
]

#context for a in document.author [
  #a #h(2cm)
]

#v(12pt)

#datetime.today().display()

#v(12pt)

///// ABSTRACT /////////////////////////////////////////////////////////////////

// #text(10pt)[
//   #smallcaps([Abstract:]) _blah blah_
// ]

// #v(24pt)

///// DOCUMENT /////////////////////////////////////////////////////////////////

#show: rest => columns(2, rest)
#set par(justify: true)

#show: shorthands.with(
  ($<<$, $angle.l$),
  ($>>$, $angle.r$),
)

#let Database   = {$upright(sans("D"))$}

= #fox[Todo]
- Redo #(sym.section)2. The Denotation section is good, but all the definitions are out of date.
- Settle on whether the things in $A$ are "aggregators," "handlers," or "programs." I'm honestly leaning towards "programs," since I think that captures more closely what the intention of each of these things are. Anyways, I need to update accordingly.
- Unify the logical clock examples. In #(sym.section)3 I write triples like `time = 0`, while in the actual worked-out example, I write just `t0`.
- Flesh out/clean up the text for the examples. I've written out selected snapshots of the database state for my own notes, but it would be good to write in some guidance.
- Come up with a better "matches" operator #emoji.face.meh

= Introduction

I came across Dynamicland @dynamicland recently, and was pretty enamoured with it---although I'll confess to being pretty enamoured with many of the things Brett Victor and co. come up with. There are a lot of interesting pieces to this system, but this particular document is a collection of my thoughts around trying to understand the core semantics of their programming language, Realtalk. It took some thinking, but after relentlessly paring down what I think the central tenant of this system could be, I've come up with something that I think is simple and elegant.

As an outline, in @denotational-semantics, I'll write out what I think are a reasonable denotational semantics of the system, and in @algorithm, I'll write out an algorithm to actually implement these sementics along with thoughts about the tradeoffs it makes. Finally, in @implementation, I'll write out some details about our choices actually implementing the system.

When reading about Realtalk, one of the things that stuck out to me was that they refer to their system as (having) a database, and things are either _in_ the database, or not. _Programs_ can be added or removed from the running system, and these programs are written as either a set of `claim`s, e.g.
```
  Claim "lexi" is a "husky".
```
or as a `when` clause, e.g.
```
  When /x/ is a "husky":
    Wish (x) is "cool".
  End
```

This, at first, felt like some kind of graph database, but that didn't really capture the actual semantics of what it seemed like to work with the system. It has, from what I've seen, this feeling of "facts in resonance." That is, the database would have the fact that `"lexi" is "cool"` only as long as `"lexi" is a "husky"` is present in the database. If the first claim is removed, then any claim that was generated from it will also be removed. #footnote[Incidentally, this has the feel of dependent pairs to me: if there is no witness, there can't be anything that follows.] This has a ripple effect throughout the system: from the previous example, there may have been `When` clauses that matched on "things that are cool," and anything _those_ generated would also need to be cleaned up.

== Denotationally<denotational-semantics>

#let matches = math.class("relation",
  math.attach(sym.harpoon, t: [?])
)

#let isa = math.class("relation", $" ":" "$)

Distilling all of this, it suggests a set-theoretic model of the state of the database over steps. In order to write this, I'll first need to define some terms:

#table(
  columns: (0.5fr, 1fr),
  stroke: none,
  align: (right, left),
  [$Database$], [The abstract database. This contains many:],
  [$o$], [An abstract object that can be in $Database.$],
  [$q$], [An abstract _query_, along with a relation],
  [$matches isa q times o$], [The _matches_ relation, between queries and objects.],
  [$h isa {o} -> {o}$], [A _handler function_ that takes as input a set of (input) objects, and returns a set of (claimed) objects.],
  [$<< q, h >>$], [A _handler_: a pair containing a query and an associated handler function.],
  [$H$], [The set of handlers.]
)


Then, to find the state of the database at the $i$th step:

#Def[
$ Database_0       & = {} \
  Database_(i + 1) & = union.big_(<< q, h >> in H) h({ o | o in Database_(i) and q matches o}) $
]<def-abstract-db>

// This whole system has the feeling of "facts in resonance;" that is, something exists in the database only so long as _something_ is asserting its existence. Deletions fall out of this system naturally: if some handler $h$ is asserting some (unique #footnote[If one or more other handlers were also asserting $t$, then the fact that $h$ stopped wouldn't mean anything for $Database_(i+1)$, since it is the union of _all_ of the outputs of all of the handlers.]) object $t$ on step $i$, and then it does not on step $i + 1$, then $t in.not Database_(i+1)$. Furthermore, on timestep $i+1$, any other handler that was depending on $t$ will have a new query result (lacking $t$), and it will recompute its output, which will possibly result in fewer tuples still. In this way, deletions ripple across steps, until the system comes to a new equilibrium.

This is also the intended output-behavior of everything else that follows. That is, at the end of the $i$th step, whatever algorithm we come up with should produce a set equivalent to $Database_(i)$---although hopefully much more efficiently!

== Example: A Logical-Clock Handler
#let timeobj(t) = {$mono("time" ) #t$}
#let inc = $mono("inc")$

For this example, let's set up some objects and queries, which are strings defined by the following grammar: $
  e &::= timeobj(n) \
  q &::= timeobj(n) | timeobj("_")
$
And $n in NN$. The query objects $q$ match either an exact time object (e.g. $timeobj(0), timeobj(42)$) or _any_ time object ($timeobj("_")$).

To allow our example to actually do something, let's define an operation #inc on $e$ such that $
  inc(timeobj(n)) = timeobj((n+1)).
$

Finally, let $h(O) = { inc(o) | o in O}.$
If we set up our database so that $Database_0 = {timeobj(0)}$ and $H = {<< timeobj("_"), h>>}$, then by @def-abstract-db, we would expect $
  D_(1) &= union.big_(<< q',h' >> in H) h'({o | o in D_(0) and q' matches o}) \
    &= h({o | o in {timeobj(0)} and q matches o}) \
    &= h({timeobj(0)}) \
    &= {inc(o) | o in {timeobj(0)}} \
    &= {inc(timeobj(0))} \
    &= {timeobj(1)}

$

Note that @def-abstract-db takes care of the removal of the previous time object, and indeed sets up an "oscillating" handler response---since our example handler matches on the kinds of objects it produces, there will always be a new state of the database in $D_(i+1)$. This does not need to be the case, and in fact, is probably usually not! Most of the time, I expect these to operate more like stages in a pipeline, but it's important to note that this behavior is available, should programmers want it.


= An Incremental Algorithm <algorithm>
#fox[Reviewed up to here #emoji.sparkles]

I think our actual goals for a performant algorithm are as follows:

1. Be able to know if $o$ matches $q$ as quickly as possible.
2. Run the handlers as little as possible.
3. Preserve the set-union semantics from @def-abstract-db.

I'll punt on the first goal, here, and claim that our algorithm, Reactor, can operate efficiently without actually knowing what the objects $o$ are, or how the queries $q$ work. Given the other two goals, I think that we would want an algorithm that does some kind of "forward-querying," for lack of a better term. The most straightforward thing to do would be to implement @def-abstract-db more or less directly: that is, at each step, for each handler, run its query on the database, call the handler functions on the result set, and then union all of those sets together to form the new database.

This would not be very efficient. Instead, I'll start with the... let's say _definitional observation_ that:

#Claim[The only time a handler needs to rerun is when its input changes.]<when-changed>

We would expect that in most cases during a running system, inserting or removing a fact wouldn't effect _every_ handler registered in the system; so we should expect performance gains if we can figure out exactly which handlers were effected at each step.

Furthermore, to simplify writing these handlers, Reactor needs to be able to minimize the calls to handlers while _removing_ objects as well. That is, if the removal doesn't affect the input set of some $h$, it doesn't need to be called (by @when-changed).

Anyways, back to @when-changed, it would be good if we had a way to---on insertion---know what handlers will need to run on the _next_ tick of the system. That is, if $o$ is new in $D_(i)$, then we know that in $D_(i+1)$, any handler that has a query $q$ that matches $o$ will need to be re-run#footnote[And just for that specific $o$ in the case of non-aggregating handlers.].

Changes are just a remove followed by an insert (as in the example), and we need to know what to do on removal: if $o in Database_(i) and o in.not Database_(i+1)$, then the output of any handler that matched on $o$ is invalid, and needs to be run again.

// "Stateful Database"
#let sdb = {$frak(D)$}

#let allAggs = {$cal(A)$}
#let aggtriple = {$angle.l q, a, I, O angle.r$}
#let aggtriplen(n) = {$angle.l q_(#n), a_(#n), I_(#n), O_(#n) angle.r$}

Let's take a first-pass at this, only using aggregators. First, let $sdb = angle.l allAggs, A angle.r$ where $allAggs$ is the set of all aggregators with their current inputs and outputs---that is, tuples of $aggtriple$, where $I$ is the set of values that $q$ matches, and $O$ is the set of values that $a(Database(q))$ would match. $A$ is the set of aggregators that need to run. (Note that $sdb_0 = angle.l {"Boot"}, {"Boot"} angle.r$).

I'll write the relation $q matches o$ to mean that the query $q$ matches the object $o$.

Things we can take advantage of:
- If $q matches o$ in step $i$, then $q matches o$ in step $i + 1$.

== Algorithm

#let object = $upright(sans(o))$
#let Objects = $upright(sans(O))$
#let Aggregator = {$<< q, a, S, I, O >>$}
#let Handler = text(fill: red)[HANDLER]
// #let Handler = {$<< q, a, I, M, O >>$}
#let dirty(new: false, ..h) = {
  let bod = h.pos().join(", ")
  $#bod^#if new { text(fill: red)[$bullet$]} else {$bullet$}$
}
// SDB Destructured
#let sdbd = {$angle.l A, R angle.r$}

/ Objects: are things that the database operates on. Individuals are written #object, and a set of objects are written #Objects.
/ Queries: match (or do not match) objects, written #box[$q matches object$].
/ Aggregators: Written $Aggregator$, where $q$ is a query, $a$ is a function from $Objects times S -> Objects times S$, $S$ is an opaque _sideband_ object, $I$ is the set of _input_ objects, and $O$ is the set of _output_ objects.
// / Handlers: Written $Handler$, where $h : object -> Objects$, and $M$ is a set of $angle.l object, object angle.r$ pairs, mapping objects in $I$ to objects in $O$.
/ Dirty: handlers are written $dirty(angle.l - angle.r)$.
/ The Database: is written $sdb = sdbd$, where $A$ is the set of registered aggregators, $H$ is the set of registered handlers, and $R$ the _refcount_ map of $object times NN$.

Also note that, given some variable that represents a tuple, I will use $a.q$ to e.g. refer to the "$q$" field of $a$.


#let insert = "insert"
#show "insert": r => $upright(sans(#r))$

#let swap = "swap"
#show "swap": r => $upright(sans(#r))$

#let remove = "remove"
#show "remove": r => $upright(sans(#r))$

#let tick = "tick"
#show "tick": r => $upright(sans(#r))$

#pseudocode-list(booktabs: true, title: [$insert: sdb -> object -> sdb$])[
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
  + *function* $remove(sdbd, o)$:
    + *let* $A' = A$
    + *let* $R' = R$
    + *if* $<<o, n>> in R'$:
      + *if* $n > 1$:
        + $R' := (R' without {<<o, n>>}) union {<<o, n-1>>}$
      + *else*
        + $R' := R' without {<<o, n>>}$
        + $<< A', R' >> := remove_(a)(<< A', R' >>, o)$
    + *return* $<<A', R'>>$
]

Next, we write the removal function on a specific aggregator:

#pseudocode-list(booktabs: true, title: [$remove_(a): sdb -> object -> sdb$])[
  + *function* $remove_(a)(sdbd, o)$:
    + *let* $A' = nothing$
    + *for each* $Aggregator in A$:
      + *if* $o in I$:
        + $A' := A' union {dirty(<<q\, a\, S\, I without {o}\, O>>)}$
      + *else*:
        + $A' := A' union Aggregator$


    + *return* $<< A', R >>$
]

Next, we have to write the tick function, that actually processes everything that happened with adds and removes.

#pseudocode-list(booktabs: true, title: [$tick: sdb -> sdb$])[
    + *function* $swap_(a)(sdbd, a, a')$:
      + *return* $<< (A without {a}) union {a'}, R>>$
    - \
  + *function* $tick(sdbd)$:
    + *let* $sdb' = sdbd$
    + *for each* $dirty(Aggregator) in A$:
      + $<< O', S' >> = a(I, S)$
      + $"Ins" = O' without O$
      + $"Rem" = O without O'$
      + #line-label(<tick-swap>) $sdb' := swap_(a)(sdb', dirty(Aggregator),$
      + $"                     " <<q, a, S', I, O' >>)$
      + *for each* $o in "Ins"$:
        + $sdb' := insert(sdb', o)$
      + *for each* $o in "Rem"$:
        + $sdb' := remove(sdb', o)$
    + #line-label(<tick-return>) *return* $sdb'$
]

Finally, we need a way to insert and remove aggregators themselves.

#pseudocode-list(booktabs: true, title: [$insert_a: sdb -> << q, a, S, O >> -> sdb$])[
  + *function* $insert_(a)(sdbd, << q, A, S, O >>)$:
    + *let* $A' = A, R' = R$
    + *for each* $o in O$:
      + $<< A', R' >> := insert(<<A', R'>>, o)$
    + #line-label(<ins-world-o>) *let* $cal(O) = O union (union.big_(a in A) a.O$)
    + #line-label(<ins-world-i>) *let* $cal(I) = { o | o in cal(O) and q matches o}$
    + #line-label(<agg-construction>) *let* $a' = dirty(<< q, a, S, cal(I), O >>)$
    + *return* $<< A union {a}, R >>$
]

Since we have no "central store" in this model, we need to generate the current, unified state of the world, which is just the union of all of the outputs of all of the existing handlers. We filter this set by what the new query matches, and then use that as the input set to a dirty version of the tuple. Note that the input object is not an aggregator proper, since it does not have the input or output sets. Instead, the aggregator is constructed on @agg-construction.

This is by far the slowest operation in this entire system, but adding and removing handlers will happen _far_ less often than the existing handlers needing to evaluate changes to their input sets, so that's what this algorithm optimizes for.

#pseudocode-list(booktabs: true, title: [$insert_a: sdb -> a -> sdb$])[
  + *function* $remove_(a)(sdbd, Aggregator)$:
    + *let* $A' = A without {Aggregator}$
    + *let* $R' = R$
    + *for each* $o in O$:
      + $<< A', R' >> := remove(<< A', R' >>, o)$

    + *return* $<< A', R' >>$
]


== Example: Aggregator
#set math.equation(numbering: none)

#let New(..what) = {text(fill: red)[#(what.pos().join(", "))]}
#let Removed = New($bracket.b$)

#let dbstate(a, r) = {$lr(angle.l
    #grid(
      columns: 1,
      rows: 20pt,
      align: horizon,
      $lr(\{, size:#150%) #a.join(", ") lr(\}, size: #150%)$,
      $lr(\{, size: #150%) #r.join(", ") lr(\}, size: #150%)$
    ) angle.r)$}

#let db_counter = counter("dbc")
#db_counter.update(0)

#let dbcc = context {
  $sdb_(#db_counter.display())$
}

#let dbc = context {
  db_counter.step()
  $dbcc$
}

As a first example, let's say our objects are elements in $ZZ$, and our query matches $o$ where $o > 5 and o < 20$. Our aggregating function $a(O, S) = <<sum_(o in O) o, S >> $. So to start, our initial database state will be $
    dbcc = dbstate(<< q, a , {}, {}, {} >> ; " ").
$

+ To start, let's insert an object that doesn't match $q$ and see what happens: $
    dbc &= insert(sdb_(0), 3) \
        &= dbstate(
          << q, a , {}, {}, {} >>;
          New(<< 3, 1>>)).
$ #fox[Show how...] So our object is inserted into the refcount set, even though no aggregator is actually matching on it.

+ If we then insert something that _does_ match a handler, say $dbc &= insert(sdb_(1), 7)$, then $
  dbcc = dbstate(
    dirty(new: #true, << q, a , {}, {New(7)}, {} >>);
    << 3, 1 >>, New(<< 7, 1 >>)
  ).
$

+ On the next tick, we process all dirty handlers, and so $
  dbc = dbstate(
    << q, a , {}, {7}, {New(7)} >> ;
    << 3, 1 >>, << 7, New(2) >>
  ).
$ Note that the aggregator in $dbcc$ is _not_ marked dirty, because 7 was already in its input set. If we then add another number, say 10, then $
  dbc = dbstate(
    << q, a , {}, {7, New(10)}, {Removed, New(17)} >> ;
    << 3, 1 >>, << 7, New(1) >>, New(<< 17, 1 >>)
  ).
$

== Example: A Clock Program
#db_counter.update(0)

#let T(n) = {$mono(t#n)$}

Let $q$ match symbols like `t1`, `t2`, ..., and let $
  a(<<I, S>>) = mono("inc")(mono("max")(I))
$, where `max` finds the largest `tn`, and `inc` takes in `t(n)` and produces `t(n + 1)`. #fox[I am being _mega_ loose with the notation, here...] Let $dbcc$ be the empty database: $
  dbcc = dbstate(" " ; " ").
$

Then, let

$ dbc &= insert_(a)(sdb_0, << q, a, {}, {#T(1)}) \
      &= dbstate(
  dirty(<< q, a, {}, {#T(1)}, {#T(1)} >>);
  << #T(1), 1 >>
). $

Note that `t1` shows up in the input set, because we union the initial output set into $cal(O)$ on @ins-world-o, and $q$ will match `t1` on @ins-world-i. Next, calling tick will yield $
  dbcc' &= dbstate(
    <<q, a, {}, {#T(1)}, {New(#T(2))} ;
    << #T(1), 1 >>, New(<< #T(2), 1 >>)
  )
$ after @tick-swap, and $
  dbc &= dbstate(
    <<q, a, {}, {Removed, New(#T(2))}, {#T(2)} ;
    Removed, << #T(2), 1 >>
  )
$ after @tick-return. Each tick, the cycle will repeat, with $a$ always deriving the next `t`. With this handler in place, any number of other handlers can depend on the output---that is, have their queries match objects like `t(n)`, and they will be updated every tick with the new time value. In this way, a single handler can drive any number of other handlers that need to run every tick.

== Example: Non-Aggregating Handlers
#db_counter.update(0)

For many cases in FoxTalk, our handler will (morally) be a function $h: object -> Objects$, that is, we think of our operations on an individual object rather than on the entire set. If we were to just use a simple for loop and call $h$ on each object in our input set and union the outputs, this would be semantically correct, but in a real system would impose a lot of unnecessary work.

In this example, we can define a higher-order aggregator function that selectively calls its inner function $h$ based on the state of individual input objects. This will allow us to efficiently write operations on _system-level resources_, such as Vulkan instances or cameras.

#let nonAgg = "nonAgg"
#show "nonAgg": r => $upright(sans(#r))$

#pseudocode-list(booktabs: true, label: <nonagg>, title: [$nonAgg: (object -> Objects) -> Objects times S -> Objects times S$])[
  - _Note that $S : { <<object, Objects >>}$ _
  + *function* $nonAgg(h)$:
    + *return function* $(I, S)$:
        + *let* $"Ins" = { o | o in I and exists.not <<o', p>> in S. o = o'}$
        + *let* $"Rem" = { <<o, O>> | <<o, O>> in S and o in.not I }$
        + *let* $S' = S without "Rem"$
        - \
        + *for each* $o in "Ins"$:
          + $S' := S' union {<<o, h(o)>>}$
        - \
        + *let* $O' = union.big_(<< o, p >> in S') p $
      + *return* $<< O', S'>>$
]

Let's work through an example with for the non-aggregating handler. Say we start with the empty database with one handler, $dbcc = << { H }, {} >>$, and say that $H = << q, nonAgg(h), {}, {}, {}>>$. Let's also say that $q$ matches $o_1, o_2,$ and $o_3$. Let's also define
$ h(o) = cases( {o_2, o_3} &"if" o = o_1,
                {o_5}      &"if" o = o_2,
                nothing    &"otherwise") $


=== Adding an object

So if we call: $insert(dbcc, o_1)$, then:


+ $R$ is empty, so we take the *else* case on line 6 of #insert. We only have one handler in $A$, which is $H$. By definition, the predicate on line 8 holds, so we change the state of the database to be: $
dbc = dbstate(dirty(new: #true, << q, nonAgg(h), {}, {New(o_1)}, {}>>) ; New(<< o_1, 1 >>) ) $

+ Next, we call tick, which will find our #dirty([H]), and call its handler function with the parameters $({o_1}, {})$. The handler function is the anonymous function defined in nonAgg starting on line 2, and we can take a look at what it does:

  + Ins is every $o$ in the input set where $o$ does not appear in the left-hand side of any tuple in $S$. Since $S$ is empty on this first step, $"Ins" = {o_1}$.

  + Rem looks for any pair in $S$ where the left-hand side of the pair _does not_ appear in $I$, but again $S$ is empty, and so $"Rem" = {}$.

  + Our new sideband state, $S' = {} without {} = {}$.

  + Next, we loop through each new input object $o$ in Ins. We call $h$ on this object, and expect a set of objects back. We store each individual output together with its triggering input in $S'$, so that by line 9, $S' = {<<o_1, o_2>>, <<o_1, o_3>>}$. (We defined $h(o_1)$ above.)

  + Finally, we compute the new output set of this handler, which is the union of all of the right-hand sides in $S'$, in this case the set ${ o_2, o_3 }$.

  + This function finally returns, and we're back on line 6 of tick.

+ We now have $O'$ and $S'$ as above, and we compute new insert sets and removal sets based on the previous output set of this handler (lines 7 and 8). In this case, $"Ins" = O'$ and $"Rem" = {}$.

+ On line 9, We unmark $H$ as dirty, and store its new sideband and output sets. At this point, say that $
    S   &= {New(<<o_1, o_2>>), New(<<o_1, o_3>>)}, \
    dbc &= dbstate(<< q, nonAgg(h), S,  {o_1}, {New(o_2), New(o_3)} ; << o_1, 1 >>)
$

+ For each inserted object, we call insert: $
    dbc &= insert(sdb_2, o_2) \
        &= dbstate(dirty(new: #true, << q, nonAgg(h), S, {o_1, New(o_2)}, {o_2, o_3}) ; << o_1, 1 >>, New(<<o_2, 1>>))
$

+ On the next tick, we go through the same process again. Inside of nonAgg, $"Ins" = { o_2 }$ and $"Rem" = {}$. Our new $S = {<<o_1, o_2>>, <<o_1, o_3>>, New(<<o_2, o_5>>)}$, and $O = { o_2, o_3, New(o_5) }$. Our current DB state after this tick is: $
    dbc &= dbstate(<< q, nonAgg(h), S, {o_1, o_2}, {o_2, o_3, New(o_5)} ; << o_1, 1 >>, <<o_2, 1>>, New(<<o_5, 1 >>))
$

=== Removing an object
Next, let's say we want to remove $o_1$, so we

+ Call $dbc = remove(sdb_4, o_1)$. Now we follow the trail from removal: $ dbcc = dbstate(dirty(<<q, nonAgg(h), S, New({o_2}), {o_2, o_3, o_5} >>) ; Removed, << o_2, 1 >>, << o_5, 1 >>) $ _Note that the $Removed$ is just to draw attention to what was removed_

+ Calling $dbc = tick(sdb_6)$ will run the inner nonAgg handler, and this time:

  + $"Ins" = {}$, and $"Rem" = {<< o_1, o_2 >>, << o_1, o_3 >>}$, since $o_1$ is no longer present in $I$.

  + $S' = {<<o_2, o_5 >>}$, since $o_2$ and $o_3$ are in Rem, and since $O'$ is computed from $S'$, $O' = {o_5}$.

+ Back in tick, we calculate that $"Rem" = {o_2, o_3, o_5} without {o_5} = { o_2, o_3 }$. We update $dbcc'$ so that $ dbcc' = dbstate(
  << q, nonAgg(h), New({<<o_2, o_5>>}), {o_2}, New({o_5})>> ;
  << o_2, 1 >>, << o_5, 1>>
). $

+ But now, on lines 13--14 in tick, we call remove on all of the objects that are no longer being asserted by our handler. So now we have a call to $remove(dbcc, o_2)$, so that $ dbc = dbstate(
  dirty(<< q, nonAgg(h), {<<o_2, o_5>>}, New({}), {o_5}>>) ;
  Removed, << o_5, 1>>
). $

+ Going through the last tick would be similar to the last, and we would end up with our final state of:$ dbc= dbstate(
  << q, nonAgg(h), New({}), {}, New({})>> ;
  Removed
). $

Crucially, by allowing opaque sideband information, we can emulate the original case of two separate handler cases, while keeping the actual Reactor functions small and focused.

= Implementation<implementation>

#figure(
  scope: "parent",
  caption: "Some Reactor Implementations Snippets",
  placement: top
)[
```rust
pub trait GeneratesHandler where Self: Eq + Hash + Sized {
    fn mk_handler(&self) -> Option<Box<dyn Handler<Self>>> { None }
}

pub trait Handler<O>
{
    fn query(&mut self, o: &O) -> bool;
    fn handle(&mut self, input: &HashSet<O>) -> HashSet<O>;

    fn free_o(&mut self, _o: &O) -> () {}
}

pub struct ReactorHandler<O: Eq + Hash>
{
    qa: Box<dyn Handler<O>>,
    pub(super) I: HashSet<O>,
    pub(super) O: HashSet<O>,
    pub(super) dirty: bool,
}


pub struct Reactor<O: Eq + Hash + GeneratesHandler + Clone + Debug> {
    pub handlers: HashMap<ReactorHandlerId, ReactorHandler<O>>,
    pub ref_counts: HashMap<O, u64>,
    generated_handlers: HashMap<O, ReactorHandlerId>,
    current_handler_id: u64
}
```
]<reactor-struct>

#let hid = {$mono("ReactorHandlerId")$}

The Reactor proper is written in Rust#footnote[Which has caused Fox no end of brow-furrowed consternation.], and there are several implementation details to note. The main Reactor struct (called `Reactor<>` in @reactor-struct) has $R$, written `ref_counts`. However, $H$ is written not as a set of handlers, but instead as a map from $hid |-> a$, and has a second map $O |-> hid$. This is so we can support the _handler handler_.

== The Handler Handler
It has always been the intention that handlers are, themselves, described in the database. Ideally, we would have an actual handler, which would look for triples like $angle.l$_some path_, is a, handler$angle.r$, and then load those `.so` files and add them to the set. However, Rust hase _some opinions_ about recursively-referential structs, and allowing the handlers to directly mutate the Reactor that holds them seems like a bad idea anyways. So, while we logically have a handler that does this, in the implementation we just rolled this into the Reactor itself: since the reactor requires that `O` be a `GeneratesHandler`, on each insert it calls `mk_handler` and if that returns a `Some(h)`, it calls $insert_(a)(mono("h"))$ on itself with the returned handler.

== Memory Management

Also of note, the `Handler` implementation has a bonus function called `free_o`, which the Reactor implementation calls whenever calling remove would result in the ref-count reaching zero. After it has removed the object from all of the other handlers' input sets, it calls `free_o` on the object, allowing (one of)#footnote[There's a strange thing that might happen where handlers $h_1$ and $h_2$ both generate $o$. In practice, if there was a resource that needed to be freed, it means that both of these would have somehow acquired and asserted an identical resource handle, pointer, etc. In this case, my claim is that it _cannot_ matter which of those does the clean-up, since they should both know what it is they're putting in the database. One could certainly write pathological handlers, but that seems like a concern for future-Fox.] the handlers that generated it to clean it up. The intended use here is for resources we get from external systems, like objects from Vulkan or OpenCV.

= An Alternative Query Algorithm

#figure(
  // scope: "parent",
  caption: "Incremental Query Tree",
  placement: top
  )[
    #grid(
      columns: (1fr, 1fr),
      gutter: 20pt,
      align: (col, row) => { if(calc.rem(col, 2) == 0) { right } else { left } },
      [$Q_(1) = << star, "is a", "husky" >>$], [$Q_(2) = << "lexi", "is a", "husky" >>$],
      [$Q_(3) = << star, "is a" >>$], [$Q_(4) = << star, "is a", ... >>$]
    )
    #v(12pt)
    #diagram(
      spacing: (18mm, 10mm),
      node-stroke: luma(80%),
      node-corner-radius: 5pt,

      node((0.5,0), [`root`], name: <root>),

      node((0.0, 0.5), [$$], name: <l1>),
      node((0.0, 1.5), [$<< {H_(3)}, {H_(4)} >>$], name: <l2>),
      node((0.0, 2.5), [$<< {H_(1)}, {} >>$], name: <l3>),

      node((1.0, 0.5), [$$], name: <r1>),
      node((1.0, 1.5), [$$], name: <r2>),
      node((1.0, 2.5), [$<< {H_2}, {} >>$], name: <r3>),

      edge(<root>, <r1>, [`lexi`], label-side: left),
      edge(<r1>, <r2>, [`is a`], label-side: left),
      edge(<r2>, <r3>, [`husky`], label-side: left),

      edge(<root>, <l1>, [$star$]),
      edge(<l1>, <l2>, [`is a`]),
      edge(<l2>, <l3>, [`husky`]),
    )
  ]<fig-query-tree>

What if we could incrementally construct a full query-graph, such that following each tuple-noun from left-to-right would end at a handler that cared about them? Then the query parser could still do the final bindings, but updating the input sets would be a tree traversal, rather than an $O(n)$ operation on handlers.

First, let the objects in the database be tuples of symbols $s in S$ of any length, written $<< s_1, s_2, ... s_n >>$ for some $n in NN$. There is also an equivalence relation on the elements of $S$, written $s = s$. Next, let _query tuples_ be tuples constructed from a new set of objects, which is just $Q = S union { star, ... }$. The equivalence relation on $Q$ is the same as on $S$, except that $star$ is equivalent to any $s in S$. The ellipsis matches any number of symbols (including 0) at and beyond that point in the tuple.

For example: $
  << "lexi", "is a", "husky" >> &matches << "lexi", star, "husky" >> \
  << "lexi", "is a", "husky" >> &matches << "lexi", star, star >> \
  << "lexi", "is a", "husky" >> &matches << "lexi", ... >> \
  << "lexi", "is a", "husky" >> &matches << "lexi", star, "husky", ... >> \
  << "lexi", "is a", "husky" >> &cancel(matches) << star, "is a", "labrador" >>. \
  $

#let indexNode = {$sans("index_node")$}
#let handlers = {$sans("handlers")$}
#let prefixHandlers = {$sans("prefix_handlers")$}
#let ns = {$serif("ns")$}

So the idea shown in @fig-query-tree is this: each node is a pair of $<< handlers, prefixHandlers >>$, where #handlers is a set of handlers, and #prefixHandlers is also a set of handlers, but represents the queries that match on tuples of any length (longer than whatever their other specifiers are).

When adding a handler $H_n$, it presents a query tuple $Q_n$, which is used to construct a prefix-tree: the tuple is traversed over each $s_i$ (in order), and at each node, a new edge matching $s_i$ inserted into the tree. If there are no more items left in the tuple, then the handler (that registered the query) is attached to that node in a list of #handlers. If the last symbol in the tuple is $...$, it is instead added to that node's #prefixHandlers list.

Then, querying is a tree-traversal, with at most two branches per node: one for the literal symbol, and a second if that node has a $star$ edge.



#pseudocode-list(booktabs: true, title: [query])[
  + *function* $q([], indexNode) =$ $indexNode"."handlers$
  + *function* $q([n, ns...], indexNode)$ =
    + *let* a = *if* $n in indexNode$ *then*
      - $q(ns, indexNode"."n)$
    - *else* $nothing$.

    + *let* b = *if* $star in indexNode$ *then*
      - $q(ns, indexNode.$star$)$
    - *else* $nothing$.

    + *return* $a union b union indexNode"."prefixHandlers$
]

= Questions

- How do we respond to events from the OS?
- How do we do negative matches? That is, I want to be able to express "`/dev/video0` is a camera $and not$(`/dev/video0` is calibrated.)" (That is, that second triple does not yet exist in the database.)

#bibliography("./bib.yml")