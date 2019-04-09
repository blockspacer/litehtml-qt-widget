#include "container_qt5.h"
#include "types.h"
#include <QDebug>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QMouseEvent>
#include <QDesktopServices>

#include "fontcache.h"

// https://github.com/Nanquitas/3DS_eBook_Reader/blob/51f1fedc2565de36253104a01f4689db00c35991/source/Litehtml3DSContainer.cpp#L18
#define PPI 132.1

typedef std::map<QByteArray, QImage*> images_map;

static int totalElements = 0;

//static QString m_base_url = "://res/";

static images_map m_images;

//static QImage m_broken_img;

static QRect getRect(const litehtml::position &position) {
    return QRect(position.x, position.y, position.width, position.height);
}

static QColor getColor(const litehtml::web_color &color) {
    return QColor(color.red, color.green, color.blue, color.alpha);
}

static QString make_url( const char* url, const char* basepath, QUrl* pUrl = nullptr )
  {
    /*QUrl	u = QUrl::fromUserInput( url, (basepath && basepath[0]) ? basepath : m_base_url );
    if( pUrl )
      *pUrl = u;
    return u.toLocalFile();*/
    return QString(url) + QString(basepath);
}

static QImage* getImage( const litehtml::tchar_t* src,  const litehtml::tchar_t* baseurl )
  {
    QString fname = make_url( src, baseurl );
    images_map::iterator img = m_images.find(fname.toUtf8());
    if(img != m_images.end()) {
      return img->second;
    }

    //return &m_broken_img;
    return nullptr;
}

static bool shouldCreateElement(const litehtml::tchar_t* tag_name, const litehtml::string_map& attributes, const std::shared_ptr< litehtml::document >& doc) {
  Q_UNUSED(tag_name);
  Q_UNUSED(attributes);
  Q_UNUSED(doc);
  return true;
}

static void setPenForBorder(QPainter *painter, const litehtml::border &border)
{
    QPen pen(painter->pen());
    pen.setWidth(border.width);
    pen.setColor(getColor(border.color));
    /*
    border_style_none,
    border_style_hidden,
    border_style_dotted,
    border_style_dashed,
    border_style_solid,
    border_style_double,
    border_style_groove,
    border_style_ridge,
    border_style_inset,
    border_style_outset
    */
    switch (border.style) {
        case litehtml::border_style_none:
        case litehtml::border_style_hidden: pen.setStyle(Qt::NoPen); break;
        case litehtml::border_style_dotted: pen.setStyle(Qt::DotLine); break;
        case litehtml::border_style_dashed: pen.setStyle(Qt::DashLine); break;
        default: pen.setStyle(Qt::SolidLine); break;
    }
    painter->setPen(pen);
}

// Now for the real stuff

int container_qt5::m_defaultFontSize = 12;

container_qt5::container_qt5(QObject* parent)
    : QObject(parent), litehtml::document_container()
{
}

container_qt5::~container_qt5()
{

}

int container_qt5::getDefaultFontSize() {
  return m_defaultFontSize;
}

void container_qt5::set_document(std::shared_ptr< litehtml::document > doc)
{
    _doc = doc;
}

void container_qt5::setScroll(const QPoint& val) {
  m_Scroll = val;
}

void container_qt5::setScrollX(const int& val) {
  m_Scroll.setX(val);
}

void container_qt5::setScrollY(const int& val) {
  m_Scroll.setY(val);
}

QPoint container_qt5::getScroll() const {
  return m_Scroll;
}

void container_qt5::repaint(QPainter& painter)
{
    //qDebug() << __FUNCTION__ << m_drawArea.width();

    painter.setRenderHint(QPainter::Antialiasing);

    _doc->render(m_drawArea.width());
    _doc->draw(&painter, getScroll().x(), getScroll().y(), nullptr);

    if (_doc->width() != m_lastDocWidth ||_doc->height() != m_lastDocHeight) {
      m_lastDocWidth = _doc->width();
      m_lastDocHeight = _doc->height();
      emit docSizeChanged(_doc->width(), _doc->height());
    }
}

void container_qt5::setLastMouseCoords(int x, int y, int xClient,int yClient) {
    lastCursorX = x;
    lastCursorY = y;
    lastCursorClientX = xClient;
    lastCursorClientY = yClient;
}

