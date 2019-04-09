// -*- coding: utf-8 -*-

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "container_qt5.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

private:
  Ui::MainWindow *ui;
  litehtmlWidget* m_litehtmlWidget;
  litehtml::context ctxt;
};

#endif // MAINWINDOW_H
