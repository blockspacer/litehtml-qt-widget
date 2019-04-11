#pragma once

#include "common.h"

/*#include "PlatformString.h"
#include <wtf/HashSet.h>
#include <wtf/Vector.h>
#include <wtf/text/StringHash.h>*/

class MIMETypeRegistry {
public:
    static String getMIMETypeForExtension(const String& extension);
    static QVector<String> getExtensionsForMIMEType(const String& type);
    static String getPreferredExtensionForMIMEType(const String& type);
    static String getMediaMIMETypeForExtension(const String& extension);
    static QVector<String> getMediaMIMETypesForExtension(const String& extension);

    static String getMIMETypeForPath(const String& path);

    // Check to see if a mime type is suitable for being loaded inline as an
    // image (e.g., <img> tags).
    static bool isSupportedImageMIMEType(const String& mimeType);

    // Check to see if a mime type is suitable for being loaded as an image
    // document in a frame.
    static bool isSupportedImageResourceMIMEType(const String& mimeType);

    // Check to see if a mime type is suitable for being encoded.
    static bool isSupportedImageMIMETypeForEncoding(const String& mimeType);

    // Check to see if a mime type is suitable for being loaded as a JavaScript
    // resource.
    static bool isSupportedJavaScriptMIMEType(const String& mimeType);

    // Check to see if a non-image mime type is suitable for being loaded as a
    // document in a frame.  Includes supported JavaScript MIME types.
    static bool isSupportedNonImageMIMEType(const String& mimeType);

    // Check to see if a mime type is suitable for being loaded using <video> and <audio>
    static bool isSupportedMediaMIMEType(const String& mimeType);

    // Check to see if a mime type is a valid Java applet mime type
    static bool isJavaAppletMIMEType(const String& mimeType);

    // Check to see if a mime type is a plugin implemented by the
    // browser (e.g. a Qt Plugin).
    static bool isApplicationPluginMIMEType(const String& mimeType);

    static HashSet<String>& getSupportedImageMIMETypes();
    static HashSet<String>& getSupportedImageResourceMIMETypes();
    static HashSet<String>& getSupportedImageMIMETypesForEncoding();
    static HashSet<String>& getSupportedNonImageMIMETypes();
    static HashSet<String>& getSupportedMediaMIMETypes();
};

const String& defaultMIMEType();
