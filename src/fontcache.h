// -*- coding: utf-8 -*-

#pragma once

#include <QStringList>
#include <QFile>
#include <QHash>
#include <QFont>
#include <QTextStream>
#include <QFontDatabase>
#include <QtDebug>

class FontCache
{
public:
  FontCache();

  ~FontCache();

  static FontCache* getInstance();

  static QString getDefaultFontName();

  QFont* getFont(const QString& name) const;

  QFont* addFont(const QString& path, const QString& name, int pixelSize = 12);

private:
  QHash<QString, QFont*> cache;

  /* Here will be the instance stored. */
  static FontCache* instance;

  static QString defaultFontName;
};

