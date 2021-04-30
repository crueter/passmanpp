#include "password_generator_action.hpp"
#include "../gui/password_generator_dialog.hpp"
#include "../util.hpp"

// Create a password generator dialog action and attach it to a QLineEdit (I/A)
QAction *passwordGeneratorAction(QLineEdit *lineEdit) {
    QAction *random = new QAction(getIcon(QObject::tr("roll")), QObject::tr("Password Generator (Ctrl+R)"));
    random->setShortcut(QKeySequence(QObject::tr("Ctrl+R")));

    QObject::connect(random, &QAction::triggered, [lineEdit] {
        PasswordGeneratorDialog *di = new PasswordGeneratorDialog;
        di->setup();
        const QString text = di->show();
        if (lineEdit != nullptr && !text.isEmpty()) {
            lineEdit->setText(text);
        }
    });

    if (lineEdit != nullptr) {
        lineEdit->addAction(random, QLineEdit::TrailingPosition);
    }
    return random;
}
