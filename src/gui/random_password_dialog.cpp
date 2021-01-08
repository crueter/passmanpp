#include <QAction>
#include <sodium/randombytes_sysrandom.h>

#include "random_password_dialog.h"

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
        for (int i = 97; i < (97 + 25); ++i) {
            groups.emplaceBack(i);
        }
    }
    if (options & Uppers) {
        for (int i = 65; i < (65 + 25); ++i) {
            groups.emplaceBack(i);
        }
    }
    if (options & Numbers) {
        for (int i = 48; i < (48 + 10); ++i) {
            groups.emplaceBack(i);
        }
    }
    if (options & Braces) {
        // ()[]{}
        groups.emplaceBack(40);
        groups.emplaceBack(41);
        groups.emplaceBack(91);
        groups.emplaceBack(93);
        groups.emplaceBack(123);
        groups.emplaceBack(125);
    }
    if (options & Punctuation) {
        // .,:;?
        groups.emplaceBack(44);
        groups.emplaceBack(46);
        groups.emplaceBack(58);
        groups.emplaceBack(59);
        groups.emplaceBack(63);
    }
    if (options & Dashes) {
        // -/\_|
        groups.emplaceBack(45);
        groups.emplaceBack(47);
        groups.emplaceBack(92);
        groups.emplaceBack(95);
    }
    if (options & Math) {
        // !*+<=>
        groups.emplaceBack(33);
        groups.emplaceBack(42);
        groups.emplaceBack(43);
        groups.emplaceBack(60);
        groups.emplaceBack(61);
        groups.emplaceBack(62);
    }
    if (options & Logograms) {
        // #$%&
        for (int i = 35; i <= 38; i++) {
            groups.emplaceBack(i);
        }
        // @^`~
        groups.emplaceBack(64);
        groups.emplaceBack(94);
        groups.emplaceBack(96);
        groups.emplaceBack(126);
    }
    if (options & EASCII) {
        // [U+0080, U+009F] are control characters
        // U+00A0 is a non-breaking space
        for (int i = 161; i <= 172; i++) {
            groups.emplaceBack(i);
        }
        // U+00AD is a soft hyphen
        for (int i = 174; i <= 255; i++) {
            groups.emplaceBack(i);
        }
    }

    return groups;
}

QString RandomPasswordDialog::generate() {
    Group chars = getGroup();

    QString pass;

    length = lengthBox->value();

    for (int i = 0; i < length; i++) {
        int pos = randombytes_uniform(chars.size());

        pass.append(chars[pos]);
    }

    return pass;
}

void RandomPasswordDialog::init() {
    layout = new QGridLayout(this);

    display = new QLineEdit;

    visible = new QAction(QIcon::fromTheme(tr("password-show-on")), tr("Toggle Password"));

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

    auto optButton = [this](const char *text, const char *tooltip, int row, int col) -> QPushButton * {
        QPushButton *button = new QPushButton(tr(text));
        button->setToolTip(tr(tooltip));

        button->setCheckable(true);
        button->setChecked(true);

        buttonLayout->addWidget(button, row, col);

        connect(button, &QPushButton::clicked, regen, &QPushButton::click);

        return button;
    };

    lowersBox       = optButton("a-z", "Lowercase Letters", 0, 0);
    uppersBox       = optButton("A-Z", "Capital Letters", 0, 1);
    numbersBox      = optButton("0-9", "Numbers", 0, 2);
    bracesBox       = optButton("()[]{}", "Braces", 1, 0);
    punctsBox       = optButton(". , : ; ?", "Punctuation", 1, 1);
    dashesBox       = optButton("- / \\ _ |", "Dashes and Slashes", 1, 2);
    mathBox         = optButton("! * + < = >", "Math", 1, 3);
    logogramsBox    = optButton("# $ % && @ ^ ` ~", "Logograms", 0, 3);
    easciiBox       = optButton("Extended ASCII", "Extended ASCII", 0, 4);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
}

void RandomPasswordDialog::setup() {
    visible->setCheckable(true);
    display->addAction(visible, QLineEdit::TrailingPosition);

    connect(visible, &QAction::triggered, this, [this](bool checked) {
        char iconName[18] = "password-show-o";
        QLineEdit::EchoMode echoMode = QLineEdit::Normal;

        if (checked) {
            std::strncat(iconName, "n", 1);
        } else {
            std::strncat(iconName, "ff", 2);
            echoMode = QLineEdit::Password;
        }

        visible->setIcon(QIcon::fromTheme(tr(iconName)));
        display->setEchoMode(echoMode);
    });

    connect(regen, &QPushButton::clicked, this, [this] {
        display->setText(generate());
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
        display->setText(generate());
    });

    QObject::connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    QObject::connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    setContentsMargins(30, 30, 30, 0);

    QColor _window = QColor(62, 62, 66);
    QColor _border = QColor(86, 86, 90);

    QPalette buttonPalette;

    buttonPalette.setColor(QPalette::Light, _border);
    buttonPalette.setColor(QPalette::Dark, _border);
    buttonPalette.setColor(QPalette::Window, _window);

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
    layout->addWidget(buttonBox, 6, 3);

    display->setText(generate());
}

QString RandomPasswordDialog::show() {
    int ret = exec();

    if (ret == QDialog::Accepted) {
        return display->text();
    } else {
        return "";
    }

}
