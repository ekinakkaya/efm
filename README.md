# efm

*this project is currently being developed*

a file manager that is lightweight and reliable. Aimed to be simple, yet functional.
Written in C and only uses standard Linux libraries and packages.

## System

Technically every Linux machine should be able to run efm. The ui might be buggy for different terminal applications. It's been tested on my Manjaro environment and Windows Terminal running WSL Ubuntu 20.04. Not tested with ssh.

## Installation

```sh
git clone https://github.com/ekinakkaya/efm.git
cd efm
make && sudo make install clean
```

## Usage

```sh
efm
```

## Key Bindings



### TODO
```
- Remove pp() function

- Use these features
    
    Setup the terminal for the TUI.
    '\e[?1049h': Use alternative screen buffer.
    '\e[?7l':    Disable line wrapping.
    '\e[?25l':   Hide the cursor.
    '\e[2J':     Clear the screen.
    '\e[1;Nr':   Limit scrolling to scrolling area.
                 Also sets cursor to (0,0).
    
    Reset the terminal to a useable state (undo all changes).
    '\e[?7h':   Re-enable line wrapping.
    '\e[?25h':  Unhide the cursor.
    '\e[2J':    Clear the terminal.
    '\e[;r':    Set the scroll region to its default value.
                Also sets cursor to (0,0).
    '\e[?1049l: Restore main screen buffer.

- select multiple files

- search

- dynamic array management

- print file sizes

- copy and paste files
  - https://stackoverflow.com/questions/43265332/copying-files-in-c-program

- cut and paste files

- delete files

- tar support

- [DONE] list directories

- [DONE] handle terminal size

- [DONE] !!! DECIDE WHAT THE UI WILL LOOK LIKE

- [DONE] list files properly

- [DONE] make the directories list an ordered list

- [DONE] file selection in ui

- [DONE] make is_directory function
  - (this is crucial for navigating up and down in directories and
    highlighting directories in ui)

- [DONE] highlight directories

- [DONE] colored filenames

- [DONE] navigate up and down in directories

- [DONE] read and sort the directory ONLY when the directory is changed

```
