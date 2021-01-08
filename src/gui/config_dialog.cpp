#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QDesktopServices>

#include "config_dialog.h"

ConfigDialog::ConfigDialog(Database *database, bool create)
{
    this->database = database;
    this->create = create;
}

QLineEdit *ConfigDialog::lineEdit(const char *text, QString defText, const char *label) {
    QLineEdit *le = new QLineEdit;

    le->setPlaceholderText(tr(text));
    le->setText(defText);

    metaLayout->addRow(tr(label), le);
    return le;
};

QComboBox *ConfigDialog::comboBox(QList<std::string> vec, const char *label, int val) {
    QComboBox *box = new QComboBox;

    QStringList list;
    for (int i = 0; i < vec.size(); ++i) {
        list.push_back(QString::fromStdString(vec[i]));
    }

    box->addItems(list);
    box->setCurrentIndex(create ? 0 : val);
    encLayout->addRow(tr(label), box);
    return box;
};

void ConfigDialog::calcMem() {
    if (hashBox->currentIndex() == 2) {
        int val = hashIterBox->value() * 128 * 32768;
        double dispVal = std::round(val / 10000) / 100;
        memBox->setValue(dispVal);
    }
}

void ConfigDialog::updateBoxes(int index) {
    bool hashVis = (index != 3);
    hashIterBox->setVisible(hashVis);
    encLayout->labelForField(hashIterBox)->setVisible(hashVis);

    bool memVis = (index == 0 || index == 2);
    memBox->setVisible(memVis);
    encLayout->labelForField(memBox)->setVisible(memVis);
    calcMem();

    bool memRO = (index == 2);
    memBox->setReadOnly(memRO);
}

void ConfigDialog::init() {
    _diC = QColor(54, 54, 56);

    diPalette.setColor(QPalette::Window, _diC);
    diPalette.setColor(QPalette::Base, _diC);

    full = new QGridLayout(this);

    bar = new QMenuBar;
    help = bar->addMenu(tr("Help"));

    bold.setBold(true);
    italic.setItalic(true);

    _window = QColor(62, 62, 66);
    _border = QColor(86, 86, 90);

    sectPalette.setColor(QPalette::Light, _border);
    sectPalette.setColor(QPalette::Dark, _border);
    sectPalette.setColor(QPalette::Window, _window);

    metaTitle = new QLabel(tr("General Info"));
    metaWidget = new QFrame;
    metaLayout = new QFormLayout(metaWidget);
    metaDesc = new QLabel(tr("Optional name and description for your database."));

    nameEdit = lineEdit("Name", database->name, "  Database Name:");
    descEdit = lineEdit("Description", database->desc, "  Description:");

    encTitle = new QLabel(tr("Security Settings"));
    encWidget = new QFrame;
    encLayout = new QFormLayout(encWidget);
    encDesc = new QLabel(tr("Adjust encryption, hashing, and checksum functions, as well as some additional parameters."));
    encLayout->addRow(encDesc);

    checksumBox = comboBox(checksumMatch, "  Checksum Function:", database->checksum);
    hashBox = comboBox(hashMatch, "  Password Hashing Function:  ", database->hash);
    encryptionBox = comboBox(encryptionMatch, "  Data Encryption Function:", database->encryption);

    iterVal = database->hashIters;

    hashIterBox = new QSpinBox;
    memBox = new QSpinBox;

    passTitle = new QLabel(tr("Authentication Settings"));
    passWidget = new QFrame;
    passLayout = new QFormLayout(passWidget);
    passDesc = new QLabel(tr("Password and optional key file."));

    pass = new QLineEdit;

    keyEdit = new QLineEdit;
    newKf = new QPushButton(tr("New"));
    getKf = new QPushButton(tr("Open"));

    keyBox = new QDialogButtonBox;
    keyWarn = new QLabel(tr("  Note: Do not select a file that may, at any point, change. This will make your database entirely inaccessible! It's recommended to create a new key file rather than use an existing one, unless it is a *.pkpp file."));

    miscTitle = new QLabel(tr("Miscellaneous Settings"));
    miscWidget = new QFrame;
    miscLayout = new QFormLayout(miscWidget);
    miscDesc = new QLabel(tr("Other, unrelated settings."));

    compressBox = new QCheckBox;

    clearBox = new QSpinBox;
    buttonBox = new QDialogButtonBox(this);

    glw = new QWidget;
    gl = new QGridLayout(glw);
}

