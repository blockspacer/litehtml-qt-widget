#include "mimetyperegistryqt.h"
#include "mimetyperegistry.h"

#include "common.h"

struct ExtensionMap {
    const char* extension;
    const char* mimeType;
};

static const ExtensionMap extensionMap[] = {
    { "bmp", "image/bmp" },
    { "css", "text/css" },
    { "gif", "image/gif" },
    { "html", "text/html" },
    { "htm", "text/html" },
    { "ico", "image/x-icon" },
    { "jpeg", "image/jpeg" },
    { "jpg", "image/jpeg" },
    { "js", "application/x-javascript" },
    { "mng", "video/x-mng" },
    { "pbm", "image/x-portable-bitmap" },
    { "pgm", "image/x-portable-graymap" },
    { "pdf", "application/pdf" },
    { "png", "image/png" },
    { "ppm", "image/x-portable-pixmap" },
    { "rss", "application/rss+xml" },
    { "svg", "image/svg+xml" },
    { "text", "text/plain" },
    { "tif", "image/tiff" },
    { "tiff", "image/tiff" },
    { "txt", "text/plain" },
    { "xbm", "image/x-xbitmap" },
    { "xml", "text/xml" },
    { "xpm", "image/x-xpm" },
    { "xsl", "text/xsl" },
    { "xhtml", "application/xhtml+xml" },
    { "wml", "text/vnd.wap.wml" },
    { "wmlc", "application/vnd.wap.wmlc" },
    { 0, 0 }
};

String MIMETypeRegistry::getMIMETypeForExtension(const String &ext)
{
    String s = ext.toLower();//lower();

    const ExtensionMap *e = extensionMap;
    while (e->extension) {
        if (s == e->extension)
            return e->mimeType;
        ++e;
    }

    return String();
}

bool MIMETypeRegistry::isApplicationPluginMIMEType(const String& mimeType)
{
    /*return mimeType.startsWith("application/x-qt-plugin", false)
        || mimeType.startsWith("application/x-qt-styled-widget", false);*/
    return false; // TODO
}
