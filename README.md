# Custom File System — Team CATS

We built a custom file system in C for our CSC 415 team project. It stores files
and directories in a persistent volume file and provides an interactive shell
for navigating directories, managing files, and transferring data between the
custom volume and the host file system.

Our implementation covers volume initialization, free-space management,
directory operations, and buffered file I/O. It builds on the provided
block-level I/O library and shell framework.

## What We Built

- **Volume formatting and persistence:** A volume control block records the
  volume signature, block size, block count, and locations of the root directory
  and free-space map. Startup formats a new volume or loads an existing one.
- **Extent-based storage:** Contiguous runs of blocks represent available space
  and file data. The allocator uses a first-fit strategy, and released blocks
  return to the free-space map. Secondary extent tables support file growth
  beyond the extent slots stored directly in a directory entry.
- **Hierarchical directories:** Directories contain `.` and `..` entries and
  support creation, removal, iteration, and expansion. Path parsing resolves
  absolute and relative paths, and working-directory functions support navigation.
- **Buffered file operations:** `b_open`, `b_read`, `b_write`, `b_seek`, and
  `b_close` manage file descriptors, buffers, and positions within file extents.
  Opening files supports creation, truncation, append positioning, and read/write
  access modes.
- **File metadata:** Directory entries track names, file types, sizes, storage
  extents, and creation, modification, and access timestamps. `fs_stat` exposes
  file information to the shell.
- **Shell integration:** The shell connects these operations to commands for
  listing, copying, moving, creating, and deleting files, including imports and
  exports between the custom volume and the host.

## Build and Run

The project targets Linux and uses GCC, Make, and the GNU Readline development
library. The repository includes precompiled Linux block-I/O objects for x86-64
and AArch64; the Makefile selects one based on the host architecture.

```sh
make
make run
```

By default, `make run` opens `SampleVolume` with a requested volume size of
10,000,000 bytes and a block size of 512 bytes. To use a separate volume:

```sh
make run RUNOPTIONS="MyVolume 10000000 512"
```

A new volume is initialized on first use. An existing volume with our file-system
signature is loaded so its contents remain available across sessions. Enter
`exit` to close the shell.

## Shell Commands

| Command | Purpose |
| --- | --- |
| `ls` | List directory contents. |
| `md <path>` | Create a directory. |
| `cd <path>` | Change the working directory. |
| `pwd` | Print the working directory. |
| `touch <file>` | Create or touch a file. |
| `cat <file>` | Display file contents using the shell's limited `cat` implementation. |
| `cp <source> <destination>` | Copy a file within the custom volume. |
| `mv <source> <destination>` | Move a regular file by copying it and deleting the source; directory moves are not supported. |
| `rm <path>` | Remove a file or an empty directory. |
| `cp2fs <host-source> <destination>` | Import a file from the host file system. |
| `cp2l <source> <host-destination>` | Export a file to the host file system. |
| `history` | Show command history. |
| `help` | List shell commands. |
| `exit` | Close the file system and exit. |

For example, inside the shell:

```text
md documents
cp2fs README.md /documents/readme.txt
cd /documents
ls
cat readme.txt
pwd
exit
```

## Source Layout

| Files | Responsibility |
| --- | --- |
| `fsInit.c`, `fsVCB.c`, `fsVCB.h` | Volume formatting, loading, and shutdown. |
| `fsStorage.c`, `fsStorageExts.c`, `fsStorage.h` | Free-space allocation, release, and extent-table helpers. |
| `fsDir.c`, `fsDir.h`, `fsDirSearch.c` | Directory storage, growth, entry lookup, and path parsing. |
| `mfsCwd.c`, `mfsDirItr.c`, `mfsGod.c`, `mfsInfo.c`, `mfs.h` | Public directory, navigation, deletion, and metadata operations. |
| `b_io.c`, `b_io.h` | Buffered file I/O and seeking. |
| `fsshell.c` | Interactive command-line interface. |
| `fsLow.h`, `fsLow.o`, `fsLowM1.o` | Provided partition and logical-block I/O interface and binaries. |
| `Hexdump/` | Provided utility for inspecting volume contents. |

## Project Documentation

- [Final project writeup](csc415-filesystem-CATS-writeup.pdf)
- [Milestone 1 writeup](Milestone1_Writeup_milo-pa.pdf)
- `SampleVolume`: included volume file for running and exploring the project.
