# Semi-coordinated unique ID generation

## Simplified Problem

Suppose you have a distributed environment in which some unknown
number of processes will spawn and, without coordination between
processes, each process generates some unknown number of entities that
must be assigned globally unique *b*-bit identifiers at the time each
entity is created.

We want a (randomized) algorithm for assigning unique IDs to entities
that minimizes the probability of any collisions between all entities.

Note: there is no known global ordering or prior unique identifiers
for processes, because that would require some kind of coordination.
We can assume coordination and ordering within each process, but each
process does not know how many entities it will generate until after
the last is generated.

## As a "prisoner's" problem
You awake on an alien ship suffering complete amnesia. You are told that
you and other abductees can return to Earth in single-occupancy escape
pods, but only if you are smart enough for each of you to choose unique
escape pods, which are numbered 1 to N. If any of you choose the same pod,
you will all be killed for alien science.

You are being brought in one-by-one to choose an escape pod, without
knowing anything about which have already been chosen, EXCEPT that
some of you will be allowed to receive a written note from the previous
person to choose but some will not. You have not received a note from
the previous person--if there was one. You don't know how
many people there are trying to escape, nor how likely the next person is
to be allowed to see your note.

You have been told how many escape pods there are and you have been
given a fair coin (reset to a pre-determined state). You don't see any
clocks on the wall or anything like that, and because of the amnesia, you
don't know any identifying information about yourself, such as your
birthday or what room you were being held in.

What is the best strategy to maximize chances of escape, and what then do
you write in the note to the next person?

## Intended Application
This is the essence of the problem of generating unique identifiers
for SST files in
[RocksDB](https://github.com/facebook/rocksdb). Because RocksDB is a
general library used on various hardware configurations and
distributed environments, we cannot necessarily rely on **structured**
identifiers for RocksDB instances, via prior coordination such as MAC
address + process ID + wall clock time. (See also [Eric Lippert's GUID
Guide](https://ericlippert.com/2012/04/24/guid-guide-part-one/) and
[MongoDB ObjectID format](https://github.com/mongodb/specifications/blob/master/source/objectid.rst).

Although SST unique IDs are unlikely to all go into a global pool for
look-up, it is common for a machine to host a subset of RocksDB
instances with a shared caching layer, for which we need stable and
unique keys. It is also common for instances to be migrated from one
machine to another somewhat arbitrarily. So although we really want to
minimize the probability of any collisions among unique IDs among some
later-determined subset of processes, minimizing the probabily of any
collisions globally is a simpler and "close enough" optimization goal.

Note that minimizing the expected number of collisions is potentially
a different problem than minimizing the probability of any
collisions. A collision could lead to a data correctness bug in
RocksDB; in such a case, it is arguably better to hit a cluster of
collisions than one isolated collisions because the cluster is more
likely to be detected (e.g. process crash more likely vs. silent
corruption) as requiring mitigation (e.g. promote a new leader).

## Naive Solution

The naive solution, corresponding to [RFC 4122 Version
4](https://en.wikipedia.org/wiki/Universally_unique_identifier#Version_4_(random)),
is to assign each entity an independently random identifier. For *n*
processes each generating on average *m* entities, the probability of
any collisions is approximately `1 - exp(-(n*m)^2 / 2^(b+1))`. For
example, with *n*=100, *m*=100, and *b*=32, we have about a 1% chance
of collision.

## Improved Limited Solution (obvious?)
Suppose we make the assumption that all processes generate exactly *m*
entities. A logical approach is to divide the set of 2^*b* possible
identifiers into buckets of size *m*. Each process chooses a random
bucket and assigns entities bijectively to the identifiers in the
bucket. The probability of collision is `1 - exp(-n^2 * m / 2^(b+1))`,
as it only depends on the number of buckets `2^b / m` and number of
processes *n*.

## Improved General Solutions (apparently new)
Now let us return to the general problem in which the number of
entities generated by each process is unknown until they are all
generated. Let *m* be the average number of entities generated per
process.

### Addition from a random base ID
Each process can choose a random `base_id` and simply assign ID
`base_id + i (mod 2^b)` to each *i*th entity it generates. It is easy
to see that this approach is no better than the limited bucket
solution for the same *m*, but should have probability within some
small constant of that based on the expected number of buckets
intersecting random ranges with average length the same as bucket
size.

### Xor from a random base ID
Each process can choose a random `base_id` and simply assign ID
`base_id ??? i` to each *i*th entity it generates. This is
plausibly better than addition because it tends to pack IDs into a
minimum of power-of-two sized buckets.

## Simulation results

Running the Monte Carlo simulation in monte_carlo/sim.cc, we see
results like this:

```
$ ./sim.out 100 26 30000  # b=20, m=100, n=26
Collision probability random: 0.9593
Collision probability bucket_fixed: 0.0305
Collision probability addition_fixed: 0.0613667
Collision probability xor_fixed: 0.0383333
Collision probability addition_geom: 0.0611333
Collision probability xor_geom: 0.0552333
$ ./sim.out 128 20 30000  # b=20, m=128, n=20
Collision probability random: 0.955633
Collision probability bucket_fixed: 0.0229667
Collision probability addition_fixed: 0.0467667
Collision probability xor_fixed: 0.0235667
Collision probability addition_geom: 0.0464
Collision probability xor_geom: 0.0437
$ ./sim.out 129 20 30000  # b=20, m=129, n=20
Collision probability random: 0.957567
Collision probability bucket_fixed: 0.0231667
Collision probability addition_fixed: 0.0455333
Collision probability xor_fixed: 0.0439
Collision probability addition_geom: 0.0458333
Collision probability xor_geom: 0.0426
```

`_fixed` configurations use constant *m* while `_geom` configurations use
geometric distribution (memoryless) with average *m* entities per process.

As predicted, we can very much improve on the naive random solution. The
unrealistic bucket approach gives a kind of lower bound for the
probability we can hope to attain with the realistic addition and xor
approaches. Although in some limited cases xor can come close to the
bucket approach, both xor and addition are more typically around twice the
collision probability of the bucket approach. Overall, xor does maintain a
small advantage over addition.
