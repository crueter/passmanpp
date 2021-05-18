#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QMenuBar>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QFileDialog>

#include <botan/auto_rng.h>

#include <passman/pdpp_entry.hpp>

#include "config_widget.hpp"
#include "../database.hpp"
#include "../passman_constants.hpp"

QComboBox *ConfigWidget::comboBox(QList<std::string> vec, const char *label, const int val) {
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

void ConfigWidget::updateBoxes(const int index) {
    const bool hashVis = (index != 3);
    hashIterBox->setVisible(hashVis);
    encLayout->labelForField(hashIterBox)->setVisible(hashVis);
    benchmark->setVisible(hashVis);
    benchmarkBox->setVisible(hashVis);

    const bool memRO = (index == 2);
    memBox->setReadOnly(memRO);

    const bool memVis = (index == 0 || memRO);
    memBox->setVisible(memVis);
    encLayout->labelForField(memBox)->setVisible(memVis);
    calcMem();
}

ConfigWidget::ConfigWidget(Database *t_database, const bool t_create)
    : create(t_create)

    , metaTitle(new QLabel(tr("General Info")))
    , metaWidget(new QFrame)
    , metaLayout(new QFormLayout(metaWidget))
    , metaDesc(new QLabel(tr("Optional name and description for your database.")))

    , encTitle(new QLabel(tr("Security Settings")))
    , encWidget(new QFrame)
    , encLayout(new QFormLayout(encWidget))
    , encDesc(new QLabel(tr("Adjust encryption, hashing, and HMAC functions, as well as some additional parameters.")))

    , benchmark(new QPushButton(tr("\tBenchmark Unlock Time")))
    , benchmarkBox(new QDoubleSpinBox)

    , hashIterBox(new QSpinBox)
    , memBox(new QSpinBox)

    , passTitle(new QLabel(tr("Authentication Settings")))
    , passWidget(new QFrame)
    , passLayout(new QFormLayout(passWidget))
    , passDesc(new QLabel(tr("Password and optional key file.")))

    , pass(new QLineEdit)

    , keyEdit(new QLineEdit)
    , newKf(new QPushButton(tr("New")))
    , getKf(new QPushButton(tr("Open")))

    , keyBox(new QDialogButtonBox)
    , keyWarn(new QLabel(tr("\tNote: Do not select a file that may, at any point, change. This will make your database entirely inaccessible! It's recommended to create a new key file rather than use an existing one, unless it is a *.pkpp file.")))

    , miscTitle(new QLabel(tr("Miscellaneous Settings")))
    , miscWidget(new QFrame)
    , miscLayout(new QFormLayout(miscWidget))
    , miscDesc(new QLabel(tr("Other, unrelated settings.")))

    , compressBox(new QCheckBox)

    , clearBox(new QSpinBox)

    , glw(new QWidget)
    , gl(new QGridLayout(glw))

    , tabs(new QTabWidget(this))
{
    database = t_database;
    window = t_database->window;

    buttonBox = new QDialogButtonBox(this);
    layout = new QGridLayout(this);

    menubar = new QMenuBar;
    title = tr("Database Config");

    help = menubar->addMenu(tr("Help"));

    iterVal = database->hashIters;

    this->setStyleSheet(tr("background-color: rgb(54, 54, 56)"));

    bold.setBold(true);
    italic.setItalic(true);

    widgetStyle = tr("border: 2px solid rgb(86, 86, 90); background: rgb(62, 62, 66); margin: 2px");
}

bool ConfigWidget::setup() {
    if (!create) {
        try {
            database->parse();
        } catch (std::exception& e) {
            displayErr(passman::tr(e.what()));
        }
    }

    help->addAction(tr("Choosing Options"), this, []{
        QDesktopServices::openUrl(QUrl(QString::fromStdString(Constants::passmanGithub) + "blob/main/Choosing%20Options.md"));
    });

    metaTitle->setFont(bold);
    metaLayout->addRow(metaTitle);
    metaLayout->addRow(metaDesc);

    nameEdit = lineEdit("Name", database->name.asQStr(), "\tDatabase Name:");
    descEdit = lineEdit("Description", database->desc.asQStr(), "\tDescription:");

    metaWidget->setFrameStyle(QFrame::Panel | QFrame::Raised);
    metaWidget->setLineWidth(2);

    metaWidget->setAutoFillBackground(true);
    metaWidget->setStyleSheet(widgetStyle);

    tabs->addTab(metaWidget, tr("&General"));

    encTitle->setFont(bold);
    encLayout->addRow(encTitle);
    encLayout->addRow(encDesc);

    hmacBox = comboBox(passman::Constants::hmacMatch, "\tHMAC Function:", database->hmac);
    hashBox = comboBox(passman::Constants::hashMatch, "\tPassword Hashing Function:  ", database->hash);
    encryptionBox = comboBox(passman::Constants::encryptionMatch, "\tData Encryption Function:", database->encryption);

    encWidget->setFrameStyle(QFrame::Panel | QFrame::Raised);
    encWidget->setLineWidth(2);

    encWidget->setAutoFillBackground(true);
    encWidget->setStyleSheet(widgetStyle);

    if (qApp->property("debug").toBool()) {
        qDebug() << database->makeKdf()->toString();
    }

    benchmarkBox->setRange(0.1, 65535);
    benchmarkBox->setValue(1.);
    benchmarkBox->setSingleStep(0.1);
    benchmarkBox->setSuffix("s");
    benchmarkBox->setToolTip(tr("Benchmark how many hashing iterations it would take to unlock the database in this many seconds."));

    QObject::connect(benchmark, &QPushButton::clicked, this, [this] {
        passman::KDF *kdf = database->makeKdf(static_cast<uint8_t>(hmacBox->currentIndex()), static_cast<uint8_t>(hashBox->currentIndex()), static_cast<uint8_t>(encryptionBox->currentIndex()), {}, {}, 4, static_cast<uint16_t>(memBox->value()));

        setCursor(Qt::WaitCursor);
        int iters = kdf->benchmark(static_cast<int>(benchmarkBox->value() * 1000));
        hashIterBox->setValue(iters);
        unsetCursor();
    });

    hashIterBox->setRange(8, 255);
    hashIterBox->setSingleStep(1);
    hashIterBox->setValue(iterVal);
    hashIterBox->setToolTip(tr("How many times to hash the password."));

    encLayout->addRow(tr("\tPassword Hashing Iterations:"), hashIterBox);
    encLayout->addRow(benchmark, benchmarkBox);

    memBox->setRange(1, 65535);
    memBox->setSingleStep(1);
    memBox->setSuffix(" MB");
    memBox->setValue(database->memoryUsage);
    memBox->setToolTip(tr("How much memory, in MB, to use for password hashing."));

    encLayout->addRow(tr("\tMemory Usage:"), memBox);

    QObject::connect(hashIterBox, &QSpinBox::valueChanged, this, [this] { calcMem(); });

    calcMem();

    QObject::connect(hashBox, &QComboBox::currentIndexChanged, [this](int index) { updateBoxes(index); });

    updateBoxes(database->hash);

    tabs->addTab(encWidget, tr("&Security"));

    passTitle->setFont(bold);
    passLayout->addRow(passTitle);
    passLayout->addRow(passDesc);

    passWidget->setFrameStyle(QFrame::Panel | QFrame::Raised);
    passWidget->setLineWidth(2);

    passWidget->setAutoFillBackground(true);
    passWidget->setStyleSheet(widgetStyle);

    pass->setPlaceholderText(tr("Password"));
    pass->setEchoMode(QLineEdit::Password);
    passLayout->addRow(tr("\tPassword:"), pass);

    keyEdit->setText(database->keyFilePath.asQStr());

    QObject::connect(newKf, &QPushButton::clicked, [this] {
        keyEdit->setText(QFileDialog::getSaveFileName(nullptr, tr("New Key File"), "", passman::Constants::keyExt));
    });

    QObject::connect(getKf, &QPushButton::clicked, [this] {
        keyEdit->setText(QFileDialog::getOpenFileName(nullptr, tr("Open Key File"), "", passman::Constants::keyExt));
    });

    keyBox->addButton(newKf, QDialogButtonBox::ActionRole);
    keyBox->addButton(getKf, QDialogButtonBox::ActionRole);

    passLayout->addRow(tr("\tKey File:"), keyEdit);
    passLayout->addRow(keyBox);
    passLayout->setAlignment(keyBox, Qt::AlignLeft);

    keyWarn->setFont(italic);
    passLayout->addRow(keyWarn);

    tabs->addTab(passWidget, tr("&Authentication"));

    miscTitle->setFont(bold);
    miscLayout->addRow(miscTitle);
    miscLayout->addRow(miscDesc);

    miscWidget->setFrameStyle(QFrame::Panel | QFrame::Raised);
    miscWidget->setLineWidth(2);

    miscWidget->setAutoFillBackground(true);
    miscWidget->setStyleSheet(widgetStyle);

    compressBox->setChecked(database->compress);

    miscLayout->addRow(tr("\tCompression (recommended):"), compressBox);

    clearBox->setRange(1, 255);
    clearBox->setSingleStep(1);
    clearBox->setSuffix(" seconds");
    clearBox->setValue(database->clearSecs);
    clearBox->setToolTip(tr("How long to wait before clearing the clipboard when copying passwords"));

    miscLayout->addRow(tr("\tClipboard Clear Delay:"), clearBox);

    tabs->addTab(miscWidget, tr("&Miscellaneous"));

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

            if (create) {
                passman::PDPPEntry *entry = new passman::PDPPEntry({}, database);

                for (passman::Field *f : entry->fields()) {
                    f->setData("EXAMPLE");
                }
            }

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
                    auto enc = database->makeKdf()->makeEncryptor();

                    Botan::AutoSeeded_RNG rng;

                    database->ivLen = enc->default_nonce_length();
                    database->iv = rng.random_vec(database->ivLen);
                }

                database->passw = database->makeKdf()->transform(pw);
            }
            database->save();

            if (!create && !pw.isEmpty()) {
                setCursor(QCursor(Qt::WaitCursor));
                database->passw = database->makeKdf()->transform(pw);
            }

            if (create) {
                database->edit();
            } else {
                window->back();
            }
        }
    });

    QObject::connect(buttonBox, &QDialogButtonBox::rejected, window, &MainWindow::back);

    glw->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    gl->addWidget(tabs);
    gl->addWidget(buttonBox);

    layout->addWidget(glw);
    gl->setAlignment(Qt::AlignBottom);
    return true;
}

void ConfigWidget::show() {
    pass->setFocus();

    window->setWidget(this);
}
