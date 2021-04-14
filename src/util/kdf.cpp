#include <botan/compression.h>
#include <QElapsedTimer>

#include "kdf.hpp"

KDF::KDF(const QVariantMap &p) {
    setParams(p);
}

bool KDF::setParams(const QVariantMap &p) {
    uint16_t i1 = static_cast<uint16_t>(p.value("i1", 0).toUInt());
    setI1(i1);

    uint16_t i2 = static_cast<uint16_t>(p.value("i2", 0).toUInt());
    setI2(i2);

    uint16_t i3 = static_cast<uint16_t>(p.value("i3", 0).toUInt());
    setI3(i3);

    uint8_t hmac = static_cast<uint8_t>(p.value("hmac", 0).toUInt());
    if (!setHmacFunction(hmac)) {
        return false;
    }

    uint8_t hash = static_cast<uint8_t>(p.value("hash", 0).toUInt());
    if (!setHashFunction(hash)) {
        return false;
    }

    uint8_t encryption = static_cast<uint8_t>(p.value("encryption", 0).toUInt());
    if (!setEncryptionFunction(encryption)) {
        return false;
    }

    VectorUnion seed = p.value("seed").toByteArray();
    if (!setSeed(seed)) {
        return false;
    }

    VectorUnion keyFile = p.value("keyfile", "").toString();
    if (!setKeyFile(keyFile)) {
        return false;
    }

    return true;
}

uint16_t KDF::i1() {
    return m_i1;
}

void KDF::setI1(uint16_t t_i1) {
    m_i1 = t_i1;
}

uint16_t KDF::i2() {
    return m_i2;
}

void KDF::setI2(uint16_t t_i2) {
    m_i2 = t_i2;
}

uint16_t KDF::i3() {
    return m_i3;
}

void KDF::setI3(uint16_t t_i3) {
    m_i3 = t_i3;
}

uint16_t KDF::rounds() {
    switch (hashFunction()) {
        case 1: {
            return i1();
        } default: {
            return i2();
        }
    }
}

uint16_t KDF::memoryUsage() {
    switch (hashFunction()) {
        case 0: {
            return i1();
        } case 2: {
            return static_cast<uint16_t>(128 * i1() * i2());
        } default: {
            return 0;
        }
    }
}

uint16_t KDF::parallelism() {
    return i3();
}

uint8_t KDF::hmacFunction() {
    return m_hmacFunction;
}

bool KDF::setHmacFunction(uint8_t t_hmacFunction) {
    if (t_hmacFunction >= Constants::hmacMatch.size()) {
        return false;
    }

    m_hmacFunction = t_hmacFunction;
    return true;
}

uint8_t KDF::hashFunction() {
    return m_hashFunction;
}

bool KDF::setHashFunction(uint8_t t_hashFunction) {
    if (t_hashFunction >= Constants::hashMatch.size()) {
        return false;
    }

    m_hashFunction = t_hashFunction;
    return true;
}

uint8_t KDF::encryptionFunction() {
    return m_encryptionFunction;
}

bool KDF::setEncryptionFunction(uint8_t t_encryptionFunction) {
    if (t_encryptionFunction >= Constants::encryptionMatch.size()) {
        return false;
    }

    m_encryptionFunction = t_encryptionFunction;
    return true;
}


VectorUnion KDF::seed() {
    return m_seed;
}

bool KDF::setSeed(VectorUnion t_seed) {
    auto encryptor = makeEncryptor();
    if (encryptor->default_nonce_length() != t_seed.size()) {
        return false;
    }

    m_seed = t_seed;
    return true;
}

VectorUnion KDF::keyFile() {
    return m_keyFile;
}

VectorUnion KDF::readKeyFile() {
    QFile kf(keyFile().asQStr());
    kf.open(QIODevice::ReadOnly);
    QTextStream key(&kf);

    return key.readAll();
}

bool KDF::setKeyFile(VectorUnion t_keyFile) {
    QFile f(t_keyFile.asQStr());
    if (!f.exists()) {
        return false;
    }

    f.close();
    m_keyFile = t_keyFile;
    return true;
}


