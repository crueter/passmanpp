#include "password_visible_action.hpp"
#include "../util/extra.hpp"

// Create a password visibility toggle action and attach it to a QLineEdit
QAction *passwordVisibleAction(QLineEdit *lineEdit, const bool on) {
    QAction *view = new QAction(getIcon(tr("password-show-o") + tr(on ? "n" : "ff")), QObject::tr("Toggle Password"));
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

        view->setIcon(getIcon(tr(iconName)));
        lineEdit->setEchoMode(echoMode);
    });

    lineEdit->addAction(view, QLineEdit::TrailingPosition);
    return view;
}
