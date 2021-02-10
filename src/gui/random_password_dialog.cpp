#include <QAction>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QSpinBox>
#include <QPushButton>
#include <QSlider>

#include <sodium/randombytes_sysrandom.h>

#include "random_password_dialog.hpp"
#include "../actions/password_visible_action.hpp"
#include "../util/extra.hpp"

RandomPasswordDialog::Options RandomPasswordDialog::getOptions() {
    Options opt;

    if (lowersBox->isChecked()) {
        opt |= Lowers;
    }

    if (uppersBox->isChecked()) {
        opt |= Uppers;
    }

    if (numbersBox->isChecked()) {
        opt |= Numbers;
    }

    if (bracesBox->isChecked()) {
        opt |= Braces;
    }

    if (punctsBox->isChecked()) {
        opt |= Punctuation;
    }

    if (dashesBox->isChecked()) {
        opt |= Dashes;
    }

    if (mathBox->isChecked()) {
        opt |= Math;
    }

    if (logogramsBox->isChecked()) {
        opt |= Logograms;
    }

    if (easciiBox->isChecked()) {
        opt |= EASCII;
    }

    if (opt == 0) {
        opt = Default;
    }

    return opt;
}

Group RandomPasswordDialog::getGroup() {
    Group groups;

    options = getOptions();

    if (options & Lowers) {
        groups.append(range<QChar>(97, 25));
    }
    if (options & Uppers) {
        groups.append(range<QChar>(65, 25));
    }
    if (options & Numbers) {
        groups.append(range<QChar>(48, 10));
    }
    if (options & Braces) {
        groups.append(fromString("()[]{}"));
    }
    if (options & Punctuation) {
        groups.append(fromString(".,:;?"));
    }
    if (options & Dashes) {
        groups.append(fromString("-/\\_|"));
    }
    if (options & Math) {
        groups.append(fromString("!*+<=>"));
    }
    if (options & Logograms) {
        groups.append(fromString("#$%&@^`~"));
    }
    if (options & EASCII) {
        // U+0080-U+009F are control characters, and
        // U+00A0 is a non-breaking space
        groups.append(range<QChar>(161, 12));

        // U+00AD is a soft hyphen
        groups.append(range<QChar>(174, 82));
    }

    for (const QChar &c : includes) {
        if (!groups.contains(c)) {
            groups.emplaceBack(c);
        }
    }

    for (const QChar &c : excludes) {
        groups.removeAll(c);
    }

    groups.squeeze();

    return groups;
}

const QString RandomPasswordDialog::generate() {
    Group chars = getGroup();

    QString pass;

    length = lengthBox->value();

    for (int i = 0; i < length; ++i) {
        pass.append(chars[randombytes_uniform(static_cast<uint32_t>(chars.size()))]);
    }

    display->setText(pass);

    return pass;
}

