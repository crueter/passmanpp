Checksum functions:
 - Blake2b is very fast in software but relatively slow in hardware. Default option.
 - SHA-3 (or Keccak) won the SHA-3 contest by NIST, replacing SHA-2. It's slower in software but very, very fast in hardware that supports it. passman++ uses SHA3-256.
 - SHAKE256 is a subset of SHA-3 and can be used interchangeably with SHA3-256. It is, however, more secure against collision attacks (theoretically).
 - Skein-512 was a finalist in NIST's SHA-3 contest. Based on Threefish, SHA-3 or Blake2b are generally preferred but it hasn't been broken yet, so it's a viable alternative if you want to use it.
 - SHA-256 is rather insecure and is only provided for compatibility with pre-2.0 databases. Only use if you need the fastest possible.

Derivation functions:
 - PBKDF2 is the only one available. However, it's for a good reason: there's few if not no other options. It's been a proven standard and is easily compatible with all sorts of checksum functions.

Password Hashing functions:
 - Argon2id was the winner of the Password Hashing Competition in July 2015. Hybrid of Argon2i and Argon2d combining the security functions of both. Extremely hard to bruteforce due to its very high memory requirements, but higher memory usage or hashing iterations can be very slow. Generally, you'll want to use this, as it's the best of the best, essentially. Default option for very obvious reasons.
 - Bcrypt-PBKDF should be avoided because Argon2id is better against bruteforce attacks. It's still very secure, however, so use it if you please.
 - No hashing with only a derivation function should also generally be avoided, but with something such as Blake2b it's still secure.

Encryption functions:
 - AES-256/GCM has been the industry standard for a very, very long time. Very fast, and basically the best you'll find. Due to how widely used it is, you'll likely find out very early on if a security vulnerability is found, meaning that I can replace it with a different encryption option quickly. Default option.
 - Twofish is "less" secure, but in reality, it still hasn't been broken. No real advantage to AES, but use if you want.

Choosing options:
 - If you just want the "most" secure options, select Blake2b or SHA-3(256), Argon2id, and AES-256/GCM.
 - Select others if you really have a reason to or want some sort of compatibility with other programs for whatever reason.
 - More hashing iterations makes it slower. Making it slower makes bruteforcing harder but it takes longer for your password to authenticate. Generally you'll want a value below 20, as anything higher can be very, very slow unless you've got a modern desktop Ryzen.
