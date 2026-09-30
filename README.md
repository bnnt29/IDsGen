# ID Generator Tool

This repository contains two C-based PDF generators built around `libharu`:

- `Kinderstadt Ausweis Befüller`: generates participant ID cards from CSV data
- `Money generator`: generates printable play money sheets

## Repository Structure

- `Kinderstadt Ausweis Befüller/`
- `Money generator/`

## Kinderstadt ID Card Filler

This project generates PDF ID cards for children's activities based on CSV files.

### Purpose

The tool automates the creation of participant ID cards.
Data (name, role, etc.) is read from CSV files and merged into a PDF.

### Components

- `programm.c`: PDF rendering (layout, fields, page structure)
- `pipe_inputer.c`: CSV input for a simple single-group scenario
- `pipe_inputer_dualkey.c`: CSV input with role mapping from a separate file
- `quickstart.sh`: predefined quick start with example files
- `gen_id.sh`: interactive start for the simple scenario
- `gen_id_dualkey.sh`: interactive start for role mapping
- `roles.txt`: roles and RGB color values
- `Teilnehmer2024.csv`: example input file

### Requirements

- Linux
- `gcc`
- `make`
- `libharu` (`-lhpdf`) including headers/library in the compiler/linker path

Note: The repository includes sources for `libharu` and `libpng`, but the default build uses the local compiler/linker (`gcc ... -lhpdf`). If `libhpdf` cannot be found, install the library first or configure linker/include paths accordingly.

### Quick Start

Run from the repository root:

```bash
cd "Kinderstadt Ausweis Befüller"
chmod +x quickstart.sh
./quickstart.sh
```

Internally, the quick start does:

1. `make`
2. Reads `roles.txt` and `Teilnehmer2024.csv`
3. Pipe call:
   `./pipe_inputer_dualkey.run roles.txt Teilnehmer2024.csv Rolle Vorname Name Extra | ./programm.run out 1`
4. Generates `out.pdf`
5. `make clean`

### Input Data

Participant CSV example (`Teilnehmer2024.csv`):

```csv
Vorname;Name;Rolle;Extra
Bennet;Richter;Team;1:3
```

The quick start expects these column names:

- `Rolle`
- `Vorname`
- `Name`
- `Extra`

`roles.txt` maps roles to colors in blocks:

```txt
#Team
235
0.0
0.0
#Buerger
87
153
36
```

Per-role schema:

1. Role name prefixed with `#` (for example `#Team`)
2. Red value
3. Green value
4. Blue value

### Alternative Starts

```bash
cd "Kinderstadt Ausweis Befüller"
chmod +x gen_id.sh gen_id_dualkey.sh
./gen_id.sh
# or
./gen_id_dualkey.sh
```

### Important Current Layout Note

Layout changes (positions, fields, sizes, visual structure of the ID cards) are currently **not** configurable via config files.

If you want to change the card layout, you currently need to modify and recompile the C code, mainly in:

- `Kinderstadt Ausweis Befüller/programm.c`

### Manual Build

```bash
cd "Kinderstadt Ausweis Befüller"
make
```

Build output:

- `programm.run`
- `pipe_inputer.run`
- `pipe_inputer_dualkey.run`

Clean up:

```bash
make clean
```

### Known Limitations

- CSV keys (column names) must match the provided parameters exactly.
- The layout is hardcoded and not configurable.
- Error handling and input validation are currently basic.

## Money Generator

The Money Generator creates printable PDF sheets with play money entries.

### Components

- `Money generator/programm.c`: rendering logic and command line parsing
- `Money generator/resources/`: fonts and images used in the PDF design
- `Money generator/makefile`: build target for `programm.run`

### Build

```bash
cd "Money generator"
make
```

This builds:

- `programm.run`

### Usage

Run:

```bash
./programm.run <output> <Name> <Times> <Amount> [<Name> <Times> <Amount> ...]
```

Example:

```bash
./programm.run Geld_Mix Alex 4 1 Sam 2 5 Team 1 10
```

This creates `Geld_Mix.pdf`.

Argument meaning:

- `<output>`: output filename without `.pdf`
- `<Name>`: label shown on the money item
- `<Times>`: how many items of this entry should be generated
- `<Amount>`: value printed on the item

### Notes

- The generator uses hardcoded layout values in `Money generator/programm.c`.
- Visual/layout changes currently require C code changes and recompilation.
