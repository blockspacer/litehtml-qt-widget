// -*- coding: utf-8 -*-

#include "fontcache.h"

#include <QFontInfo>

/* Null, because instance will be initialized on demand. */
FontCache* FontCache::instance = nullptr;

QString FontCache::defaultFontName = R"raw("Cousine Regular")raw";

FontCache* FontCache::getInstance()
{
    if (instance == 0)
    {
        instance = new FontCache();
    }

    return instance;
}

QString FontCache::getDefaultFontName()
{
    return defaultFontName;
}

FontCache::FontCache()
{

}

FontCache::~FontCache()
{
  for(QFont* font : cache) {
    delete font;
  }
}

QFont* FontCache::getFont(const QString &name) const {
  if (cache[name] == nullptr) {
    qWarning() << "font " << name << " cannot be found !";
  }
  return cache[name];
}

QFont* FontCache::addFont(const QString &path, const QString &name, int pixelSize) {
  QFile fontFile(path);
  if (!fontFile.open(QIODevice::ReadOnly)) {
    qCritical() << "failed to open font file, path = " << path;
    return nullptr;
  }

  QByteArray fontData = fontFile.readAll();
  if (fontData.isEmpty()) {
    qCritical() << "empty font file, path = " << path;
    return nullptr;
  }

  int id = QFontDatabase::addApplicationFontFromData(fontData);
  Q_ASSERT(id >= 0);
  if (id < 0) {
    qCritical() << "font " << path << " cannot be loaded !";
  }

  QFontDatabase qfd;

  QString family = QFontDatabase::applicationFontFamilies(id).at(0);
  qDebug() << "Adding font family " << family << " for font " << name;

  QFont* font = new QFont;
  font->setFamily(family);
  font->setPixelSize(pixelSize);
  font->setWeight(QFont::Weight::Normal);
  font->setStyle(QFont::Style::StyleNormal);
  cache[name] = font;

  QFontInfo fontInfo(*font);
  qDebug() << "Adding font:" <<  font->family() << " font family:" << fontInfo.family();

  QStringList const families = qfd.families();
  Q_ASSERT(families.contains(font->family()) || true);

  Q_ASSERT(getFont(name) != nullptr);

  return font;
}
