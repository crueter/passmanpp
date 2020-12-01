# 2.0
passman++ version 2.0 is coming soon. It will feature a massive suite of new security and general features. Please see [#2.0 UPDATE](https://github.com/binex-dsk/passmanpp#20-update) for more info on what I plan to do.

While this is happening, no other updates will come out. Occasionally, however, I WILL commit my progress. Don't expect most if not any commits to work when directly cloned.

# passman++
An extremely simple and minimal yet just as secure and powerful command-line and GUI password manager. Super lightweight and easy to use.

# Building
After downloading the source zip or tar.gz and extracting (DO NOT GIT CLONE! THE MAIN BRANCH IS UNSTABLE!), make sure you have [Botan](https://github.com/randombit/botan/), [libsodium](https://github.com/jedisct1/libsodium), and [Qt](https://qt.io) installed, and then:
```bash
$ qmake passman.pro
$ make
```
Optionally:
```bash
# install -m755 passman /usr/bin/passman
```
Or on Arch Linux (with yay):
```bash
$ yay -S passman++
```
Dependencies will be automatically installed in this case.
# Contributing
Feel free to contribute whatever you want.

# Acknowledgements
passman++ is made possible by:

- The following FOSS libraries:
  * [Botan](https://github.com/randombit/botan/)
  * [Qt](https://qt.io)
  * [libsodium](https://github.com/jedisct1/libsodium)
- The amazing people on the [Qt forum](https://forum.qt.io) (and [randombit](https://github.com/randombit), too!) for helping me debug many of my internal issues
- My friend Lenny for originally helping me create PyPassMan's AES version
- The Qt devs and [docs](https://doc.qt.io)
- [KeePassXC](https://github.com/keepassxreboot/keepassxc) for the idea
- And of course, the Botan ([randombit](https://github.com/randombit)) and libsodium devs ([jedisctl](https://github.com/jedisct1)).

# To Do
- Separate users
- My own encryption algorithm
- pdb-to-pdpp file converter (i.e. converting the old PyPassMan format to this much more secure version)
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
- More advanced password generator
- Better entry modification tool

# 2.0 UPDATE
All planned updates for passman++ 2.0:
- Create a full GUI interface
- Password health/entropy checker
- Icons and attachments
- Allow for more stuff to be stored there, i.e. user-input attributes
- More advanced password generator + EASCII
- Switch to CMake
- Add some help

# 2.0 UPDATE: Already Done
Previously planned updates for passman++ 2.0 that have already been implemented:
- Properly align the randomize/view icons for EntryDetails()
- Allow users to choose the number of iterations of hashing (to slow down bruteforcing)
- Database UUIDs
- Proper name/description for files in the header
- Key file implementation
- Create a proper .pdpp file format
- Allow choosing different checksum, derivation, hashing, and encryption methods
  * AES-256/GCM, Twofish
  * Blake2b(256), SHA-3(256), SHAKE-256(256), Skein-512, SHA-256
  * Argon2id, Bcrypt-PBKDF, or no hashing at all (only derivation)
  * Potentially see if any more PBKD functions can be used
- Database config and changing it on-the-fly

# Relation to PyPassMan
I would NOT recommend using PyPassMan currently. As you've read, 2.0 will be a MASSIVE update, so much so that it'll be superior in every single possible conceivable way imaginable to PyPassMan. Plus, I stopped development of that a while ago and removed it from the AUR.

The .pdpp format that passman++ uses is **not** compatible with passman's .pdb format. They have different headers, config, password protection implementations, etc. Some day I'll write a pdb-to-pdpp converter.

# Security
Technical side out of the way...

By using passman++, all security is tested thoroughly, using tried-and-true standards with no known security holes.

Your password and its hash are NEVER stored absolutely ANYWHERE. All operations involving your password require you to input the password, and all verification is done by Botan's AES implementation. Every single bit of data is only accessible to those who know the password, which is your ONLY way to get access to it. Without your password, there is NO way to get your data, so keep it safe somewhere!

Password generation is done using libsodium's cryptographically secure random character generator. All your passwords will appear to be almost, if not completely random.

The only "security hole" this has is somebody deleting your database files. Keep backups using the `backup` command. Store your databases in safe locations that only you have access to. Hell, you could even make a QR code of your database. Store it properly and you'll be fine.

This program is only intended to work under Linux. Feel free, however, to compile this for Windows and use it yourself, or even distribute it separately. However, as per the BSD License, you are required to credit me, and include the same BSD License in your version.
