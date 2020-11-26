# 2.0
passman++ version 2.0 is coming soon. It will feature a massive suite of new security and general features. Please see section "2.0 SHIT" for more info on what I plan to do.

While this is happening, no other updates will come out. Occasionally, however, I WILL commit my progress. Don't expect most if not any commits to work when directly cloned.

# passman++
An extremely simple and minimal yet just as secure and powerful command-line and GUI password manager. Super easy to use, with a lot of GUI implementation for even easier use.

# Building
After downloading the source zip or tar.gz and extracting (DO NOT GIT CLONE! THE MAIN BRANCH MAY BE UNSTABLE!), make sure you have [Botan](https://github.com/randombit/botan/), [libsodium](https://github.com/jedisct1/libsodium), and [Qt](https://qt.io) installed, and then:
```bash
$ cd src
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
Feel free to contribute whatever you want. It's currently in an early stage of development, as I have plans to expand this project a lot. So any help is appreciated!

# Acknowledgements
passman++ is made possible by:

- The following FOSS libraries:
 * [Botan](https://github.com/randombit/botan/)
 * [Qt](https://qt.io)
 * [libsodium](https://github.com/jedisct1/libsodium)
- The amazing people on the [Qt forum](https://forum.qt.io) for helping me debug many of my internal issues
- My friend Lenny for originally helping me create PyPassMan's AES version
- The Qt devs and [docs](https://doc.qt.io)
- [KeePassXC](https://github.com/keepassxreboot/keepassxc) for the idea
- And of course, the Botan ([randombit](https://github.com/randombit)) and libsodium devs ([jedisctl](https://github.com/jedisct1)).

# To Do
Like I said I've got some huge plans for this project. Here's some of them:
- Separate users
- My own encryption algorithm
- pdb-to-pdpp file converter (i.e. converting the old PyPassMan format to this much more secure version)
- Multiple different formats
- Groups (clarify: what does this mean? can't remember but I'll figure it out)
- Maybe... just maybe... KeePass* format integration
- Some sort of vault (like Plasma Vault)

So from all of this you can clearly tell, I've got some fuckin ambitions. In fact, the original inspiration for the passman project was that I wanted a more lightweight version of KeePassXC. Which should be obvious looking at the amount of overlap between my planned shit and KeePassXC's features. Yeah. 

# Already Done
Stuff I planned to do and have already done:
- Backing up databases
- Using Qt for a GUI (IN PROGRESS!)
- More error handling
- Streamlined way to access your data (aka, the GUI)
- More advanced password generator (partially done; will add extended ASCII maybe)
- Better entry modification tool

# 2.0 SHIT
shit I need to do for 2.0:
- Create a full GUI interface
- Password health/entropy checker
- Icons and attachments
- Key file implementation
- Allow for more stuff to be stored there, i.e. user-input attributes
- More advanced password generator + EASCII
- Allow choosing different checksum, derivation, hashing, and encryption methods (mostly done - test out every single combination and ensure they work)
  * AES-256/GCM, Twofish
  * Blake2b(256), SHA-3(256), SHAKE-256(256), Skein-512, SHA-256
  * Argon2id, Bcrypt-PBKDF, or no hashing at all
  * Potentially see if any more PBKD functions can be used
- Database config (sort of maybe done but not really?)
- Create a proper .pdpp file format with a header; docs for header (NEARLY DONE)

# 2.0 SHIT THATS DONE
- Properly align the randomize/view icons for EntryDetails() (DONE)
- Allow users to choose the number of iterations of hashing (to slow down bruteforcing) (DONE)
- Database UUIDs (DONE)
- Proper name/description for files in the header (DONE)

# Relation to PyPassMan
The original AES Python version works great, however, I quickly realized that if I wanted people to use this, I needed to add more features and make it overall just better. And thus, the idea of a C++ passman rewrite was born sometime late September. And the first release came to be on October 21, 2020.

At the moment, passman++ is superior in every way to the original Python project. Far more secure, more features, easier to use, faster, the list goes on and on, plus a large amount of Qt implementations. Either way, use this. It's gonna be getting way better, and I'm not developing the Python project anymore because holy FUCK C++ is SO much better than Python. And WAY more fun. Wow.

The .pdpp format that passman++ uses is **not** compatible with passman's .pdb format. The two have nearly identical bodies but they use different password protection implementations and different headers. Some day I'll write a pdb-to-pdpp converter. I highly doubt it'll be easy tho.

# Security
Technical side out of the way...

By using passman++, all security is tested thoroughly, using tried-and-true standards with no known security holes.

Your password and its hash are NEVER stored absolutely ANYWHERE. All operations involving your password require you to input the password, and all verification is done by Botan's AES implementation. Every single bit of data is only accessible to those who know the password, which is your ONLY way to get access to it. Without your password, there is NO way to get your data, so keep it safe somewhere!

Password generation is done using libsodium's cryptographically secure random character generator. All your passwords will appear to be almost, if not completely random.

The only "security hole" this has is somebody deleting your database files. Keep backups using the `backup` command. Store your databases in safe locations that only you have access to. Hell, you could even make a QR code of your database. Just, store it properly. Please. I'm not going through THAT again.

This program is only intended to work under Linux. Feel free, however, to compile this for Windows and use it yourself, or even distribute it separately. REMEMBER TO CREDIT ME!
