# 2.0
passman++ version 2.0 is coming soon. It will feature a massive suite of new security and general features. Please see [#2.0 UPDATE](https://github.com/binex-dsk/passmanpp#20-update) for more info on what I plan to do.

While this is happening, no other updates will come out. Occasionally, however, I WILL commit my progress. Don't expect most if not any of the commits to work when directly cloned.

2.0.0rc1 and 2.0.0rc2 are completely stable, tested, and working. You may use them if you please, as they contain nearly all the major changes from 2.0, but do not contain GUI or user-input attributes.

# passman++
An extremely simple, minimal, and easy-to-use yet just as secure and powerful command-line and GUI password manager.

# Relation to PyPassMan
I would NOT recommend using PyPassMan currently. As you've read, 2.0 will be a MASSIVE update, so much so that it'll be superior in every single conceivable way imaginable to PyPassMan. Plus, I stopped development of that a while ago and removed it from the AUR.

The .pdpp format that passman++ uses is **not** compatible with passman's .pdb format. They have different headers, config, password protection implementations, etc. Some day I'll write a pdb-to-pdpp converter.

# Debug
Define the environment variables `PASSMAN_DEBUG` and/or `PASSMAN_VERBOSE` to any value and you will activate debug/verbose mode, which will give you extra output. Sometimes I may ask you to use this in a bug report.

# Security
If you find any security vulnerabilities at ALL, check `SECURITY.md` to report it.

By using passman++, all security is tested thoroughly, using tried-and-true standards with no known security holes.

Your password and its hash are NEVER stored absolutely ANYWHERE. All operations involving your password require you to input the password, and all verification is done by Botan's AES implementation. Every single bit of data is only accessible to those who know the password, which is your ONLY way to get access to it. Without your password, there is NO way to get your data, so keep it safe somewhere!

Password generation is done using libsodium's cryptographically secure random number generator. All your passwords will appear to be almost, if not completely random.

However, someone can still delete your database, so keep backups using the `backup` command and store your databases in safe locations that only you have access to. You could even make a QR code of your database. Store it properly and you'll be fine.

# Building
## Manually
- Download the source tar.gz (or zip) file, from the [releases](https://github.com/binex-dsk/passmanpp/releases) page and extract it
- Install [Botan](https://github.com/randombit/botan/), [libsodium](https://github.com/jedisct1/libsodium), and [Qt](https://qt.io) 6
- For 1.4.0p and below, run:
```bash
$ cd src
$ qmake passman.pro
```
- For 2.0.0rc1 and rc2, run:
```bash
$ cmake .
```
- Then run:
```bash
$ make
```
- Optionally, to install it, run:
```bash
install -m755 passman /usr/local/bin/passman
```
- Otherwise, for anything newer than 2.0.0rc2, simply run:
```bash
$ bash build.sh
```
- Or `install.sh` if you want to install

NOTE: Building and installing bleeding-edge versions (directly cloning) may fail. It's safer to go with the latest pre-release if you want "beta" builds.

## AUR
Using yay:
```bash
$ yay -S passman++
```
Or the development (pre-release) version:
```bash
$ yay -S passman++-devel
```
Dependencies will be automatically installed in this case.

# Acknowledgements
passman++ is made possible by:

- The following FOSS libraries:
  * [Botan](https://github.com/randombit/botan/)
  * [Qt](https://qt.io)
  * [libsodium](https://github.com/jedisct1/libsodium)
- The amazing people on the [Qt forum](https://forum.qt.io) (and [randombit](https://github.com/randombit), too!) for helping me debug many of my internal issues
- My friend Lenny for originally helping me create PyPassMan's AES version
- The Qt devs and [docs](https://doc.qt.io)
- CMake
- [KeePassXC](https://github.com/keepassxreboot/keepassxc) for the idea
- And of course, the Botan ([randombit](https://github.com/randombit)) and libsodium devs ([jedisct1](https://github.com/jedisct1)).

# To Do
- Separate users
- My own encryption algorithm
- pdb-to-pdpp file converter (i.e. converting the old PyPassMan format to this much more secure version)
- Multiple different formats
- Groups (sort of like KeePass's directories)
- Maybe... just maybe... KeePass* format integration
- Some sort of vault (like Plasma Vault)
- Icons and attachments
- Password health/entropy checker
- More advanced password generator + EASCII

# Already Done
Stuff I planned to do and have already done:
- Backing up databases
- Using Qt for a GUI
- More error handling
- Streamlined way to access your data (aka, the GUI)
- More advanced password generator
- Better entry modification tool

# 2.0 UPDATE
All planned updates for passman++ 2.0:
- Create a full GUI interface
- Allow for more stuff to be stored there, i.e. user-input attributes

# 2.0 UPDATE: Already Done
Previously planned updates for passman++ 2.0 that have already been implemented:
- Properly align the randomize/view icons for EntryDetails()
- Allow users to choose the number of iterations of hashing (to slow down bruteforcing)
- Proper name/description for files in the header
- Key file implementation
- Create a proper .pdpp file format
- Allow choosing different checksum, derivation, hashing, and encryption methods
  * AES-256/GCM, Twofish/GCM, SHACAL2/EAX, Serpent/GCM
  * Blake2b, SHA-3, SHAKE-256, Skein-512, SHA-512
  * Argon2id, Bcrypt-PBKDF, or no hashing at all (only derivation)
  * Potentially see if any more PBKD functions can be used
- Database config and changing it on-the-fly
- Switch to CMake
- Add some help
- Organize entry modifier into a table view thing
- Reduce the use of SQLite and directly edit entry data
- Create a proper command-line interface (PARTIALLY DONE?)

# Extras
This program is only intended to work under Linux. Feel free, however, to compile this for Windows and use it yourself, or even distribute it separately. However, as per the BSD License, you are required to credit me, and include the same BSD License in your version.
