# efm

*this project is currently being developed*

a file manager that is lightweight and reliable. Aimed to be simple, yet functional.
Written in C and only uses standard Linux libraries and packages.

## System

Technically every Linux machine should be able to run efm. Still, it is not tested
other than my own Manjaro environment. 

## Installation

```sh
git clone https://github.com/ekinakkaya/efm.git
cd efm
make
sudo make install clean
```

## Usage

```sh
efm
```

## Key Bindings



### TODO
```
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

- select multiple files

- search

- dynamic array management

- print file sizes

- copy and paste files
  - https://stackoverflow.com/questions/43265332/copying-files-in-c-program

- cut and paste files

- delete files

- tar support
```
