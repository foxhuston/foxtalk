Let's say that triples aren't noun -> pred -> noun. Let's say entire triples are a node with subj, pred, and obj as union types of string and int-- and also a hash int id.

Handlers are a special kind of node that just have the function pointers as props that we care about.

When handlers register a query with FoxTalk,

Nodes:

- Claim

  - subject UNION (symbol STRING, cptr INT64, i64 INT64, u64 INT64)
  - predicate UNION (symbol STRING, cptr INT64, i64 INT64, u64 INT64)
  - object UNION (symbol STRING, cptr INT64, i64 INT64, u64 INT64)

- Handler
  - name string
  - initfn int64
  - ... (other fns)
- Query
  - subject UNION (symbol STRING, cptr INT64, i64 INT64, u64 INT64, query BOOLEAN)
  - predicate UNION (symbol STRING, cptr INT64, i64 INT64, u64 INT64, query BOOLEAN)
  - object UNION (symbol STRING, cptr INT64, i64 INT64, u64 INT64, query BOOLEAN)

Relationships:

- (Claim)-[was created by{at_tick_number INT64}]->(Handler) (many to one)
- (Claim)-[was created by{at_tick_number INT64}]->(Claim) (one to one) // Only for non-aggregating handlers
- (Claim)-[is part of a result of query{at_tick_number INT64}]->(Query) (many to many)
- (Handler)-[cares about]->(Query) (one to many)

Operations:

- Every tick:

  1. Get every [cares about] rel
  2. Generate the cypher to create [is part of a result of query{at_tick_number INT64}] for every query, for every claim each of the ->(q: query) matches
  3. Run cypher to create all of the [is part of a result of query{at_tick_number INT64}] relationships
  4. For every handler, create a query that returns a result set to run ONLY IF the `(returnedClaim: Claim)-[is part of a result of query{at_tick_number INT64}]->(myHandlersQuery: Query)` set of `returnedClaim` is different from the set at the previous tick number.
     a. For aggregating handlers, if ANY of the `returnedClaim`s are different, then return ALL `returnedClaim`
     b. For non-aggregating handlers, ONLY return the `returnedClaim`s that are different
     c. **This means that every tick, we will only run handlers if the result set of claims they care about has changed since the last tick**.

- In the case of a handler needing to be run:

  1. For each `returnedClaim`
     a. Run `handle`, which might run `reactor::claim`
  2. For non-aggregating handlers:
     a. Delete all `(creatingClaim: Claim)<-[was created by]-(deleteMe: Claim)-[was created by{at_tick_number INT64}]->(Handler)` where the `creatingClaim` was NOT in the `returnedClaim` set
  3. For aggregating handlers:
     a. Delete all `(deleteMe: Claim)-[was created by{at_tick_number INT64}]->(Handler)` where `at_tick_number` is < current tick

- On `reactor::claim`:

  1. Merge `(Claim)-[was created by{at_tick_number INT64}]->(Handler)` with the current tick number and the current handler
  2. For non-aggregating handlers:
     a. Merge `(Claim)-[was created by{at_tick_number INT64}]->(Claim)` with the current tick number and the current result being read

- On handler deletion:
  1. For the match `(creatingClaim: Claim)<-[was created by]-(handlerClaim: Claim)-[was created by]->(Handler)` and all
  <!-- 
  handler A
  handler B
  handler C

1. system -> create claim o_1
2. o_1 -> handler A -> o_2, system removes o_1
3. o_2 -> handler B -> o_3, (system removes o_2) <--- HOW
4. o_3 -> handler C -> o_4, free_A(o_2), (system removes o_3) <--- HOW
5. free_C(o_3), system removes o_4
6. free_C(o_4) -->
