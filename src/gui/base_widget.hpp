#ifndef BASEWIDGET_H
#define BASEWIDGET_H
#include <QWidget>
#include <QMenuBar>

#include "../util/extra.hpp"

class Database;
class MainWindow;
class QDialogButtonBox;
class QGridLayout;
class QToolBar;

class BaseWidget : public QWidget
{
public:
    BaseWidget() = default;
    virtual ~BaseWidget() = default;
    BaseWidget(BaseWidget *w) {
        database = w->database;
        window = w->window;
        buttonBox = w->buttonBox;
        layout = w->layout;
        toolbar = w->toolbar;
        menubar = w->menubar;
        title = w->title;
    }

    virtual bool setup() { return true; };
    virtual void show() {};

    Database *database;
    MainWindow *window;

    QDialogButtonBox *buttonBox = nullptr;
    QGridLayout *layout = nullptr;

    QToolBar *toolbar = nullptr;
    QMenuBar *menubar = new QMenuBar;

    QString title{};
};

#endif // BASEWIDGET_H
