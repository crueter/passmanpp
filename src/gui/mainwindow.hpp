#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QMenuBar>
#include <QToolBar>
#include <QStackedLayout>

#include "base_widget.hpp"

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    QList<BaseWidget *> queue;
    QWidget *centralWidget;
    QWidget *menuWidget;
    QStackedLayout *menuLayout;
    QStackedLayout *stackedLayout;
public:
    MainWindow();
    virtual ~MainWindow() = default;
    void setWidget(BaseWidget *t_widget, const bool t_clearQueue = false);
    void setToolBar(QToolBar *t_toolbar);
public slots:
    void back();
};

#endif // MAINWINDOW_H
