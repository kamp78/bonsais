# Bonsais

This library provides experimental trie implementations using Bonsai structures, described in the following articles:

* Darragh, Cleary and Witten, "Bonsai: A Compact Representation of Trees", SPE, 1993.
* Poyias and Raman, "Improved Practical Compact Dynamic Tries", SPIRE, 2015.

The former and latter are implemented by the Bonsai and BonsaiPlus classes, respectively.
BonsaiPlus provides a very simple m-Bonsai (recursive) implementation without a second hash table to store displacement values.

The library consults [mBonsai](https://github.com/Poyias/mBonsai) and uses [sdsl-lite](https://github.com/simongog/sdsl-lite).

## Performance test

### Setting

The experiments were carried out on Mac OS X 10.12 over Quad-core Intel Core i7 4.0 GHz, with 16 GB RAM (L2 cache: 256 KB; L3 cache: 8 MB).
The codes were compiled using Apple LLVM version 8 (clang-8) with optimization -O3.
The runtimes were measured using __std::chrono::duration_cast__.
To measure the required memory sizes, the __/usr/bin/time__ command was used.
The tries were constructed from all page titles from English Wikipedia of February 2015 (# of nodes: 110962030, # of keys: 11519354, raw size: 227.2 MiB).

### Required memory size on BonsaiPlus parameters 

To explore BonsaiPlus parameters that construct compact trie structures, the required memory sizes were tested for combinations of the load factor and displacement entry size.

![Result](img/fig1.pdf)

### Comparison between Bonsai and BonsaiPlus

Both of Bonsai and BonsaiPlus were implemented in 0.8 and 0.9 load factors.
Bonsai applied 5 bits to the parameter of the maximum number of collisions.
BonsaiPlus applied 6 and 8 bits to the parameter of each first displacement entry in 0.8 and 0.9 load factors, respectively.

The following table lists the timings needed in inserting keys (__Insert__), searching the keys (__Search__), and the maximum resident set size occupied by the insertion process  (__RSS__).
The order of keys was random.

| Data Structure   | RSS (MiB) | Insert (ns / key) | Search (ns / key) |
|------------------|----------:|------------------:|------------------:|
| Bonsai (0.8)     |     266.7 |              2.79 |              4.08 |
| Bonsai (0.9)     |     237.3 |              3.29 |              8.38 |
| BonsaiPlus (0.8) |     256.8 |              1.99 |              2.05 |
| BonsaiPlus (0.9) |     255.4 |              2.06 |              2.12 |