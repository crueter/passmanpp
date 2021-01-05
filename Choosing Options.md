Checksum functions:
 - Blake2b is very fast in software but relatively slow in hardware. Pretty much the most secure option. Default option.
 - SHA-3 (or Keccak) won the SHA-3 contest by NIST, replacing SHA-2. Slow in software, but very, very fast in hardware that supports it. Supposedly more secure than Blake2b, but information is limited, so they're used interchangeably and are generally considered to be equally secure.
 - SHAKE-256 is a subset of SHA-3 and can therefore be used interchangeably with it and Blake2b. Theoretically, it's more secure against collision attacks.
 - Skein-512 was a finalist in NIST's SHA-3 contest, based on Threefish. SHA-3 or Blake2b are generally preferred, but it hasn't been broken yet, so it's a viable alternative if you want to use it.
 - SHA-512 is extremely fast on 64-bit systems and that's the only reason to use it. Usually prefer anything else on newer hardware as they'll still be fast enough. May also sometimes not work with SHACAL2, though this is rare.

Password Hashing functions:
 - Argon2id was the winner of the Password Hashing Competition in July 2015. Hybrid of Argon2i and Argon2d combining the security functions of both. Extremely hard to bruteforce due to its very high memory requirements, but higher memory usage or hashing iterations can be very slow. Generally, you'll want to use this, as it's the best of the best, essentially. Default option for very obvious reasons.
 - Scrypt can be used interchangeably with Argon2id, and can actually be better in some cases. Be warned, however; higher iteration/workload values may seriously affect authentication speed.
 - Bcrypt-PBKDF should be avoided because Argon2id is better against bruteforce attacks. It's still very secure, however, so use it if you please.
 - No hashing with only a derivation function should also generally be avoided, but with something such as Blake2b it's still secure. It's also significantly faster.

Encryption functions:
 - AES-256/GCM has been the industry standard for an incredibly long time. Very fast, and basically the best you'll find. Due to how widely used it is, you'll likely find out very early on if a security vulnerability is found, meaning that I can replace it with a different encryption option quickly. Default option.
 - Twofish/GCM is "less" secure than AES, but in reality, it still hasn't been broken. If it does, however, due to it having less adoption as AES, it might take longer to find out when this happens. No real advantage to AES, but use if you want.
 - SHACAL2/EAX is very fast and just as secure. Relatively obscure, but standardized, so if you want unlocking to be fast, go ahead and use it. It's also "more versatile" than other options and can theoretically handle a much longer password.
 - Serpent/GCM is another alternative to AES and Twofish. No real advantage to AES, but use if you want.

Choosing options:
 - If you just want the "most" secure options, select Blake2b or SHA-3 or SHAKE-256, Argon2id, and AES-256/GCM.
 - If you just want the fastest options, select SHA-512, only derivation, and SHACAL2/EAX.
 - Select others if you really have a reason to or want some sort of compatibility with other programs for whatever reason.
 - More hashing iterations makes it slower. Making it slower makes bruteforcing harder but it takes longer for your password to authenticate. Generally you'll want a value below 20, as anything higher can be very, very slow unless you've got a modern desktop Ryzen.
 - If your hardware does not have hardware AES encryption, avoid using AES-256/GCM. Go with anything else, as AES is (seemingly) more vulnerable without hardware acceleration.

Other stuff:
 - More memory usage will make bruteforcing even harder.
 - Compression is seriously recommended as it will drastically reduce the size of your database.
 - Clipboard clear delay indicates how long the delay will be when you copy an entry's password.
