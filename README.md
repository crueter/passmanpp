# passman++
A simple and minimal yet powerful and secure command-line password manager. This can be used the same way as the original passman.

# Building
Building is simple. Make sure you have the Botan library installed, then run:
```
g++ main.cpp -I "/usr/include/botan-2/" -lbotan-2 -lsqlite3 -lstdc++fs
```
The flags are necessary. Will add a Makefile soon

# Contributing
Feel free to contribute whatever you want. It's currently in a very, very early stage of development, as I have plans to expand this project a lot. So any help is appreciated!

# Plans
Like I said I've got some huge plans for this project. Here's some of them:
- Backing up databases
- Separate users
- Using Qt for a GUI
- Key files and other forms of extra security
- My own encryption algorithm
- More error handling
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
- Transforming data to prevent bruteforcing with standard AES

So from all of this you can clearly tell, I've got some fuckin ambitions. In fact, the original inspiration for the passman project was that I wanted a more lightweight version of KeePassXC. The original AES Python version works great, however, I quickly realized that if I wanted people to use this, I needed to add more features and make it overall just better. And thus, the idea of a C++ passman rewrite was born sometime late September. And the first release came to be on October 21, 2020.

passman++ is currently equal in flexibility and power to the original, with some extra shit thrown in there. But after I get through a few of these plans, it'll be superior in every way. yeah