void container_qt5::setSize(int w, int h)
{
  m_drawArea.setWidth(w);
  m_drawArea.setHeight(h);
}

void container_qt5::get_language(litehtml::tstring& language, litehtml::tstring& culture) const
{
    qDebug() << "get_language";
    language = _t("en");
    culture = _t("");
}

void container_qt5::get_media_features(litehtml::media_features& media) const
{
    //qDebug() << "get_media_features";

    media.type = litehtml::media_type_screen;
    media.width = m_drawArea.width();
    media.height = m_drawArea.height();
    media.color = 8;
    media.monochrome = 0;
    media.color_index = 256;
    media.resolution = 96;
    media.device_width = m_drawArea.width();
    media.device_height = m_drawArea.height();

    qDebug() << "=> " << media.width << "x" << media.height;
}

// see https://github.com/litehtml/litebrowser/blob/master/src/ToolbarWnd.cpp#L572
std::shared_ptr< litehtml::element > container_qt5::create_element(const litehtml::tchar_t* tag_name, const litehtml::string_map& attributes, const std::shared_ptr< litehtml::document >& doc)
{
    //qDebug() << __FUNCTION__ << " this one can be 0";
    //qDebug() << __FUNCTION__ << " this one can be 0";
    //doc->root()->get_child()
  if(!shouldCreateElement(tag_name, attributes, doc)) {
    return std::shared_ptr<litehtml::element>();
  }

  /*if (!t_strcasecmp(tag_name, _t("input")))
  {
    return std::make_shared<container_el_inputbox>(doc, attributes);
  }*/

  litehtml::tstring attributeStr;
  for (auto attr : attributes)
  {
    attributeStr.append(attr.first);
    attributeStr.append(_t("="));
    attributeStr.append(attr.second);
    attributeStr.append(_t("\n"));
  }
  std::string _attributes = std::string(attributeStr.begin(), attributeStr.end());

  //ElementInfo elementInfo = {};
  int elementID = totalElements++;
  if (elementID > 0)
  {
    // TODO

    //std::shared_ptr<TagElement> result = std::make_shared(doc);
    //result->SetManagedInfo(elementInfo);
    //_elements[elementID] = result;
    //return std::shared_ptr<litehtml::element>(result.get());
    //return std::make_shared<litehtml::element>(doc);
    return std::shared_ptr<litehtml::element>();
  }

  return std::shared_ptr<litehtml::element>();
}

void container_qt5::get_client_rect(litehtml::position& client) const
{
    //qDebug() << "get_client_rect";
    // No scroll yet
    client.move_to(0, 0);
    client.width = m_drawArea.width();
    client.height = m_drawArea.height();

    //qDebug() << "==> " << client.width << "x" << client.height;
}

// Deletes the last clipping.
void container_qt5::del_clip()
{
    //qDebug() << "del_clip";
}

// Set the painting clip rectangle here. valid_x and valid_y are ignored.
// Please note, litehtml can set some clip rects.
// You have to save the clip positions and apply clipping on draw something.
void container_qt5::set_clip(const litehtml::position& pos, const litehtml::border_radiuses& bdr_radius, bool valid_x, bool valid_y)
{
  Q_UNUSED(pos);
  Q_UNUSED(bdr_radius);
  Q_UNUSED(bdr_radius);
  Q_UNUSED(valid_x);
  Q_UNUSED(valid_y);
    //qDebug() << "set_clip";
}

void container_qt5::import_css(litehtml::tstring& text, const litehtml::tstring& url, litehtml::tstring& baseurl)
{
  QFile qFile(url.c_str());

  if(!qFile.exists())
  {
    qDebug() << "nonexistent file " << qFile.fileName();
    return;
  }

  if(!qFile.open(QIODevice::ReadOnly))
  {
     qDebug() << "Can`t read data from " << qFile.fileName();
     return;
  }

  //QByteArray fileData = qFile.readAll();
  m_loaded_css[QString(url.c_str())] = qFile.readAll();
  QByteArray& fileData = m_loaded_css[QString(url.c_str())];

  if(!fileData.size())
  {
    qDebug() << "Empty data from " << qFile.fileName();
    return;
  }

  qFile.close();

  text = (const char*)&fileData.data()[0];

  //qDebug() << "imported css" << url.c_str();
}

