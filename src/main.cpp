// -*- coding: utf-8 -*-

#include <QApplication>
#include <QFile>
#include "container_qt5.h"
#include "context.h"
#include "mainwindow.h"
#include <QDebug>

#include "fontcache.h"

int main (int argc, char **argv) {
  QApplication app(argc, argv);

  FontCache::getInstance()->addFont(":/res/font/Cousine-Regular.ttf", R"raw("Cousine Regular")raw", container_qt5::getDefaultFontSize());
  FontCache::getInstance()->addFont(":/res/font/arialuni.ttf", R"raw("Arial Unicode MS")raw", container_qt5::getDefaultFontSize());
  FontCache::getInstance()->addFont(":/res/font/DroidSans.ttf", R"raw("Droid Sans")raw", container_qt5::getDefaultFontSize());
  FontCache::getInstance()->addFont(":/res/font/fa-regular-400.ttf", R"raw("Font Awesome 5 Free")raw", container_qt5::getDefaultFontSize());

  /*QFontDatabase qfd;
  for(int i=0; i < qfd.families().size(); i++)
  {
    qDebug() << "qfd.families().at("<<i<<") = " << qfd.families().at(i);
  }

  qDebug() << "QFontDatabase::Cyrillic " << qfd.families(QFontDatabase::Cyrillic);*/

  // Show window
  MainWindow w;
  w.show();

  return app.exec();
}
