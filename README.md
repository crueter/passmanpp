# passman++
An extremely simple and minimal yet just as secure and powerful command-line and GUI password manager. Super easy to use, with a lot of GUI implementation for even easier use.

# Building
After downloading the source zip or tar.gz and extracting (DO NOT GIT CLONE! THE MAIN BRANCH MAY BE UNSTABLE!):
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
# Contributing
Feel free to contribute whatever you want. It's currently in an early stage of development, as I have plans to expand this project a lot. So any help is appreciated!

# To Do
Like I said I've got some huge plans for this project. Here's some of them:
- Separate users
- Using Qt for a GUI (basically done)
- Key files and other forms of extra security
- My own encryption algorithm
- pdb-to-pdpp file converter (i.e. converting the old PyPassMan format to this much more secure version)
- Attachments
- More advanced password generator (partially done; will add extended ASCII maybe)
- Multiple different formats
- Groups
- Maybe... just maybe... KeePass* format integration
- Password, security, etc. health checker
- Some sort of vault (like Plasma Vault)
- Icons and stuff
- Storing more things than just passwords
- Transforming data to prevent bruteforcing with standard AES (i.e. bit shifting, slowing down encryption and decryption to make bruteforcing slower)
- Some configuration (per-DB probably)

So from all of this you can clearly tell, I've got some fuckin ambitions. In fact, the original inspiration for the passman project was that I wanted a more lightweight version of KeePassXC. 

# Already Done
Stuff I planned to do and have already done:
- Backing up databases
- Using Qt for a GUI (basically done)
- More error handling
- Streamlined way to access your data (aka, the GUI)
- More advanced password generator (partially done; will add extended ASCII maybe)

# Relation to PyPassMan
The original AES Python version works great, however, I quickly realized that if I wanted people to use this, I needed to add more features and make it overall just better. And thus, the idea of a C++ passman rewrite was born sometime late September. And the first release came to be on October 21, 2020.

At the moment, passman++ is superior in every way to the original Python project. Far more secure, more features, easier to use, faster, the list goes on and on, plus a large amount of Qt implementations. Either way, use this. It's gonna be getting way better, and I'm not developing the Python project anymore because holy FUCK C++ is SO much better than Python. And WAY more fun. Wow.

The .pdpp format that passman++ uses is **not** compatible with passman's .pdb format. The two have nearly identical bodies but they use different password protection implementations and different headers. Some day I'll write a pdb-to-pdpp converter. I highly doubt it'll be reversible tho.

# Security
Technical side out of the way...

By using passman++, all security is tested thoroughly, using tried-and-true standards with no known security holes.

Your password and its hash are NEVER stored absolutely ANYWHERE. All operations involving your password require you to input the password, and all verification is done by Botan's AES implementation. Every single bit of data is only accessible to those who know the password, which is your ONLY way to get access to it. Without your password, there is NO way to get your data, so keep it safe somewhere!

Password generation is done using libsodium's cryptographically secure random character generator. All your passwords will appear to be almost, if not completely random.

I hold full responsibility for any internal security holes coded into the passman++ program. Any damages caused by security issues on this program are, within reason, the fault of me legally. However, I do not hold any responsibility for problems caused by your own stupidity. Remember: you are the only one who knows your password. It is up to you to remember that password, keep it private, and to properly store your data.

The only "security hole" this has is somebody deleting your database files. Keep backups using the `backup` command. Store your databases in safe locations that only you have access to. Hell, you could even make a QR code of your database. Just, store it properly. Please.

This program is only intended to work under Linux. Feel free, however, to compile this for Windows and use it yourself, or even distribute it separately. REMEMBER TO CREDIT ME!
