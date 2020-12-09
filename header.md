# .pdpp format
The  `.pdpp` format used by passman++. These docs are useful for knowing the inner workings of passman++, or simply for integrating `.pdpp` support into your application.

# Header
The header contains the magic number, version number, all information needed to work with the database, and the name and description.
- 4 bytes: PD++ (magic number)
- 1 byte: version number
- 1 byte: checksum choice:
  * 0 = Blake2b
  * 1 = SHA-3
  * 2 = SHAKE-256
  * 3 = SKEIN-512
  * 4 = SHA-512
  * The output length, in bytes, for all except SHA-512, is the maximum key length for the encryption option chosen.
- 1 byte: derivation choice:
  * 0 = PBKDF2(checksum choice)
  * More maybe to come - this option will probably be removed in the final 2.0 release if I can't get others to work
- 1 byte: hash choice:
  * 0 = Argon2 + derivation
  * 1 = Bcrypt + derivation
  * 2 = no hash, only derivation
- 1 byte: number of hashing iterations
- 1 byte: keyfile required (0 or 1 for off/on)
- 1 byte: type of encryption
  * 0 = AES-256/GCM
  * 1 = TwoFish/GCM
  * 2 = SHACAL2/EAX
  * 3 = Serpent/GCM
- database IV
  * length of IV is the default nonce length of the encryption option chosen
- database name (null-terminated)
- database description (null-terminated)

# Data
The rest of the data is the encrypted SQLite data.
- create a table for each entry
  * basic name, email, url, notes, and password (all `text`)
  * notes' newlines are stored as " || char(10) || "
  * in the case of multiple newlines, this is truncated as 
  * allow for user input attributes, no duplicates, types: string, number, bool
    - store strings as `text`, numbers as `real`, and bools as `integer` (0 for off, anything else for on)
    - in future releases, there MAY be a multiline text option. It'll probably `blob` type or something
  * store each input attribute as its own column
  * (future) store an icon name as text, which refers to the system theme's icon of that name
- encrypt the table's CREATE TABLE and INSERT statements with the chosen encryption function, key is the password checksummed then hashed with the chosen checksum and hash methods (salted with the IV), IV is the database IV
- **BEFORE** encryption, compress with gzip

# Extra development help
See the `sql.cpp`, `database.cpp`, and `entry_handler.cpp` files for help with creating an implementation. In fact, feel free to straight-up use this backend - without any modifications. However, you'll probably want to do some - this is intended simply as a standard application, and GUIs must be accomodated to your type of application. Especially in later revisions, as the GUI will be drastically improved, and the CLI gone.

Nothing actually needs to be the same, though - feel free to use any SQLite backend you choose. However, you'll almost certainly need to use Botan - passman++ relies on many of its features that simply aren't available on others like Crypto++. Don't worry that Botan is just a C++ library though - it's got some C bindings. Check in the [handbook](https://botan.randombit.net/handbook/api_ref/) for more details.

NEVER use ANY beta standards (such as this one). These are HIGHLY UNSTABLE and subject to breaking changes. Always, always, ALWAYS create your implementation using the latest release's standard format.

ALL attributes for entries MUST be editable. In the case of a name update, remove the old name and create a new table with the new name.

The SQLite is flexible to your will - you can freely create new connections and store configuration there, for example. However, always keep separate tables for entries. NEVER create just a single table for entry data, as this will break compatibility!

Finally, once again, look at my implementations first. It will give you a good guide on how to read and interact with the data, generate databases, create SQL statement sequences, and much, much more. Like I said, you can even use that as your backend if you please.

An approach I wouldn't object to would simply to be including passman++ with a few modifications in your application, and just using that to interface with databases. This isn't the best idea, though - it might make cross-format integration hard.