void container_qt5::transform_text(litehtml::tstring& text, litehtml::text_transform tt)
{
  //qDebug() << "transform_text";

// TODO: UTF8 https://github.com/pinaraf/litehtml-qt/blob/f3f4959f1d4a4884a0a18d87db7483e886cc3ee3/containers/cairo/cairo_container.cpp#L942

/*
 * The text-transform property in CSS controls text case and capitalization.
    lowercase makes all of the letters in the selected text lowercase.
    uppercase makes all of the letters in the selected text uppercase.
    capitalize capitalizes the first letter of each word in the selected text.
    none leaves the text's case and capitalization exactly as it was entered.
    inherit gives the text the case and capitalization of its parent.
 */

  std::string temp = text;

  // capitalizes the first letter of each word
  if (tt == litehtml::text_transform_capitalize) {
    // Convert lowercase letter to uppercase
    // see http://www.cplusplus.com/reference/cctype/toupper/
    temp[0] = toupper(temp[0]); // TODO: do to each word

  } else if (tt == litehtml::text_transform_uppercase) {
    // std::transform applies the given function to a range and stores the result in another range
    // see https://en.cppreference.com/w/cpp/algorithm/transform
    std::transform(temp.begin(), temp.end(), temp.begin(), ::toupper);

  } else if (tt == litehtml::text_transform_lowercase) {
    // std::transform applies the given function to a range and stores the result in another range
    // see https://en.cppreference.com/w/cpp/algorithm/transform
    std::transform(temp.begin(), temp.end(), temp.begin(), ::tolower);
  }

  text = temp.c_str();
}

void container_qt5::set_cursor(const litehtml::tchar_t* cursor_)
{
    /*QString cursor(cursor_);
    if (cursor == "auto")
        setCursor(Qt::IBeamCursor);
    else if (cursor == "pointer")
        setCursor(Qt::PointingHandCursor);
    else
        qDebug() << __FUNCTION__ << cursor;*/
}

void container_qt5::on_anchor_click(const litehtml::tchar_t* url, const litehtml::element::ptr& el)
{
    //qDebug() << __FUNCTION__ << url;
    QDesktopServices::openUrl(QUrl(url));
}

void container_qt5::link(const std::shared_ptr< litehtml::document >& doc, const litehtml::element::ptr& el)
{
    //qDebug() << __FUNCTION__;
}

void container_qt5::set_base_url(const litehtml::tchar_t* base_url)
{
    //qDebug() << __FUNCTION__;
}

void container_qt5::set_caption(const litehtml::tchar_t* caption)
{
    //qDebug() << __FUNCTION__;
}

void container_qt5::draw_borders(litehtml::uint_ptr hdc, const litehtml::borders& borders, const litehtml::position& draw_pos, bool root)
{
    //qDebug() << __FUNCTION__ << " for root = " << root;
    QPainter *painter = (QPainter *) hdc;
    painter->save();
    QRect area = getRect(draw_pos);
    if (root) {
        painter->setPen(Qt::NoPen);
        painter->fillRect(area, Qt::white);
    } else {
        if (borders.top.style != litehtml::border_style_none) {
            setPenForBorder(painter, borders.top);
            painter->drawLine(area.topLeft(), area.topRight());
        }
        if (borders.bottom.style != litehtml::border_style_none) {
            setPenForBorder(painter, borders.bottom);
            painter->drawLine(area.bottomLeft(), area.bottomRight());
        }
        if (borders.left.style != litehtml::border_style_none) {
            setPenForBorder(painter, borders.left);
            painter->drawLine(area.topLeft(), area.bottomLeft());
        }
        if (borders.right.style != litehtml::border_style_none) {
            setPenForBorder(painter, borders.right);
            painter->drawLine(area.topRight(), area.bottomRight());
        }
    }
    painter->restore();
}

