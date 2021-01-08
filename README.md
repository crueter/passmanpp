# 2.0
passman++ version 2.0 is coming soon. It will feature a massive suite of new security and general features. Please see [#2.0 UPDATE](https://github.com/binex-dsk/passmanpp#20-update) for more info on what I plan to do.

While this is happening, no other updates will come out. Occasionally, however, I WILL commit my progress. Don't expect most if not any of the commits to work when directly cloned.

2.0.0rc5 is the latest pre-release build. It contains just about every planned feature change for 2.0, except for a few things, including but not limited to:
- A few GUI updates
- Some extra password generator features

# passman++
An extremely simple, minimal, and easy-to-use yet just as secure and powerful command-line and GUI password manager.

# Debug
Define the environment variables `PASSMAN_DEBUG` and/or `PASSMAN_VERBOSE` to any value and you will activate debug/verbose mode, which will give you extra output. Sometimes I may ask you to use this in a bug report.

# Security
If you find any security vulnerabilities at ALL, check `SECURITY.md` to report it.

By using passman++, all security is tested thoroughly, using tried-and-true standards with no known security holes.

Your password and its hash are NEVER stored absolutely ANYWHERE. All operations involving your password require you to input the password, and all verification is done by Botan's AES implementation. Every single bit of data is only accessible to those who know the password, which is your ONLY way to get access to it. Without your password, there is NO way to get your data, so keep it safe somewhere!

Password generation is done using libsodium's cryptographically secure random number generator. All your passwords will appear to be almost, if not completely random.

However, someone can still delete your database, so keep backups of your database by hitting Ctrl+Shift+S on the main screen, and store your databases in safe locations that only you have access to. You could even make a QR code of your database. Store it properly and you'll be fine.

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
- Otherwise, for 2.0.0rc3 and newer, simply run:
```bash
$ ./build.sh
```
- Or `./install.sh` if you want to install

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
- [KeePassXC](https://github.com/keepassxreboot/keepassxc) for the inspiration, boatloads of feature ideas, and a general help for when I need to implement a more complex feature
- And of course, the Botan ([randombit](https://github.com/randombit)) and libsodium devs ([jedisct1](https://github.com/jedisct1)).

# To Do
- Separate users
- My own encryption algorithm
- Multiple different formats
- Groups (sort of like KeePass's directories)
- Maybe... just maybe... KeePass* format integration
- Some sort of vault (like Plasma Vault)

# Already Done
Stuff I planned to do and have already done:
- Backing up databases
- Using Qt for a GUI
- More error handling
- Streamlined way to access your data (aka, the GUI)

# 2.0 UPDATE
All planned updates for passman++ 2.0:
- Add extra included and excluded characters for random password generator via user input
- Make password generator global
- Make password generator buttons more evident whether they are pressed or not
- Password health/entropy checker
- Rather than deleting and creating a new entry, rename it
- Utilize the password-show-* icons more

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
- Database config and changing it on-the-fly
- Switch to CMake
- Add some help
- Organize entry modifier into a table view thing
- Reduce the use of SQLite and directly edit entry data
- Add command-line options
- Create a full GUI interface
- Clear clipboard after a set amount of time when password is copied
- Password copy button
- Allow editing of Argon2id memory usage
- Change conversion dialog to use the regular password entry thingy
- Ability to disable compression
- Show notes on the table thing
- More advanced password generator
- Even more error handling
- EASCII in password generator

# 2.1 UPDATE
Plans for the passman++ 2.1 update:
- Separate more things into real "menus" with tabs and such
- Allow for no password at all
- Dedicated settings area with more options
- Locking out the database
- Timer for when the database should be automatically locked, or if it should be locked upon losing focus of the main window
- When the computer is hibernated or put to sleep, lock the database
- Add a delay benchmark for hashing iterations
- Allow for more stuff to be stored there, i.e. user-input attributes
  * YES, I have moved this to here. Dont feel like doing it in 2.0
- Store passwords in a secure vector

# 3.0 UPDATE
Plans for the passman++ 3.0 update:
- Allow for duplicate entries, give each entry a "UUID" to allow for this (making the table name the UUID)
- Entry modification dates
- pdb-to-pdpp file converter (i.e. converting the old PyPassMan format to this much more secure version)
- Icons and attachments

# Extras
This program is only intended to work under Linux. Feel free, however, to compile this for Windows and use it yourself, or even distribute it separately. However, as per the BSD License, you are required to credit me, and include the same BSD License in your version.
