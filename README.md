# fishNET

A back-propagation neural network simulator, written in C as an undergraduate
thesis at the University of Sydney in 1988.

**Thesis:** *Brain Simulation: Computation in Back-Propagation Neural Networks*  
**Author:** Scott MacGibbon  
**Degree:** Bachelor of Engineering, University of Sydney

The disks lived in the back of the bound thesis until 2026
<img width="768" height="1024" alt="FB0F6BC4-7A4F-4D9E-BA57-54F5610DCDF6_1_105_c" src="https://github.com/user-attachments/assets/c240a231-8075-44ba-b7c6-fd0bbab69998" />

<img width="2048" height="1536" alt="8B2F6FAE-C879-41D4-A9C0-4DDF9DF5817A_1_102_o" src="https://github.com/user-attachments/assets/96a5d4de-e49d-46a5-babe-5244f36d536b" />





## Contents

- `a-working-antique/` — Modernised ANSI C version; this is the one to build and run
- `the-museum--from-original-disks/` — Original K&R C source as recovered from the floppy disks, kept for reference

## Building

```sh
cd a-working-antique
make
```

Requires a standard C compiler and `libm`. Tested with Apple clang on macOS.
The binary is produced at `a-working-antique/fishNET`. Run it from that directory —
the help file `help.hlp` must be present in the working directory.

## Running

fishNET is driven entirely from the command line. The typical workflow is:

**1. Train a network from a config file and save it:**

```sh
./fishNET -cdata/letters/letters.cfg -1 -s
```

`-c[file]` loads a configuration file (the filename follows the flag with no space,
e.g. `-cdata/letters/letters.cfg`). `-1` selects EACH mode (update weights after
every training case). `-s` saves the trained network when training completes.

Training prints an error count each sweep and stops when errors reach zero.
The network is saved to the path specified in the config file.

**2. Run a saved network on new data:**

```sh
./fishNET -nnets/letters.net -e
```

`-n[file]` loads a pre-trained network. `-e` runs it over the execute input
specified in the network file and writes results to the execute output file.

**3. Continue training a saved network:**

```sh
./fishNET -nnets/letters.net -t -s
```

`-t` resumes training from where it left off, using the sweep count stored in
the network file.

## Parameters

| Flag | Meaning |
|------|---------|
| `-c[file]` | Load configuration file (no space between flag and filename) |
| `-n[file]` | Load pre-trained network file |
| `-e` | Execute: run loaded network and write output (requires `-n`) |
| `-t` | Train: continue training loaded network (requires `-n`) |
| `-1` | EACH mode: apply weight updates after every training case |
| `-a` | ALL mode: accumulate gradients over all cases then update (default) |
| `-s` | Save network to file after training completes |
| `-d` | Don't save network |
| `-x` | Save learning statistics only (sweep count, alpha, epsilon) |
| `-v` | Verbose: print data and layer outputs at each step |
| `-q` | Quiet: suppress all output except errors |
| `-?`, `-h` | Display help |

## Letter recognition demo

The `a-working-antique/data/letters/` directory contains a ready-to-run example:
five 13×15 pixel bitmaps of the letters A, V, C, E and T, with one-hot expected
outputs. The network architecture is 195 inputs → 40 hidden → 5 outputs.

```sh
cd a-working-antique
./fishNET -cdata/letters/letters.cfg -1 -s
cat data/letters/result.dat
```

Each `[start]` block in the result file shows the five output activations for one
input letter. The highest value identifies which letter the network recognised.
