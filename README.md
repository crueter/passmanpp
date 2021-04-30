# passman++
An extremely simple, minimal, and easy-to-use yet just as secure and powerful GUI password manager.

# Security
If you find any security vulnerabilities at ALL, check `SECURITY.md` to report it.

By using passman++, all security is tested thoroughly, using tried-and-true standards with no known security holes.

Your password is never stored absolutely anywhere. All of your sensitive data, be it your master password, entry data/passwords, additional database info, etc. are all stored in memory that is inaccessible via memory dumps and is completely cleared when necessary.

Every single bit of data is only accessible to those who know the password, which is your ONLY way to get access to it. Without your password, there is NO way to get your data, so keep it safe somewhere!

Password generation is done using libsodium's cryptographically secure random number generator. All your passwords will appear to be almost, if not completely random.

However, someone can still delete your database, so keep backups of your database by hitting Ctrl+Shift+S on the main screen (or File > Save As...), and store your databases in safe locations that only you have access to. You could even make a QR code of your database. Store it properly and you'll be fine.

# Building
## Manually
- Download the source tar.gz (or zip) file, from the [releases](https://github.com/binex-dsk/passmanpp/releases) page and extract it
- Install [Botan](https://github.com/randombit/botan/), and [Qt](https://qt.io) 6 through your distribution's package manager, or [install from the AUR](https://github.com/binex-dsk/passmanpp#AUR).
- For 1.4.0p and below, run:
```bash
$ cd src
$ qmake passman.pro
```
- For 2.0.0-2.0.1, simply run:
```bash
$ ./build.sh
```
  * Or `./install.sh` if you want to install
- For 2.1.0 and newer, run the following commands:
```bash
$ cmake -S . -B build
$ cmake --build build
```
Optionally, to install the files (run as root):
```bash
# cmake --build build --target install
```
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
Or the LTS version:
```bash
$ yay -S passman++-lts
```
Dependencies will be automatically installed in these cases.

# Acknowledgements
passman++ is made possible by:

- me
- The following FOSS libraries:
  * [Botan](https://github.com/randombit/botan/)
  * [Qt](https://qt.io)
- The amazing people on the [Qt forum](https://forum.qt.io) (and [randombit](https://github.com/randombit), too!) for helping me debug many of my internal issues
- The Qt devs and [docs](https://doc.qt.io)
- CMake
- [KeePassXC](https://github.com/keepassxreboot/keepassxc) for the inspiration, boatloads of feature ideas, and a general help for when I need to implement a more complex feature
- And of course, the Botan devs ([randombit](https://github.com/randombit))

# Debug
On the command line, specify the arguments `-d` or `--debug` and `-V` or `--verbose` and you will activate debug/verbose mode respectively, which will give you extra output. Sometimes I may ask you to use this in a bug report.

# FAQ
## Why is this so slow on my computer?
I have genuinely no idea how you can actually have such a terrible computer that an incredibly simple password manager is slow, but, if unlocking databses takes some time, edit your database, and select hash, HMAC, and encryption functions to speed it up, and turn down hashing iterations. You can use the benchmark button to test their speed. See [Choosing Options](https://github.com/binex-dsk/passmanpp/blob/main/Choosing%20Options.md).

## How do I launch this without using the command line?
(>=2.1.0) From your application launcher (start menu for your DE, or through dmenu, rofi, etc), launch "passman++". From a file manager you can also open any `.pdpp` files and passman++ will open with that file.

(<=2.0.1) No desktop file is provided for 2.0.1 and below. Sorry.

## Do I have to remake all of my databases for 2.0.0?
Nope! If you use an old database with 2.0.0 or above, passman++ will automatically convert it to the new format. However, you might be better off recreating it.

## Can you automate installing dependencies or include the libraries in an archive for releases?
cba, install it yourself lol

## Why do you use this entire library when you use so little of it? Can't you just implement it yourself?
Yes, and in the future, I plan on implementing a few things from the currently used libraries, like the random byte generator. The reason I use these libraries in the first place, though, is because they are significantly better than anything I can do; not only have they been developed for far longer, but it's also much less work that I have to do.

## Will you make Windows or Mac versions?
Unlikely, unless I can figure out a way to statically link Qt 6 and Botan with MinGW. As for Mac, yeah no never.

## Why would you even make this when things like KeePassXC already exist and are better in every single regard?
I plan to change that in the future ;)

Anyways, with pretty much all of my applications, they aren't intended to be the "best" version of that particular type of application; they are instead alternatives, usually much more lightweight, or, in rare cases like with this one, have a few features that others don't have.

Oh yeah, it also has seriously improved my skill with C++.

## Can I import databases from KeePass?
Not yet. In the future, however, I do plan on implementing that, since the database formats are documented. However, they're also retardedly overcomplicated. Like, seriously.

## Why don't you conform your code to standard X?
If I don't conform my code to a particular standard, there are a few reasons why:
- Too much work to care about
- I use a different standard that I like better (i.e. variable/file naming conventions)
- Little to no benefits
- The standard makes no sense

## Rewrite it in Rust!
nomegalul

# To Do
- Separate users/groups
- Multiple different formats
- KeePass* database integration
- Separate more things into real "menus" with tabs and stuff
- self note: `constexpr`s and `noexcept`s

# Already Done
Stuff I previously planned to do and have already done:
- Backing up databases
- More error handling
- Allow users to choose the number of iterations of hashing (to slow down bruteforcing)
- Proper name/description for files in the header
- Key file implementation
- Create a proper .pdpp file format
- Allow choosing different HMAC, derivation, hashing, and encryption methods
  * AES-256/GCM, Twofish/GCM, SHACAL2/EAX, Serpent/GCM
  * Blake2b, SHA-3, SHAKE-256, Skein-512, SHA-512
  * Argon2id, Bcrypt-PBKDF, Scrypt, or no hashing at all (only derivation)
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
- Add extra included and excluded characters for random password generator via user input
- Make password generator global
- Make password generator buttons more evident whether they are pressed or not
- Rather than deleting and creating a new entry, rename it
- Store passwords in locked memory
- Make some tooltips and text more concise
- More shortcuts
- Fix some oddities with the GUI
- Put limits and restrictions on certain values to stop segfaults, memory leaks, etc.
- Separate widgets and dialogs
- Integrate more things into appearing on the main database window
- More intuitive settings area, with tabs and such
- Locking out the database
- Better CMakeLists.txt
- MIME type and desktop files
- Add a delay benchmark for hashing iterations
- Create a libpassman as a base for third-party `.pdpp` file integration

# 2.2 UPDATE
- Allow for more stuff to be stored there, i.e. user-input attributes
- When the computer is hibernated or put to sleep, close the app and deallocate all memory
- Allow for no password at all
- Password health/entropy checker
- Separate database creation and database configuration, plus a "simple" and "advanced" mode for each
- Timer for when the database should be automatically locked, or if it should be locked upon losing focus of the main window
- Global configuration
- Display entropy in password generator
- CLI application

# 3.0 UPDATE
Plans for the passman++ 3.0 update:
- Allow for duplicate entries, give each entry a "UUID" to allow for this (making the table name the UUID)
- Entry modification dates
- pdb-to-pdpp file converter
- Icons and attachments
- some GPG stuff
