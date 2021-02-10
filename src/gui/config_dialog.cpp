#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QMenuBar>
#include <QDesktopServices>
#include <QDialogButtonBox>

#include "config_dialog.hpp"
#include "../entry.hpp"
#include "../database.hpp"

QComboBox *ConfigDialog::comboBox(QList<std::string> vec, const char *label, const int val) {
    QComboBox *box = new QComboBox;

    QStringList list;
    for (const std::string &s : vec) {
        list.push_back(QString::fromStdString(s));
    }

    box->addItems(list);
    box->setCurrentIndex(create ? 0 : val);
    encLayout->addRow(tr(label), box);
    return box;
}

void ConfigDialog::updateBoxes(const int index) {
    const bool hashVis = (index != 3);
    hashIterBox->setVisible(hashVis);
    encLayout->labelForField(hashIterBox)->setVisible(hashVis);

    const bool memVis = (index == 0 || index == 2);
    memBox->setVisible(memVis);
    encLayout->labelForField(memBox)->setVisible(memVis);
    calcMem();

    const bool memRO = (index == 2);
    memBox->setReadOnly(memRO);
}

ConfigDialog::ConfigDialog(Database *t_database, const bool t_create)
    : database(t_database)
    , create(t_create)
{
    diC = QColor(54, 54, 56);

    diPalette.setColor(QPalette::Window, diC);
    diPalette.setColor(QPalette::Base, diC);

    full = new QGridLayout(this);

    bar = new QMenuBar;
    help = bar->addMenu(tr("Help"));

    bold.setBold(true);
    italic.setItalic(true);

    const QColor window{62, 62, 66};
    const QColor border{86, 86, 90};

    sectPalette.setColor(QPalette::Light, border);
    sectPalette.setColor(QPalette::Dark, border);
    sectPalette.setColor(QPalette::Window, window);

    metaTitle = new QLabel(tr("General Info"));
    metaWidget = new QFrame;
    metaLayout = new QFormLayout(metaWidget);
    metaDesc = new QLabel(tr("Optional name and description for your database."));

    nameEdit = lineEdit("Name", database->name.asQStr(), "  Database Name:");
    descEdit = lineEdit("Description", database->desc.asQStr(), "  Description:");

    encTitle = new QLabel(tr("Security Settings"));
    encWidget = new QFrame;
    encLayout = new QFormLayout(encWidget);
    encDesc = new QLabel(tr("Adjust encryption, hashing, and HMAC functions, as well as some additional parameters."));
    encLayout->addRow(encDesc);

    hmacBox = comboBox(Constants::hmacMatch, "  HMAC Function:", database->hmac);
    hashBox = comboBox(Constants::hashMatch, "  Password Hashing Function:  ", database->hash);
    encryptionBox = comboBox(Constants::encryptionMatch, "  Data Encryption Function:", database->encryption);

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
        QDesktopServices::openUrl(QUrl(QString::fromStdString(Constants::github) + "blob/main/Choosing%20Options.md"));
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

    if (Constants::debug) {
        qDebug() << "Database params:" << database->hmac << database->hash << database->encryption << database->memoryUsage;
    }

    hashIterBox->setRange(8, 64);
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

    keyEdit->setText(database->keyFilePath.asQStr());

    QObject::connect(newKf, &QPushButton::clicked, [this] {
        keyEdit->setText(QFileDialog::getSaveFileName(nullptr, tr("New Key File"), "", Constants::keyExt));
    });

    QObject::connect(getKf, &QPushButton::clicked, [this] {
        keyEdit->setText(QFileDialog::getOpenFileName(nullptr, tr("Open Key File"), "", Constants::keyExt));
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
        paramsChanged = create || (encryptionBox->currentIndex() != database->encryption || hashBox->currentIndex() != database->hash || hmacBox->currentIndex() != database->hmac || memBox->value() != database->memoryUsage || hashIterBox->value() != database->hashIters);

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

        for (Field *f : entry->fields()) {
            f->setData("EXAMPLE");
        }
    }

    QString pw = pass->text();

    database->keyFilePath = keyEdit->text();
    database->keyFile = !database->keyFilePath.empty();

    if (database->keyFile && !QFile::exists(database->keyFilePath.asQStr())) {
        QFile f(database->keyFilePath.asQStr());
        f.open(QIODevice::ReadWrite);
        QDataStream q(&f);

        Botan::AutoSeeded_RNG rng;
        for (const uint8_t v : rng.random_vec(128)) {
            q << v;
        }

        f.close();
    }

    database->hmac = static_cast<uint8_t>(hmacBox->currentIndex());
    database->hash = static_cast<uint8_t>(hashBox->currentIndex());
    database->hashIters = static_cast<uint8_t>(hashIterBox->value());
    database->encryption = static_cast<uint8_t>(encryptionBox->currentIndex());
    database->name = nameEdit->text();
    database->desc = descEdit->text();

    if (database->name.empty()) {
        database->name = "None";
    }

    if (database->desc.empty()) {
        database->desc = "None";
    }

    database->memoryUsage = static_cast<uint16_t>(memBox->value());
    database->compress = compressBox->isChecked();
    database->clearSecs = static_cast<uint8_t>(clearBox->value());

    if (paramsChanged) {
        setCursor(QCursor(Qt::WaitCursor));

        if (create) {
            auto enc = database->makeEncryptor();

            Botan::AutoSeeded_RNG rng;
            database->ivLen = enc->default_nonce_length();
            database->iv = rng.random_vec(database->ivLen);
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
