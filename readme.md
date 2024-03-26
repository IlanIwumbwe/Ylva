# YLVA

<p align="center" width="100%">
    <img width="33%" src="ylva.jpg">
</p>

Welcome to *Ylva!*

A minimal chess engine in the terminal. Ylva will see improvements over time.

**Not** UCI compliant. 

Run using `run.sh`. This brings up a menu, with options to play with a friend, with the engine, engine v engine, or run Perft.
 
Current move generation speed is `~500k` nodes/second. Proper benchmarking to be done in the future.

### Notable techniques

Ylva uses bitboards as the data structure that stores information about a board state. 

Bitboards are great because when used right, they speed up move generation significantly, because most operations required during move generation are reduced to bitwise operations. 12 bitboards are used for each of the 6 chess pieces, for each colour. 

Slider piece move generation is really costly, and various techniques such as [magic bitboards](https://www.chessprogramming.org/Magic_Bitboards) exist to speed up slider piece move generation. This implementation uses the [classical approach](https://www.chessprogramming.org/Classical_Approach) as it was the easiest to implement as a starting point.

The engine is uses a very simple material based evaluation and minimax search to decide which moves are best.

### Todos

[Future plans for Ylva](https://github.com/IlanIwumbwe/Ylva/issues/2)