void container_qt5::draw_background(litehtml::uint_ptr hdc, const litehtml::background_paint& bg)
{
    QPainter* pnt = (QPainter*)hdc;
    //apply_clip( pnt );

    //qDebug() << "draw_background" << __FUNCTION__ << bg.image.c_str();
    // draw_background draw_background https://images2017.cnblogs.com/blog/847289/201712/847289-20171207174342691-530707282.png

    // clip_box. Defines the position of the clipping box. See the background-clip CSS property.
    QPoint clip_a(offsetX + bg.clip_box.left(), offsetY + bg.clip_box.top());
    QPoint clip_b(offsetX + bg.clip_box.right(), offsetY + bg.clip_box.bottom());
    QRect clipRect(clip_a, clip_b);

    // The image contains the background image URL. URL can be relative.
    // Use the baseurl member to find the base URL for image.
    if (bg.image.empty())
    {
      // This member defines the background-color CSS property.
      pnt->fillRect(clipRect, QBrush(getColor(bg.color)) );
      return;
    }


    // Defines the position of the origin box. See the background-origin CSS property.
    // The background-origin property specifies the origin position (the background positioning area) of a background image.
    // bg.origin_box
    // TODO https://github.com/PingmanTools/LiteHtmlSharp/blob/3f4db0ff0ab4b5cd7f5e10aa1b6ed94f2eee0bcb/LiteHtmlLib/src/DocContainer.cpp#L67


    /*
      This is the background-attachment CSS property. Can be one of the following values:

      background_attachment_scroll - CSS scroll
      background_attachment_fixed - - CSS fixed
    */
    if (bg.attachment == litehtml::background_attachment::background_attachment_fixed)
    {
      // TODO
      return;
    }

    /// \note color used to fill rect if image not found/broken
    QColor dummyColor(100, 100, 100, 100);

    // TODO: https://github.com/nem0/lumixengine_html/blob/e2da43e704ad0ad474c8ecafc16fcae4a51e8aff/src/editor/plugins.cpp#L162
    //Crc32 crc32;
    //auto iter = m_images.find(crc32.calculateFromData(bg.image.c_str()));
    auto iter = m_images.find(bg.image.c_str());
    if (iter == m_images.end()) {
      //qDebug() << "iter == m_images.end()" << __FUNCTION__;
      // drw placeholder
      pnt->fillRect(clipRect, dummyColor );
      return;
    }

    QImage* img = getImage( bg.image.c_str(), bg.baseurl.c_str() );
    if (!img) {
      pnt->fillRect(clipRect, dummyColor );
      return;
    }

    //auto img = iter.value();

    /*
     * This the background-repeat CSS property. Can be one of the following values:

        background_repeat_repeat - CSS repeat
        background_repeat_repeat_x - CSS repeat-x
        background_repeat_repeat_y - CSS repeat-y
        background_repeat_no_repeat - CSS no-repeat
     */

    auto imscaled = img->scaled(clip_b.x() - clip_a.x(), clip_b.y() - clip_a.y(), Qt::IgnoreAspectRatio,Qt::SmoothTransformation);

    QBrush *brushTiledImage = new QBrush(imscaled);
    brushTiledImage->setStyle(Qt::BrushStyle::TexturePattern);

    switch (bg.repeat)
    {
      case litehtml::background_repeat_no_repeat:
      {
        //pnt->drawImage( bg.position_x, bg.position_y, *img, 0, 0, img->width(), img->height() );
        pnt->drawImage( clip_a.x(), clip_a.y(), imscaled, 0, 0, clip_b.x() - clip_a.x(), clip_b.y() - clip_a.y() );
        //win->DrawList->AddImage(img.textureId, clip_a, clip_b);
        break;
      }
      case litehtml::background_repeat_repeat_x:
      {
        pnt->setBackground(*brushTiledImage);
        pnt->drawImage( clip_a.x(), clip_a.y(), imscaled, 0, 0, clip_b.x() - clip_a.x(), clip_b.y() - clip_a.y() );
        //ImVec2 uv((clip_b.x - clip_a.x) / img.w, 0);
        //glBindTexture(GL_TEXTURE_2D, my_opengl_texture);
        //ImGui::GetWindowDrawList()->PushTextureID(img.textureId);
        //win->DrawList->AddImage(img.textureId, clip_a, clip_b, ImVec2(0, 0), uv);
        //ImGui::GetWindowDrawList()->PopTextureID();
        break;
      }
      break;
      case litehtml::background_repeat_repeat_y:
      {
        pnt->setBackground(*brushTiledImage);
        pnt->drawImage( clip_a.x(), clip_a.y(), imscaled, 0, 0, clip_b.x() - clip_a.x(), clip_b.y() - clip_a.y() );
        //ImVec2 uv(0, (clip_b.y - clip_a.y) / img.h);
        //glBindTexture(GL_TEXTURE_2D, my_opengl_texture);
        //ImGui::GetWindowDrawList()->PushTextureID(img.textureId);
        //win->DrawList->AddImage(img.textureId, clip_a, clip_b, ImVec2(0, 0), uv);
        //ImGui::GetWindowDrawList()->PopTextureID();
        break;
      }
      case litehtml::background_repeat_repeat:
      {
        pnt->setBackground(*brushTiledImage);
        pnt->drawImage( clip_a.x(), clip_a.y(), imscaled, 0, 0, clip_b.x() - clip_a.x(), clip_b.y() - clip_a.y() );
        //ImVec2 uv((clip_b.x - clip_a.x) / img.w, (clip_b.y - clip_a.y) / img.h);
        //glBindTexture(GL_TEXTURE_2D, my_opengl_texture);
        //ImGui::GetWindowDrawList()->PushTextureID(img.textureId);
        //win->DrawList->AddImage(img.textureId, clip_a, clip_b, ImVec2(0, 0), uv);
        //ImGui::GetWindowDrawList()->PopTextureID();
        break;
      }
    }
}

