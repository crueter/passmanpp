#include "password_visible_action.h"

QAction *passwordVisibleAction(QLineEdit *lineEdit, bool on) {
    char iconName1[18] = "password-show-o";

    if (on) {
        std::strncat(iconName1, "n", 2);
    } else {
        std::strncat(iconName1, "ff", 3);
    }

    QAction *view = new QAction(QIcon::fromTheme(QObject::tr(iconName1)), QObject::tr("Toggle Password"));
    view->setCheckable(true);

    QObject::connect(view, &QAction::triggered, [view, lineEdit](bool checked) {
        char iconName[18] = "password-show-o";
        QLineEdit::EchoMode echoMode = QLineEdit::Normal;

        if (checked) {
            std::strncat(iconName, "n", 2);
        } else {
            std::strncat(iconName, "ff", 3);
            echoMode = QLineEdit::Password;
        }

        view->setIcon(QIcon::fromTheme(QObject::tr(iconName)));
        lineEdit->setEchoMode(echoMode);
    });

    lineEdit->addAction(view, QLineEdit::TrailingPosition);
    return view;
}
