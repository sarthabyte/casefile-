# CASEFILE

A terminal-based C murder-mystery game.

## Build and run on Linux / Linux Mint

Open a terminal in this project directory and run:

```bash
make
./casefile
```

Or build and launch in one command:

```bash
make run
```

The Makefile uses GCC with C99 and warning flags. Install the compiler tools on Linux Mint with:

```bash
sudo apt update
sudo apt install build-essential
```

## Makefile commands

| Command | Purpose |
|---|---|
| `make` | Compile the game into `casefile` |
| `make run` | Compile if needed, then launch the game |
| `make test` | Build and run the save/load tests |
| `make test-build` | Build all three separate test executables |
| `make clean` | Remove generated executables and temporary test files |

**Save-file protection:** `make clean` deliberately does not delete `savegame.dat`. The screen-clearing helper only clears terminal output; it does not change the game state or save file.

## Project layout

- `src/` - C source files
- `include/` - Header files
- `data/` - Suspect, evidence, location and dialogue data
- `tests/` - Separate test programs

Run the game from this project directory because the program loads data using relative paths such as `data/suspects.txt`.

## Dialogue data note

The supplied `suspects.txt` refers to `data/dialogue/vikram.txt`, `arjun.txt`, `meena.txt`, and `ravi.txt`. Those four dialogue files were not among the uploaded files, so the folder contains a README placeholder. The game compiles without them, but suspect questioning will report that no dialogue statements were loaded until the original dialogue files are added.

`make test-build` compiles the three uploaded test programs. `make test` runs the save/load test suite (`test_files`). The suspect/dialogue test (`test_person1`) needs the missing dialogue data files to pass; `test_courtroom` is an interactive test executable.

## Six-room investigation and locker puzzle

A new game generates a randomized six-digit locker combination. One digit is hidden in each of Rooms 1-6, alongside the six case-relevant evidence clues. The player can investigate up to six distinct rooms. The concealed locker is behind the painting in the Manager's Office (Room 6). Digits must be read in ascending room-number order. The generated puzzle and discovery progress are stored in the version 3 save format. Older version 2 saves are not compatible with this updated build.