void container_qt5::get_image_size(const litehtml::tchar_t* src, const litehtml::tchar_t* baseurl, litehtml::size& sz)
{
  //qDebug() << __FUNCTION__;
  QImage*	img = getImage( src, baseurl );
  if( img ) {
    sz.width = img->width();
    sz.height = img->height();
  } else {
    sz.width = sz.height = 100;
  }
}

void container_qt5::load_image(const litehtml::tchar_t* src, const litehtml::tchar_t* baseurl, bool redraw_on_ready)
{
  //qDebug() << __FUNCTION__ << src << baseurl;

  QString fname = make_url( src, baseurl );

  if( m_images.find(fname.toUtf8())==m_images.end() ) {
    QImage*	img = new QImage( );
    if( img->load( fname ) ) {
      m_images[fname.toUtf8()] = img;
    }
    else {
      qWarning() << "cannot load image: " << fname;
    }
  }
}

void container_qt5::draw_list_marker(litehtml::uint_ptr hdc, const litehtml::list_marker& marker)
{
    //qDebug() << __FUNCTION__;
    //qDebug() << marker.marker_type << marker.pos.x << "x" << marker.pos.y << marker.pos.left() << marker.pos.right();
    //qDebug() << marker.baseurl << QString::fromStdString(marker.image);

    QPainter *painter = (QPainter *) hdc;
    QRect position = getRect(marker.pos);
    QColor color = getColor(marker.color);
    painter->save();
    painter->setPen(color);
    switch (marker.marker_type) {
        case litehtml::list_style_type_none: break;
        case litehtml::list_style_type_circle:
          painter->setPen( QPen( color ) );
          painter->setBrush( Qt::NoBrush );
          painter->drawEllipse(position);
          break;
        case litehtml::list_style_type_disc:
          painter->setBrush(color);
          painter->drawEllipse(position);
          break;
        case litehtml::list_style_type_square:
          painter->fillRect(position, getColor(marker.color));
          break;
        // How to implement numeral markers ??
        default: break;
    }
    painter->restore();
}
/*
		list_style_type_none,
		list_style_type_circle,
		list_style_type_disc,
		list_style_type_square,
		list_style_type_armenian,
		list_style_type_cjk_ideographic,
		list_style_type_decimal,
		list_style_type_decimal_leading_zero,
		list_style_type_georgian,
		list_style_type_hebrew,
		list_style_type_hiragana,
		list_style_type_hiragana_iroha,
		list_style_type_katakana,
		list_style_type_katakana_iroha,
		list_style_type_lower_alpha,
		list_style_type_lower_greek,
		list_style_type_lower_latin,
		list_style_type_lower_roman,
		list_style_type_upper_alpha,
		list_style_type_upper_latin,
		list_style_type_upper_roman,
*/

const litehtml::tchar_t* container_qt5::get_default_font_name() const
{
    //qDebug() << __FUNCTION__;
    return FontCache::getInstance()->getDefaultFontName().toStdString().c_str();
}

