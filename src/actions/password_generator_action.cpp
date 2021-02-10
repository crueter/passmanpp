#include "password_generator_action.hpp"
#include "../gui/random_password_dialog.hpp"

// Create a password generator dialog action and attach it to a QLineEdit
QAction *passwordGeneratorAction(QLineEdit *lineEdit) {
    QAction *random = new QAction(QIcon::fromTheme(QObject::tr("roll")), QObject::tr("Password Generator (Ctrl+R)"));
    random->setShortcut(QKeySequence(QObject::tr("Ctrl+R")));

    QObject::connect(random, &QAction::triggered, [lineEdit] {
        RandomPasswordDialog *di = new RandomPasswordDialog;
        di->setup();
        lineEdit->setText(di->show());
    });

    lineEdit->addAction(random, QLineEdit::TrailingPosition);
    return random;
}
