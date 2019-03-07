# Meta tic-tac-toe

Meta tic-tac-toe AI with minimax and MCTS heuristic

An implementation of a meta tic-tac-toe AI that uses a minimax (negamax) algorithm with pure Monte Carlo tree search as heuristic.


## Features

- ASCII visualization of the board
```
[ /---\ | X X . | O O X ]
[ |   | | O . X | O O . ]
[ \---/ | O X . | . . X ]
[ - - - * - - - * - - - ]
[<. X .>| X . . | . O . ]
[<O . X>| X O . | . X . ]
[<O X .>| . X O | . O X ]
[ - - - * - - - * - - - ]
[ \   / | /---\ | . . O ]
[  >-<  | |   | | . X O ]
[ /   \ | \---/ | X . . ]
```
- Works for any board size (up to 15<sup>4</sup>), though graphics aren't designed for N less than 3
  - This limit is due to the `left` field in `Square` and `Meta` structs, but by changing them to `uint16_t` and `uint32_t` respectively, N can go up to 127.
  - However, since the boards are stored on the stack (see design goals), increasing N too far causes segfaults.
- Can tweak AI strength using command-line arguments
- Can run with any combination of player/AI for X/O


## Running

Run `make` to build the standard 3x3x3x3 board. `make 4` and `make 5` build versions for 4x4x4x4 and 5x5x5x5 respectively. For larger versions, just run the command in the Makefile with `-DN=6`, replacing 6 with the value of N you want.

The default configuration runs two equal-strength AIs against each other. To play as the first player (X), use the flag `--p1ai 0` to turn off the AI for player 1.

### Player move

To play as the player, one uses the following commands:
- `p`: Print the board again.
- `z`: Make a move. (`z` is the last letter, and making a move is the last action of your turn.)
  - To refer to a square, first use the X and Y coordinates of the sub-board, then use the X and Y coordinates of the square within the sub-board.
  - The top-left is (X, Y) = (0, 0). Moving right increases X, moving down increases Y.
  - For example, the bottom-left square in the middle-right sub-board is 2102.
  - This method, as opposed to something like a "qweasdzxc" addressing scheme, is to allow for larger boards.
- `m`: Display the coordinates of the sub-boards within the board, and of the squares within a sub-board ("m" comes from "map").
- `h`: Print the help text.

### AI move 

When an AI plays, it will output some information:
- Opinion: A percentage denoting how likely the AI thinks it will win.
  - 200% and -200% denote that it is certain it will win or lose respectively.
  - A number between 100% and -100% denotes, in the best possibility (leaf node) it has looked at, the difference between the wins and the losses; i.e. 100% means all wins, -100% means all losses, 0% means equal number of wins and losses.
  - Note that ties are not counted as wins or losses; for example, a leaf node with 30 wins, 20 losses, and 50 ties will have an opinion of 10%.
- Move: The position on the board the AI played at.
  - The addressing scheme is as described above.
- Evals: The number of playouts the AI ran. 
  - Note that this is not always the same as the `nt` flags, as at every node, the AI allocates an equal number of playouts for each branch.
  - For example, if a particular node has 100 playouts allocated to it, and has 7 branches, each child node would get 14 playouts, for a total of 98.
  - Unless 14 playouts is less than the `minnt` for the AI; then instead it would just run the MCTS heuristic evaluation and return.

### List of commands

```
$ ./mttt --help
Meta tic-tac-toe (N = 3)
usage: ./mttt [--help] [--sizeof] [--seed (none)] [--start 1]
        [--p1ai 1] [--p1nt 100000] [--p1minnt 40]
        [--p2ai 1] [--p2nt 100000] [--p2minnt 40]
  --help        display help and exit
  --sizeof      display size of structs and exit
  --seed        set random seed
  --start       select starting player (1 = X, 2 = O)
  --p(1|2)ai    enable AI for given player (0 = off, 1 = on)
  --p(1|2)nt    maximum number of playouts for a move
  --p(1|2)minnt minimum number of playouts for a leaf node
```


## Design goals

I initially wanted to write this project in Haskell (to treat the board as an algebraic data type), but I soon switched to Python. However, I never finished that implementation, and it would probably have been very slow. Later I decided to write it in C. It turns out this was conducive to some interesting implementation techniques and goals.
- No dynamic memory allocation
- Fast execution speed
- Minimal RAM (actually, writable memory) usage
- Platform-independent
- Bonus: in theory, can run (slowly) on embedded devices with little modification


## Interesting optimizations

- `N` (board size) is defined as a macro to allow for additional compile-time optimizations (such as constant folding), especially when computing addresses.
- The pieces on the board are represented as `int8_t`, which reduces the size of a board struct by a factor of 4 or 8 (depending on if originally 32-bit or 64-bit).
- `PREC` (precision of evaluation results) was originally a multiplicative constant (1000000), but it's faster to use a shift, and convert the result into a readable value only when displaying it.


## Future work

- Make it even faster?
- Add a feature to save/load the state of the game
- Test out the MM MCTS AI against other algorithms, such as pure MCTS or a weighted-sum heuristic
- Test out the MM MCTS AI against itself, with varying parameters


## History

I first heard about MCTS from [this StackOverflow answer about an AI for 2048](https://stackoverflow.com/a/23853848), where I was struck by its simplicity and generality.

However, a pure MCTS strategy may not work well for a 2-player game. For example, consider a move where the opponent has exactly one winning response. Obviously this move is a losing move. However, because the opponent's response is rare in random play, the pure MCTS strategy may choose the branch anyway.

This led me to consider the idea of combining MCTS with minimax, where we can treat the opponent's nodes as the worst case of their branches (instead of just the average case), and our nodes as the best case, at least for the first few levels.


## References

While writing this README, I found that combining minimax and MCTS has in fact been considered before, in this paper:

Baier, Hendrik, and Mark HM Winands. "Monte-Carlo tree search and minimax hybrids." 2013 IEEE Conference on Computational Inteligence in Games (CIG). IEEE, 2013.

However they flip the roles of minimax and Monte-Carlo tree search (using minimax to influence playouts when possible).


## License

Licensed under GPLv3. See the LICENSE file for details.

Copyright (c) Andrew Li 2019
