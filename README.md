# Bonsais

This library provides an experimental trie implementation using Bonsai structures, described in the following articles:

* Darragh, Cleary and Witten, "Bonsai: A Compact Representation of Trees", SPE, 1993.
* Poyias and Raman, "Improved Practical Compact Dynamic Tries", SPIRE, 2015.

The former and latter are implemented by the __BonsaiDCW__ and __BonsaiPR__ classes, respectively.
BonsaiPR provides a very simple m-Bonsai (recursive) implementation without the secondary Bonsai hash table to store displacement values because it is difficult to estimate the table size.

I consulted the [mBonsai](https://github.com/Poyias/mBonsai) implementation.

## Performance test

### Setting

The experiments were carried out on Intel Xeon E5540 @2.53 GHz, with 32 GiB of RAM (L2 cache: 1 MiB; L3 cache: 8 MiB), running Ubuntu Server 16.04 LTS.
The codes were compiled using g++ (version 5.4.0) with optimization -O9.
The runtimes were measured using __std::chrono::duration_cast__.
To measure the required memory sizes, the __/usr/bin/time__ command was used.
The tries were constructed from all page titles from English Wikipedia of February 2015 (# of nodes: 110,962,030, # of keys: 11,519,354, raw size: 227.2 MiB).

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

| Data Structure | RSS (MiB) | Insert (ns / key) | Search (ns / key) |
|-----------------|----------:|------------------:|------------------:|
| BonsaiDCW (0.8) | 267.6 | 3.68 | 6.24 |
| BonsaiDCW (0.9) | 238.3 | 4.54 | 14.88 |
| BonsaiPR (0.8) | 259.4 | 2.33 | 2.04 |
| BonsaiPR (0.9) | 257.2 | 2.46 | 2.18 |
