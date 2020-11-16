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
- [DONE] create directory

- copy and paste files
  - https://stackoverflow.com/questions/43265332/copying-files-in-c-program
  - https://forgetcode.com/c/577-copy-one-file-to-another-file

- select multiple files

- search

- [DONE]dynamic array management

- rename files

- cut and paste files

- delete files

- tar support

- Use LS_COLORS

- print file sizes

- [DONE] list directories

- [DONE] handle terminal size

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