std::unique_ptr<Botan::Cipher_Mode> KDF::makeEncryptor(uint8_t t_encryptionFunction) {
    if (t_encryptionFunction == 63) {
        t_encryptionFunction = encryptionFunction();
    }

    return Botan::Cipher_Mode::create(Constants::encryptionMatch.at(t_encryptionFunction), Botan::ENCRYPTION);
}

std::unique_ptr<Botan::Cipher_Mode> KDF::makeDecryptor(uint8_t t_encryptionFunction) {
    if (t_encryptionFunction == 63) {
        t_encryptionFunction = encryptionFunction();
    }

    return Botan::Cipher_Mode::create(Constants::encryptionMatch.at(t_encryptionFunction), Botan::DECRYPTION);
}

std::unique_ptr<Botan::PasswordHash> KDF::makeDerivation(uint8_t t_hmacFunction) {
    if (t_hmacFunction == 63) {
        t_hmacFunction = hmacFunction();
    }

    auto enc = makeEncryptor();
    std::string hmacChoice(Constants::hmacMatch[t_hmacFunction]);

    if (hmacChoice != "SHA-512") {
        hmacChoice += '(' + std::to_string(enc->maximum_keylength() * 8) + ')';
    }

    return Botan::PasswordHashFamily::create("PBKDF2(" + hmacChoice + ')')->default_params();
}

std::unique_ptr<Botan::PasswordHash> KDF::makeHasher(uint8_t t_hashFunction) {
    if (t_hashFunction == 63) {
        t_hashFunction = hashFunction();
    }

    qDebug() << tr(Constants::hashMatch.at(t_hashFunction));
    auto h = Botan::PasswordHashFamily::create(Constants::hashMatch.at(t_hashFunction))->from_params(i1(), i2(), i3());
    return h;
}


VectorUnion KDF::transform(VectorUnion t_data, VectorUnion t_seed) {
    if (t_seed.empty()) {
        t_seed = seed();
    }

    auto enc = makeEncryptor();
    if (hashFunction() < 3) {
        secvec ptr(512);
        auto hash = makeHasher();

        hash->derive_key(ptr.data(), ptr.size(), t_data.asConstChar(), t_data.size(), t_seed.data(), enc->default_nonce_length());

        if (qApp->property("verbose").toBool()) {
            qDebug() << t_data;
            qDebug() << "After Hashing:" << Botan::hex_encode(ptr).data() << Qt::endl;
        }

    }

    secvec ptr(enc->maximum_keylength());
    auto deriv = makeDerivation();

    deriv->derive_key(ptr.data(), ptr.size(), t_data.asConstChar(), t_data.size(), t_seed.data(), enc->default_nonce_length());

    if (qApp->property("verbose").toBool()) {
        qDebug() << toString() << t_seed << t_data;
        qDebug() << "After Derivation:" << Botan::hex_encode(ptr).data() << Qt::endl;
    }

    return ptr;
}

int KDF::benchmark(const int t_msec) {
    QByteArray key = QByteArray(16, '\x50');
    QByteArray seed = QByteArray(32, '\x2B');

    QElapsedTimer timer;
    timer.start();

    if (transform(key, seed)) {
        return static_cast<int>(rounds() * (static_cast<float>(t_msec) / static_cast<float>(timer.elapsed())));
    }

    return 1;
}

QString KDF::toString() {
    QString hash = hashFunction() < 3 ? tr(Constants::hashMatch.at(hashFunction())) : "None";
    QString hmac = tr(Constants::hmacMatch.at(hmacFunction()));
    QString encryption = tr(Constants::encryptionMatch.at(encryptionFunction()));

    uint16_t hrounds = rounds();
    uint16_t mem = memoryUsage();
    return tr(hash + ", using " + hmac + " and " + encryption + " (" + QString::number(hrounds) + " rounds, " + QString::number(mem / 1000) + " MB, 1 thread)");
}
