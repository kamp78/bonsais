# Bonsais

This library provides an experimental trie implementation using Bonsai structures, described in the following articles:

* Darragh, Cleary and Witten, "Bonsai: A Compact Representation of Trees", SPE, 1993.
* Poyias and Raman, "Improved Practical Compact Dynamic Tries", SPIRE, 2015.

The former and latter are implemented by the __BonsaiDCW__ and __BonsaiPR__ classes, respectively.
BonsaiPR provides a very simple m-Bonsai (recursive) implementation without the second Bonsai hash table to store displacement values.

I consulted the [mBonsai](https://github.com/Poyias/mBonsai) implementation.

## Performance test

### Setting

The experiments were carried out on Quad-core Intel Core i7 @4.0 GHz, with 16 GiB of RAM (L2 cache: 256 KiB; L3 cache: 8 MiB), running Mac OS X 10.12.
The codes were compiled using Apple LLVM version 8 (clang-8) with optimization -O3.
The runtimes were measured using __std::chrono::duration_cast__.
To measure the required memory sizes, the __/usr/bin/time__ command was used.
The tries were constructed from sampled geographic names on the _asciiname_ column from the GeoNames dump (# of nodes: 8,575,826, # of keys: 1,000,000, raw size: 14.9 MiB).

### Test for BonsaiPR parameters 

To explore BonsaiPR parameters for constructing compact trie structures, some combinations of two parameters were tested.
One is the load factor of a hash table.
The other is each primary slot size for storing displacement values, which is the third parameter of the constructor, *width_1st*.

![Result](https://github.com/kamp78/bonsais/blob/master/graph.png?raw=true)

From the results, pairs (0.8, 6) and (0.9, 8) are good parameter combinations.

### Comparison between BonsaiDCW and BonsaiPR

Both of BonsaiDCW and BonsaiPR were implemented in 0.8 and 0.9 load factors.
BonsaiDCW applied 5 to the third parameter of the constructor, *colls_bits*.
BonsaiPR applied 6 and 8 to *width_1st* in 0.8 and 0.9 load factors, respectively.

The following table lists the timings needed in inserting keys (__Insert__), searching the keys (__Search__), and the maximum resident set size occupied by the insertion process  (__RSS__).
The order of keys was random.

| Data Structure | RSS (MiB) | Insert (us / key) | Search (us / key) |
|-----------------|----------:|------------------:|------------------:|
| BonsaiDCW (0.8) | 22.4 | 1.78 | 2.67 |
| BonsaiDCW (0.9) | 20.2 | 2.05 | 5.28 |
| BonsaiPR (0.8) | 21.7 | 1.31 | 1.25 |
| BonsaiPR (0.9) | 21.6 | 1.36 | 1.30 |
