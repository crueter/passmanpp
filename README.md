# passman++
An extremely simple and minimal yet just as secure and powerful command-line password manager. Super easy to use, with some GUI implementation for even easier accessing of files.

This uses a format even simpler than the application itself, the `.pdpp` format. More information in #pdpp format.

# Building
After `git clone`ing:
```bash
$ cd src
$ qmake passman.pro
$ make
```
Optionally:
```bash
# install -m755 passman /usr/bin/passman
```
# Contributing
Feel free to contribute whatever you want. It's currently in an early stage of development, as I have plans to expand this project a lot. So any help is appreciated!

# To Do
Like I said I've got some huge plans for this project. Here's some of them:
- Separate users
- Using Qt for a GUI (partially done)
- Key files and other forms of extra security
- My own encryption algorithm
- pdb-to-pdpp file converter (i.e. converting the old PyPassMan format to this much more secure version)
- Attachments
- More advanced password generator
- Multiple different formats
- Groups
- Streamlined way to access your data (aka, the GUI)
- Maybe... just maybe... KeePass* format integration
- Password, security, etc. health checker
- Some sort of vault (like Plasma Vault)
- Icons and stuff
- Storing more things than just passwords
- Transforming data to prevent bruteforcing with standard AES (i.e. bit shifting, slowing down encryption and decryption to make bruteforcing slower)
- Some configuration (per-DB or per-program?)

So from all of this you can clearly tell, I've got some fuckin ambitions. In fact, the original inspiration for the passman project was that I wanted a more lightweight version of KeePassXC. 

# Already Done
Stuff I planned to do and have already done:
- Backing up databases
- Using Qt for a GUI (partially done)
- More error handling (mostly done, will add when needed)
- Streamlined way to access your data (aka, the GUI) (also partially done)

# Relation to PyPassMan
The original AES Python version works great, however, I quickly realized that if I wanted people to use this, I needed to add more features and make it overall just better. And thus, the idea of a C++ passman rewrite was born sometime late September. And the first release came to be on October 21, 2020.

At the moment, passman++ is much better than the original Python project. Far more secure, more features, easier to use, faster, the list goes on and on. Either way, use this. It's gonna be getting way better, and I'm not developing the Python project anymore because holy FUCK C++ is SO much better than Python. Wow.

The .pdpp format that passman++ uses is **not** compatible with passman's .pdb format. The two have nearly identical bodies but they use different password protection implementations and different headers. Some day I'll write a pdb-to-pdpp converter. I highly doubt it'll be reversible tho.

# pdpp format
The .pdpp format only contains a single line of header data, which is the hex-encoded IV to be used with decryption. The rest is the encrypted database and what actually gets you access to your data. Using SHA256-hashed password protection to ensure all keys are the same length, the body contains AES-256/GCM encrypted SQLite3 INSERT data that programmatically restores the database to its last saved state perfectly.

Encryption is done simply by generating a series of CREATE TABLE and INSERT statements, with those INSERT statements containing all stored entries' data. With that generated series of statements, the database's master password is SHA-256 hashed, then used as the AES-256/GCM key, along with a randomly generated IV, to encrypt the series of statements, storing it into a file.

# Security
Technical side out of the way...

By using passman++, all security is tested thoroughly, using tried-and-true standards with no known security holes.

Your password and its hash are NEVER stored absolutely ANYWHERE. All operations involving your password require you to input the password, and all verification is done by Botan's AES implementation. Every single bit of data is only accessible to those who know the password, which is your ONLY way to get access to it. Without your password, there is NO way to get your data, so keep it safe somewhere!

Password generation is done using libsodium's cryptographically secure random character generator. All your passwords will appear to be almost, if not completely random.

I hold full responsibility for any internal security holes coded into the passman++ program. Any damages caused by security issues on this program are, within reason, the fault of me legally. However, I do not hold any responsibility for problems caused by your own stupidity. Remember: you are the only one who knows your password. It is up to you to remember that password, keep it private, and to properly store your data.

The only "security hole" this has is somebody deleting your database files. Keep backups using the `backup` command. Store your databases in safe locations that only you have access to. Hell, you could even make a QR code of your database. Just, store it properly. Please.

This program is only intended to work under Linux. Feel free, however, to compile this for Windows and use it yourself, or even distribute it separately. REMEMBER TO CREDIT ME!
