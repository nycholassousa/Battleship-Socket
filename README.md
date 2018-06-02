# Battleship
This project is the implementation in C of the battleship game.

## How to play

- Build with the command `gcc -o battleship battleship.c`
- Launch two instances of the program with `./battleship`
- Use the IP address `127.0.0.1` if two instances of the program run on a single computer
- To place ships, type the direction, the column and the line in three letters:
  - VE5: place ship vertically, starting from E5
  - ha0: place ship horizontally, starting from A10 (0 designates the 10th row)
- To fire, use two letters (e.g. "D7")

## TODO

- Change TCP to UDP
- Add more visual information
- Disconnect timer
- Finish/Close the game when any player leaves