RandomPasswordDialog::RandomPasswordDialog() {
    layout = new QGridLayout(this);

    display = new QLineEdit;

    visible = passwordVisibleAction(display, true);

    regen = new QPushButton(QIcon::fromTheme(tr("view-refresh")), "");

    lengthLabel = new QLabel(tr("Length:"));
    lengthSlider = new QSlider(Qt::Horizontal);
    lengthBox = new QSpinBox;

    buttonLabel = new QLabel(tr("Character Types"));
    QFont font;
    font.setBold(true);

    buttonLabel->setFont(font);

    buttonWidget = new QFrame;
    buttonLayout = new QGridLayout(buttonWidget);

    const QColor chCl{33, 63, 33};
    checkedPalette.setColor(QPalette::Button, chCl);

    auto optButton = [this](const char *text, const char *tooltip, int row, int col, bool on = false) -> QPushButton * {
        QPushButton *button = new QPushButton(tr(text));
        button->setToolTip(tr(tooltip));

        button->setCheckable(true);
        button->setChecked(on);

        buttonLayout->addWidget(button, row, col);

        connect(button, &QPushButton::clicked, regen, &QPushButton::click);
        connect(button, &QPushButton::clicked, this, [button, this](bool checked) {
            if (checked) {
                button->setPalette(checkedPalette);
            } else {
                button->setPalette(QPalette());
            }
        });

        if (on) {
            button->setPalette(checkedPalette);
        }
        return button;
    };

    lowersBox = optButton("a-z", "Lowercase Letters", 0, 0, true);
    uppersBox = optButton("A-Z", "Capital Letters", 0, 1, true);
    numbersBox = optButton("0-9", "Numbers", 0, 2, true);
    bracesBox = optButton("()[]{}", "Braces", 1, 0);
    punctsBox = optButton(". , : ; ?", "Punctuation", 1, 1, true);
    dashesBox = optButton("- / \\ _ |", "Dashes and Slashes", 1, 2);
    mathBox = optButton("! * + < = >", "Math", 1, 3, true);
    logogramsBox = optButton("# $ % && @ ^ ` ~", "Logograms", 0, 3);
    easciiBox = optButton("Extended ASCII", "Extended ASCII", 0, 4);

    extraInclude = new QLineEdit;
    includeLabel = new QLabel(tr("Extra characters to include:"));

    extraExclude = new QLineEdit;
    excludeLabel = new QLabel(tr("Characters to exclude:"));

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
}

void RandomPasswordDialog::setup() {
    visible->setCheckable(true);
    visible->setChecked(true);

    connect(regen, &QPushButton::clicked, this, [this] {
        generate();
    });

    lengthSlider->setRange(1, 256);
    lengthSlider->setValue(32);

    connect(lengthSlider, &QSlider::valueChanged, this, [this](int value) {
        lengthBox->setValue(value);
    });

    lengthBox->setRange(1, 256);
    lengthBox->setValue(32);

    connect(lengthBox, &QSpinBox::valueChanged, this, [this](int value) {
        lengthSlider->setValue(value);
        generate();
    });

    connect(extraInclude, &QLineEdit::textChanged, this, [this] {
        QString text = extraInclude->text();
        includes = Group(text.begin(), text.end());

        generate();
    });

    connect(extraExclude, &QLineEdit::textChanged, this, [this] {
        QString text = extraExclude->text();
        excludes = Group(text.begin(), text.end());

        generate();
    });

    QObject::connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    QObject::connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    setContentsMargins(30, 30, 30, 0);

    const QColor window{62, 62, 66};
    const QColor border{86, 86, 90};

    QPalette buttonPalette;

    buttonPalette.setColor(QPalette::Light, border);
    buttonPalette.setColor(QPalette::Dark, border);
    buttonPalette.setColor(QPalette::Window, window);

    buttonWidget->setFrameStyle(QFrame::Panel | QFrame::Raised);
    buttonWidget->setLineWidth(2);
    buttonWidget->setAutoFillBackground(true);

    buttonWidget->setPalette(buttonPalette);

    layout->setSizeConstraint(QLayout::SetFixedSize);

    layout->addWidget(display, 0, 0, 1, 4);
    layout->addWidget(regen, 0, 4);

    layout->addWidget(lengthLabel, 1, 0);
    layout->addWidget(lengthSlider, 1, 1, 1, 3);
    layout->addWidget(lengthBox, 1, 4);
    layout->addWidget(buttonLabel, 2, 0);
    layout->addWidget(buttonWidget, 3, 0, 2, 5);

    layout->addWidget(includeLabel, 6, 2);
    layout->addWidget(extraInclude, 6, 3, 1, 2);

    layout->addWidget(excludeLabel, 7, 2);
    layout->addWidget(extraExclude, 7, 3, 1, 2);

    layout->addWidget(buttonBox, 8, 3);

    generate();
}

const QString RandomPasswordDialog::show() {
    if (exec() == QDialog::Accepted) {
        return display->text();
    }
    return "";
}