int container_qt5::get_default_font_size() const
{
    //qDebug() << __FUNCTION__;
    return m_defaultFontSize;
}

int container_qt5::pt_to_px(int pt)
{
    //qDebug() << __FUNCTION__;
    return pt / 72 * PPI;
}

void container_qt5::draw_text(litehtml::uint_ptr hdc, const litehtml::tchar_t* text, litehtml::uint_ptr hFont, litehtml::web_color color, const litehtml::position& pos)
{
    //qDebug() << __FUNCTION__;
    QPainter *painter = (QPainter *) hdc;
    QFont *font = (QFont *) hFont;
    //QFont *font = new QFont("Arial", 12, 12, false);
    painter->setFont(*font);
    //painter->setFont(QFont("Times",22));
    painter->setPen(getColor(color));
    //painter->setBrush(QColor(color.red, color.green, color.blue, color.alpha));
    QFontMetrics metrics(*font);
    painter->drawText(pos.x, pos.bottom() - metrics.descent(), text);

    //qDebug() << "Paint " << text << " at " << pos.x << "x" << pos.y;
    //QString text2 = QString::fromUtf8(qPrintable(u8"F?llungRaupeStepываываtext"));//tr("F?llungRaupeStepываываtext");
    //painter->drawText(pos.x, pos.bottom() - metrics.descent(), QString::fromUtf8(qPrintable(text)));
    //painter->drawText(pos.x, pos.bottom() - metrics.descent(), text2.toUtf8());
    //painter->drawText(pos.x, pos.bottom() - metrics.descent(), text2.toLatin1());
    //painter->drawText(pos.x, pos.bottom() - metrics.descent(), text2.toLocal8Bit());
}

int container_qt5::text_width(const litehtml::tchar_t* text, litehtml::uint_ptr hFont)
{
    if (!hFont) {
      qDebug() << __FUNCTION__ << " can`t get text width for empty font";
      return 50;
    }

    //qDebug() << __FUNCTION__;
    QFont *font = (QFont *) hFont;
    QFontMetrics metrics(*font);
    QString txt(text);
    //qDebug() << "For" << txt << metrics.boundingRect(txt);
    if (txt == " ") {
        return metrics.boundingRect("x").width();
    }
    return metrics.boundingRect(txt).width();
}

void container_qt5::delete_font(litehtml::uint_ptr hFont)
{
    //qDebug() << __FUNCTION__;
    QFont *font = (QFont *) hFont;
    delete(font);
}

litehtml::uint_ptr container_qt5::create_font(const litehtml::tchar_t* faceName, int size, int weight, litehtml::font_style italic, unsigned int decoration, litehtml::font_metrics* fm)
{
    //TODO: decoration
    qDebug() << __FUNCTION__ << " for " << faceName << size << weight;
    //QFont *font = new QFont("Arial Unicode MS", size, weight, italic == litehtml::fontStyleItalic);
    //QFont *font = new QFont(faceName, size, weight, italic == litehtml::fontStyleItalic);
    QFont *font = FontCache::getInstance()->getFont(faceName);
    if (!font) {
      qDebug() << "unsupported font " << faceName;
      return FontCache::getInstance()->getFont(FontCache::getInstance()->getDefaultFontName());
    }
    font->setUnderline(decoration & litehtml::font_decoration_underline);
    font->setOverline(decoration & litehtml::font_decoration_overline);
    font->setStrikeOut(decoration & litehtml::font_decoration_linethrough);
    QFontMetrics metrics(*font);
    fm->height = metrics.height();//metrics.ascent() + metrics.descent() + 2;
    fm->ascent = metrics.ascent();
    fm->descent = metrics.descent();
    fm->x_height = metrics.xHeight();//metrics.boundingRect("x").height();
    return font;//&QFont("Cousine");//
}