void ConfigDialog::setup() {
    if (!create) {
        database->parse();
    }

    setWindowTitle("Database Options");

    setPalette(diPalette);

    help->addAction(tr("Choosing Options"), []{
        QDesktopServices::openUrl(QUrl(QString::fromStdString(github) + "blob/main/Choosing%20Options.md"));
    });
    full->setMenuBar(bar);

    metaTitle->setFont(bold);
    full->addWidget(metaTitle);

    metaWidget->setFrameStyle(QFrame::Panel | QFrame::Raised);
    metaWidget->setLineWidth(2);

    metaWidget->setAutoFillBackground(true);
    metaWidget->setPalette(sectPalette);
    metaLayout->addRow(metaDesc);

    full->addWidget(metaWidget);

    encTitle->setFont(bold);
    full->addWidget(encTitle);

    encWidget->setFrameStyle(QFrame::Panel | QFrame::Raised);
    encWidget->setLineWidth(2);

    encWidget->setAutoFillBackground(true);
    encWidget->setPalette(sectPalette);

    if (debug) {
        qDebug() << "Database params:" << database->checksum << database->hash << database->encryption << database->memoryUsage;
    }

    hashIterBox->setRange(8, 40);
    hashIterBox->setSingleStep(1);
    hashIterBox->setValue(iterVal);
    hashIterBox->setToolTip(tr("How many times to hash the password."));

    encLayout->addRow(tr("  Password Hashing Iterations:"), hashIterBox);

    memBox->setRange(1, 65535);
    memBox->setSingleStep(1);
    memBox->setSuffix(" MB");
    memBox->setValue(database->memoryUsage);
    memBox->setToolTip(tr("How much memory, in MB, to use for password hashing."));

    encLayout->addRow(tr("  Memory Usage:"), memBox);

    QObject::connect(hashIterBox, &QSpinBox::valueChanged, this, [this] { calcMem(); });

    calcMem();

    QObject::connect(hashBox, &QComboBox::currentIndexChanged, [this](int index) { updateBoxes(index); });

    updateBoxes(database->hash);

    full->addWidget(encWidget);

    passTitle->setFont(bold);
    full->addWidget(passTitle);

    passWidget->setFrameStyle(QFrame::Panel | QFrame::Raised);
    passWidget->setLineWidth(2);

    passWidget->setAutoFillBackground(true);
    passWidget->setPalette(sectPalette);
    passLayout->addRow(passDesc);

    pass->setPlaceholderText(tr("Password"));
    pass->setEchoMode(QLineEdit::Password);
    passLayout->addRow(tr("  Password:"), pass);

    keyEdit->setText(database->keyFilePath);

    QObject::connect(newKf, &QPushButton::clicked, [this] {
        keyEdit->setText(newKeyFile());
    });

    QObject::connect(getKf, &QPushButton::clicked, [this] {
        keyEdit->setText(getKeyFile());
    });

    keyBox->addButton(newKf, QDialogButtonBox::ActionRole);
    keyBox->addButton(getKf, QDialogButtonBox::ActionRole);

    passLayout->addRow(tr("  Key File:"), keyEdit);
    passLayout->addRow(keyBox);
    passLayout->setAlignment(keyBox, Qt::AlignLeft);

    keyWarn->setFont(italic);

    passLayout->addRow(keyWarn);

    full->addWidget(passWidget);

    miscTitle->setFont(bold);
    full->addWidget(miscTitle);

    miscWidget->setFrameStyle(QFrame::Panel | QFrame::Raised);
    miscWidget->setLineWidth(2);

    miscWidget->setAutoFillBackground(true);
    miscWidget->setPalette(sectPalette);
    miscLayout->addRow(miscDesc);

    compressBox->setChecked(database->compress);

    miscLayout->addRow(tr("  Compression (recommended):"), compressBox);

    clearBox->setRange(1, 255);
    clearBox->setSingleStep(1);
    clearBox->setSuffix(" seconds");
    clearBox->setValue(database->clearSecs);
    clearBox->setToolTip(tr("How long to wait before clearing the clipboard when copying passwords"));

    miscLayout->addRow(tr("  Clipboard Clear Delay:"), clearBox);

    full->addWidget(miscWidget);

    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    QObject::connect(buttonBox, &QDialogButtonBox::accepted, [this]() mutable {
        QString pw = pass->text();
        paramsChanged = create || (encryptionBox->currentIndex() != database->encryption || hashBox->currentIndex() != database->hash || checksumBox->currentIndex() != database->checksum || memBox->value() != database->memoryUsage || hashIterBox->value() != database->hashIters);

        if ((create || paramsChanged) && pw.isEmpty()) {
            displayErr("Password must be provided.");
        } else {
            if (create && pw.length() < 8) {
                std::cerr << "Warning: your password is less than 8 characters long. Consider making it longer." << std::endl;
            }
            accept();
        }
    });

    QObject::connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    glw->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    gl->addWidget(buttonBox);

    full->addWidget(glw);
    gl->setAlignment(Qt::AlignBottom);
    full->setMenuBar(bar);
}

int ConfigDialog::show() {
    pass->setFocus();

    int ret = exec();

    if (ret == QDialog::Rejected) {
        return false;
    }

    if (create) {
        Entry *entry = new Entry({}, database);
        entry->setDefaults();
        for (Field *f : entry->getFields()) {
            f->setData("EXAMPLE");
        }
    }

    QString pw = pass->text();

    database->keyFilePath = keyEdit->text();
    database->keyFile = !database->keyFilePath.isEmpty();

    if (database->keyFile && !QFile::exists(database->keyFilePath)) {
        genKey(database->keyFilePath);
    }

    database->checksum = checksumBox->currentIndex();
    database->hash = hashBox->currentIndex();
    database->hashIters = hashIterBox->value();
    database->encryption = encryptionBox->currentIndex();
    database->name = nameEdit->text();
    database->desc = descEdit->text();

    if (database->name.isEmpty()) {
        database->name = "None";
    }

    if (database->desc.isEmpty()) {
        database->desc = "None";
    }

    database->memoryUsage = memBox->value();
    database->compress = compressBox->isChecked();
    database->clearSecs = clearBox->value();

    if (paramsChanged) {
        setCursor(QCursor(Qt::WaitCursor));

        if (create) {
            std::unique_ptr<Botan::Cipher_Mode> enc = Botan::Cipher_Mode::create(encryptionMatch.at(database->encryption), Botan::ENCRYPTION);

            Botan::AutoSeeded_RNG rng;
            database->ivLen = enc->default_nonce_length();
            database->iv = rng.random_vec(database->ivLen);

            database->passw = database->getPw(pw);
        }

        database->passw = database->getPw(pw);
    }
    database->save();

    if (!create && !pw.isEmpty()) {
        setCursor(QCursor(Qt::WaitCursor));
        database->passw = database->getPw(pw);
    }

    return true;
}
