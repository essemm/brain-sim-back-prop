# fishNET

Back-propagation neural network simulator, originally written by Scott MacGibbon
as an undergraduate engineering thesis at the University of Sydney, 1988.
Modernised from K&R C to ANSI C; DOS-specific code removed.

## Build

```
make
```

Requires a standard C compiler and `libm`. Run all commands from this directory —
`help.hlp` must be in the current working directory.

## Letter recognition example

### Train and save the network

```
./fishNET -cdata/letters/letters.cfg -1 -s
```

- `-cdata/letters/letters.cfg` — load the configuration file (no space after `-c`)
- `-1` — EACH mode: apply weight updates after every training case
- `-s` — save the trained network when done

Training prints an error count each sweep. It converges when errors reach 0,
typically within a few hundred sweeps. The network is saved to `nets/letters.net`.

The training data is five 13×15 pixel bitmaps of the letters A, V, C, E, T.
The network learns to identify each letter: 195 inputs → 40 hidden → 5 outputs.

### Run the trained network

```
./fishNET -nnets/letters.net -e
```

- `-nnets/letters.net` — load the pre-trained network (no space after `-n`)
- `-e` — execute: run the network over the input data and write results

Results are written to `data/letters/result.dat`. Each `[start]` block is one
input case; the five values are the activations of the five output neurons
(one per letter, in order A V C E T). The highest value identifies the letter.

## Other flags

| Flag | Meaning |
|------|---------|
| `-a` | ALL mode: accumulate gradients over all cases before updating (default) |
| `-1` | EACH mode: update weights after every training case |
| `-d` | don't save the network after training |
| `-s` | save the network after training |
| `-n[file]` | load a pre-trained network file |
| `-t` | retrain a loaded network (combine with `-n`) |
| `-v` | verbose: print data and outputs at each step |
| `-q` | quiet: suppress all output except errors |
| `-?` | show help |

## Data format

Config files, training data, and network files are plain text. See `help.hlp`
for the full format description, or run `./fishNET -?`.