litehtml::tstring container_qt5::resolve_color(const litehtml::tstring& color) const
{
  struct custom_color
  {
    litehtml::tchar_t*	name;
    int					color_index;
  };
/*
  static custom_color colors[] = {
    { _t("ActiveBorder"),          COLOR_ACTIVEBORDER},
    { _t("ActiveCaption"),         COLOR_ACTIVECAPTION},
    { _t("AppWorkspace"),          COLOR_APPWORKSPACE },
    { _t("Background"),            COLOR_BACKGROUND },
    { _t("ButtonFace"),            COLOR_BTNFACE },
    { _t("ButtonHighlight"),       COLOR_BTNHIGHLIGHT },
    { _t("ButtonShadow"),          COLOR_BTNSHADOW },
    { _t("ButtonText"),            COLOR_BTNTEXT },
    { _t("CaptionText"),           COLOR_CAPTIONTEXT },
        { _t("GrayText"),              COLOR_GRAYTEXT },
    { _t("Highlight"),             COLOR_HIGHLIGHT },
    { _t("HighlightText"),         COLOR_HIGHLIGHTTEXT },
    { _t("InactiveBorder"),        COLOR_INACTIVEBORDER },
    { _t("InactiveCaption"),       COLOR_INACTIVECAPTION },
    { _t("InactiveCaptionText"),   COLOR_INACTIVECAPTIONTEXT },
    { _t("InfoBackground"),        COLOR_INFOBK },
    { _t("InfoText"),              COLOR_INFOTEXT },
    { _t("Menu"),                  COLOR_MENU },
    { _t("MenuText"),              COLOR_MENUTEXT },
    { _t("Scrollbar"),             COLOR_SCROLLBAR },
    { _t("ThreeDDarkShadow"),      COLOR_3DDKSHADOW },
    { _t("ThreeDFace"),            COLOR_3DFACE },
    { _t("ThreeDHighlight"),       COLOR_3DHILIGHT },
    { _t("ThreeDLightShadow"),     COLOR_3DLIGHT },
    { _t("ThreeDShadow"),          COLOR_3DSHADOW },
    { _t("Window"),                COLOR_WINDOW },
    { _t("WindowFrame"),           COLOR_WINDOWFRAME },
    { _t("WindowText"),            COLOR_WINDOWTEXT }
  };

    if (color == _t("Highlight"))
    {
        int iii = 0;
        iii++;
    }

    for (auto& clr : colors)
    {
        if (!t_strcasecmp(clr.name, color.c_str()))
        {
            litehtml::tchar_t  str_clr[20];
            DWORD rgb_color =  GetSysColor(clr.color_index);
#ifdef LITEHTML_UTF8
            StringCchPrintfA(str_clr, 20, "#%02X%02X%02X", GetRValue(rgb_color), GetGValue(rgb_color), GetBValue(rgb_color));
#else
            StringCchPrintf(str_clr, 20, L"#%02X%02X%02X", GetRValue(rgb_color), GetGValue(rgb_color), GetBValue(rgb_color));
#endif // LITEHTML_UTF8
            return std::move(litehtml::tstring(str_clr));
        }
    }*/
    return std::move(litehtml::tstring());
}


litehtmlWidget::litehtmlWidget(QWidget *parent)
    : QWidget(parent), container(new container_qt5())
{
  setMouseTracking(true);
}

litehtmlWidget::~litehtmlWidget()
{

}

void litehtmlWidget::paintEvent(QPaintEvent *event)
{
    container->setSize(width(), height());

    QPainter painter(this);
    container->repaint(painter);
}

void litehtmlWidget::mouseMoveEvent(QMouseEvent *event)
{
    litehtml::position::vector vec;
    container->setLastMouseCoords(event->x(), event->y(), event->x(), event->y());

    if (container->getDocument()->on_mouse_over(event->x(), event->y(), event->x(), event->y(), vec)) {
      repaint();
    }
}

void litehtmlWidget::mousePressEvent(QMouseEvent *event)
{
    litehtml::position::vector vec;
    container->setLastMouseCoords(event->x(), event->y(), event->x(), event->y());

    if (container->getDocument()->on_lbutton_down(event->x(), event->y(), event->x(), event->y(), vec)) {
      repaint();
    }
}

void litehtmlWidget::mouseReleaseEvent(QMouseEvent *event)
{
    litehtml::position::vector vec;
    container->setLastMouseCoords(event->x(), event->y(), event->x(), event->y());

    if (container->getDocument()->on_lbutton_up(event->x(), event->y(), event->x(), event->y(), vec)) {
      repaint();
    }
}
