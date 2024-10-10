# ylva.c

<p align="center" width="100%">
    <img width="33%" src="ylva.jpg">
</p>

A simple chess engine in C for educational purposes.

This is a rewrite of an older version that was in C++. A few features are missing and some stuff needs to be fixed as shown in Todos. These will be worked on subject to time constraints.

## Supported UCI commands
- 'go depth n'
- 'position'

## Other commands
- 'perft n' will count nodes to depth n. Nodes per second has gone up to `~20 million nps` from `~2 million nps`. 

## Todos

- [ ] Fix move generation bug
- [ ] Complete writing move ordering, killer move heuristic and PSQT evaluation
- [ ] Add more UCI commands

## Acknowledgements

- Richard Allbert's, [Vice series](https://bit.ly/3XpdiKU) on his Bluefever Software channel
- Marcel Vanthoor's [Rustic chess engine](https://rustic-chess.org)
- [Chess Programming Wiki](https://www.chessprogramming.org)
- Sebastian Lague's [video](https://www.youtube.com/watch?v=U4ogK0MIzqk), which was also the original inspiration for Ylva.
- Oliver Brausch's [Olithink](https://github.com/olithink), specifically his method for peeking std input to check for "stop" command interrupting engine search
- [Analog Hors'](https://analog-hors.github.io/site/magic-bitboards/) explanation of magic bitboards
- [Tord Romstard's](https://www.chessprogramming.org/Looking_for_Magics) source code for finding magic numbers

## License
MIT
