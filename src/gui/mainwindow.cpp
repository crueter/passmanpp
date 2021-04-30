#include <QMenuBar>
#include <QLayout>
#include <QPushButton>
#include <QLabel>
#include <QSettings>
#include <iostream>
#include <passman/extra.hpp>

#include "mainwindow.hpp"
#include "database_main_widget.hpp"

MainWindow::MainWindow() {
    this->setCentralWidget(centralWidget = new QWidget);
    stackedLayout = new QStackedLayout(centralWidget);
    stackedLayout->setContentsMargins(0, 0, 0, 0);

    this->setMenuWidget(menuWidget = new QWidget);
    menuLayout = new QStackedLayout(menuWidget);
    menuLayout->setContentsMargins(0, 0, 0, 0);
}

void MainWindow::setToolBar(QToolBar *t_toolbar) {
    this->removeToolBar(this->findChild<QToolBar *>());
    if (t_toolbar != nullptr) {
        this->addToolBar(t_toolbar);
        t_toolbar->show();
    }
}

void MainWindow::setWidget(BaseWidget *t_widget, const bool t_clearQueue) {
    if (t_widget == nullptr) {
        std::cerr << "Something has gone terribly wrong. Details:\n";
        std::cerr << "Expected first parameter of type BaseWidget * on MainWindow::setWidget, got nullptr instead" << std::endl;
        std::exit(1);
    }

    if (t_clearQueue) {
        queue.clear();
        for (const int i : passman::range(0, stackedLayout->count())) {
            stackedLayout->removeWidget(stackedLayout->widget(i));
            menuLayout->removeWidget(menuLayout->widget(i));
        }
    }

    queue += t_widget;

    stackedLayout->addWidget(t_widget);
    stackedLayout->setCurrentWidget(t_widget);

    menuLayout->addWidget(t_widget->menubar);
    menuLayout->setCurrentWidget(t_widget->menubar);

    this->setWindowTitle(t_widget->title);

    this->setToolBar(t_widget->toolbar);

    repaint();
}

void MainWindow::back() {
    qsizetype index = queue.length() - 2;
    if (index < 0) {
        return;
    }
    auto widgetToSet = queue[index];

    stackedLayout->removeWidget(queue.last());
    stackedLayout->setCurrentWidget(widgetToSet);

    menuLayout->removeWidget(queue.last());
    menuLayout->setCurrentWidget(widgetToSet->menubar);

    this->setWindowTitle(widgetToSet->title);

    this->setToolBar(widgetToSet->toolbar);

    repaint();
    queue.removeLast();
}
