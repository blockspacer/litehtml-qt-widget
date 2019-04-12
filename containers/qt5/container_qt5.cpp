#include "container_qt5.h"
#include "types.h"
#include <QDebug>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QMouseEvent>
#include <QDesktopServices>

#include "graphicscontext.h"

#include "shadowdata.h"

#include "common.h"
#include "image.h"
#include "imagesource.h"
#include "stillimageqt.h"

#include "fontcache.h"
#include "pathqt.h"
#include <cmath>

// https://github.com/Nanquitas/3DS_eBook_Reader/blob/51f1fedc2565de36253104a01f4689db00c35991/source/Litehtml3DSContainer.cpp#L18
#define PPI 132.1

static litehtml::position::vector	m_clips;

typedef std::map<QByteArray, QImage*> images_map;

static int totalElements = 0;

//static QString m_base_url = "://res/";

static images_map m_imagesCache;

//static QImage m_broken_img;

static QRect getRect(const litehtml::position &position) {
  return QRect(position.x, position.y, position.width, position.height);
}

static QColor getColor(const litehtml::web_color &color) {
  return QColor(color.red, color.green, color.blue, color.alpha);
}

static void	apply_clip( QPainter* pnt )
{
  if(!m_clips.empty())
  {
    litehtml::position clip_pos = m_clips.back();
    //pnt->setClipRect( clip_pos.left(), clip_pos.top(), clip_pos.width, clip_pos.height );
  }
  /*

  for(const auto& clip_box : m_clips)
  {
    rounded_rectangle(cr, clip_box.box, clip_box.radius);
    cairo_clip(cr);
  }
  */
}

static void	intersect_clip( IntRect& rect ) {
  if(!m_clips.empty())
  {
    litehtml::position clip_pos = m_clips.back();
    rect.intersect(IntRect(clip_pos.x, clip_pos.y, clip_pos.width, clip_pos.height));
    //pnt->setClipRect( clip_pos.left(), clip_pos.top(), clip_pos.width, clip_pos.height );
  }
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
  images_map::iterator img = m_imagesCache.find(fname.toUtf8());
  if(img != m_imagesCache.end()) {
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

container_qt5::container_qt5(QWidget* parent)
    : QObject(parent), litehtml::document_container()
{
    m_owner = parent;
    Q_ASSERT(m_owner != nullptr);
}

container_qt5::~container_qt5()
{

}

int container_qt5::getDefaultFontSize() {
  return m_defaultFontSize;
}

/*void container_qt5::setDrawArea(const QRect &area)
{
  m_drawArea = area;
}

QRect container_qt5::getDrawArea() const {
  return m_drawArea;
}*/

void container_qt5::set_document(std::shared_ptr< litehtml::document > doc)
{
    _doc = doc;
}

void container_qt5::setScroll(const QPoint& val) {
  m_Scroll = val;

  // allows resizing of painting area
  _doc->media_changed();
}

void container_qt5::setScrollX(const int& val) {
  m_Scroll.setX(val);

  // allows resizing of painting area
  _doc->media_changed();
}

void container_qt5::setScrollY(const int& val) {
  m_Scroll.setY(val);

  // allows resizing of painting area
  _doc->media_changed();
}

QPoint container_qt5::getScroll() const {
  return m_Scroll;
}

void container_qt5::repaint(QPainter& painter)
{
    //qDebug() << __FUNCTION__ << m_drawArea.width();

    painter.setRenderHint(QPainter::Antialiasing);

    QRect	rc = m_owner->rect();

    //_doc->render(m_drawArea.width());

    /// \note don`t allow render size < 1
    _doc->render(std::max(rc.width(), 1));
    //_doc->render(5000);

    litehtml::position clipPos;
    clipPos.width 	= rc.width();
    clipPos.height 	= rc.height();
    clipPos.x 		= rc.x();
    clipPos.y 		= rc.y();

    //GraphicsContext* graphicsContext = new GraphicsContext(&painter);

    // https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/WebKit2/WebProcess/WebPage/qt/ChunkedUpdateDrawingAreaQt.cpp#L44
    GraphicsContext graphicsContext(&painter);

    Q_ASSERT(graphicsContext.platformContext() != nullptr);
    _doc->draw(static_cast<litehtml::uint_ptr>(&graphicsContext), getScroll().x(), getScroll().y(), &clipPos);

/*qDebug() << "m_lastDocWidth" << m_lastDocWidth;
qDebug() << "m_lastDocHeight" << m_lastDocHeight;
qDebug() << "_doc->width()" << _doc->width();
qDebug() << "_doc->height()" << _doc->height();*/

    if (_doc->width() != m_lastDocWidth || _doc->height() != m_lastDocHeight) {
      emit docSizeChanged(_doc->width(), _doc->height());
      m_lastDocWidth = _doc->width();
      m_lastDocHeight = _doc->height();
    }
}

void container_qt5::setLastMouseCoords(int x, int y, int xClient,int yClient) {
    lastCursorX = x;
    lastCursorY = y;
    lastCursorClientX = xClient;
    lastCursorClientY = yClient;
}

/*void container_qt5::setSize(int w, int h)
{
  m_drawArea.setWidth(w);
  m_drawArea.setHeight(h);
}*/

void container_qt5::get_language(litehtml::tstring& language, litehtml::tstring& culture) const
{
    qDebug() << "get_language";
    language = _t("en");
    culture = _t("");
}

void container_qt5::get_media_features(litehtml::media_features& media) const
{
    //qDebug() << "get_media_features";

    QRect	rc = m_owner->rect();

    media.type = litehtml::media_type_screen;
    media.width = rc.width();
    media.height = rc.height();
    media.color = 8;
    media.monochrome = 0;
    media.color_index = 256;
    media.resolution = 96;
    media.device_width = rc.width();
    media.device_height = rc.height();

    //qDebug() << "=> " << media.width << "x" << media.height;
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
    /*//qDebug() << "get_client_rect";
    // No scroll yet
    client.move_to(0, 0);
    client.width = m_drawArea.width();
    client.height = m_drawArea.height();

    //qDebug() << "==> " << client.width << "x" << client.height;*/

    QRect	rc = m_owner->rect();
    client.x = rc.left();
    client.y = rc.top();

    // ISSUE: draw body with 100% background size
    client.width = rc.width()-m_Scroll.x();//std::max(_doc->width(), rc.width());
    client.height = rc.height()-m_Scroll.y();//_doc->height();//std::max(_doc->height(), rc.height());
}

// Deletes the last clipping.
void container_qt5::del_clip()
{
    //qDebug() << "del_clip";
    if(!m_clips.empty())
    {
      m_clips.pop_back();
      //if(!m_clips.empty())
      //{
      //	litehtml::position clip_pos = m_clips.back();
      //}
    }
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

    litehtml::position clip_pos = pos;
    QRect	rc = m_owner->rect( );

    if(!valid_x)
    {
      clip_pos.x		= 0;//rc.left();
      clip_pos.width	= rc.width();
    }

    if(!valid_y)
    {
      clip_pos.y		= 0;//rc.top();
      clip_pos.height	= rc.height();
    }

    m_clips.push_back( clip_pos );

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

    QString cursor(cursor_);

    QCursor	c( Qt::ArrowCursor );

    if( (cursor=="pointer") ) {
      c.setShape( Qt::PointingHandCursor );
    }

    m_owner->setCursor( c );
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

static IntRect borderInnerRect(const IntRect& borderRect, unsigned short topWidth, unsigned short bottomWidth, unsigned short leftWidth, unsigned short rightWidth)
{
    return IntRect(
            borderRect.x() + leftWidth,
            borderRect.y() + topWidth,
            borderRect.width() - leftWidth - rightWidth,
            borderRect.height() - topWidth - bottomWidth);
}

static bool borderWillArcInnerEdge(const IntSize& firstRadius, const IntSize& secondRadius, int firstBorderWidth, int secondBorderWidth, int middleBorderWidth)
{
    // FIXME: This test is insufficient. We need to take border style into account.
    return (!firstRadius.width() || firstRadius.width() >= firstBorderWidth)
            && (!firstRadius.height() || firstRadius.height() >= middleBorderWidth)
            && (!secondRadius.width() || secondRadius.width() >= secondBorderWidth)
            && (!secondRadius.height() || secondRadius.height() >= middleBorderWidth);
}

static void drawLineForBoxSide(ColorSpace colorSpace, GraphicsContext* graphicsContext, int x1, int y1, int x2, int y2,
                                      BoxSide s, Color c, EBorderStyle style,
                                      int adjbw1, int adjbw2)
{
    int width = (s == BSTop || s == BSBottom ? y2 - y1 : x2 - x1);

    if (style == DOUBLE && width < 3)
        style = SOLID;

    switch (style) {
        case BNONE:
        case BHIDDEN:
            return;
        case DOTTED:
        case DASHED:
            graphicsContext->setStrokeColor(c, colorSpace);
            graphicsContext->setStrokeThickness(width);
            graphicsContext->setStrokeStyle(style == DASHED ? DashedStroke : DottedStroke);

            if (width > 0)
                switch (s) {
                    case BSBottom:
                    case BSTop:
                        graphicsContext->drawLine(IntPoint(x1, (y1 + y2) / 2), IntPoint(x2, (y1 + y2) / 2));
                        break;
                    case BSRight:
                    case BSLeft:
                        graphicsContext->drawLine(IntPoint((x1 + x2) / 2, y1), IntPoint((x1 + x2) / 2, y2));
                        break;
                }
            break;
        case DOUBLE: {
            int third = (width + 1) / 3;

            if (adjbw1 == 0 && adjbw2 == 0) {
                graphicsContext->setStrokeStyle(NoStroke);
                graphicsContext->setFillColor(c, colorSpace);
                switch (s) {
                    case BSTop:
                    case BSBottom:
                        graphicsContext->drawRect(IntRect(x1, y1, x2 - x1, third));
                        graphicsContext->drawRect(IntRect(x1, y2 - third, x2 - x1, third));
                        break;
                    case BSLeft:
                        graphicsContext->drawRect(IntRect(x1, y1 + 1, third, y2 - y1 - 1));
                        graphicsContext->drawRect(IntRect(x2 - third, y1 + 1, third, y2 - y1 - 1));
                        break;
                    case BSRight:
                        graphicsContext->drawRect(IntRect(x1, y1 + 1, third, y2 - y1 - 1));
                        graphicsContext->drawRect(IntRect(x2 - third, y1 + 1, third, y2 - y1 - 1));
                        break;
                }
            } else {
                int adjbw1bigthird = ((adjbw1 > 0) ? adjbw1 + 1 : adjbw1 - 1) / 3;
                int adjbw2bigthird = ((adjbw2 > 0) ? adjbw2 + 1 : adjbw2 - 1) / 3;

                switch (s) {
                    case BSTop:
                        drawLineForBoxSide(colorSpace, graphicsContext, x1 + max((-adjbw1 * 2 + 1) / 3, 0),
                                   y1, x2 - max((-adjbw2 * 2 + 1) / 3, 0), y1 + third,
                                   s, c, SOLID, adjbw1bigthird, adjbw2bigthird);
                        drawLineForBoxSide(colorSpace, graphicsContext, x1 + max((adjbw1 * 2 + 1) / 3, 0),
                                   y2 - third, x2 - max((adjbw2 * 2 + 1) / 3, 0), y2,
                                   s, c, SOLID, adjbw1bigthird, adjbw2bigthird);
                        break;
                    case BSLeft:
                        drawLineForBoxSide(colorSpace, graphicsContext, x1, y1 + max((-adjbw1 * 2 + 1) / 3, 0),
                                   x1 + third, y2 - max((-adjbw2 * 2 + 1) / 3, 0),
                                   s, c, SOLID, adjbw1bigthird, adjbw2bigthird);
                        drawLineForBoxSide(colorSpace, graphicsContext, x2 - third, y1 + max((adjbw1 * 2 + 1) / 3, 0),
                                   x2, y2 - max((adjbw2 * 2 + 1) / 3, 0),
                                   s, c, SOLID, adjbw1bigthird, adjbw2bigthird);
                        break;
                    case BSBottom:
                        drawLineForBoxSide(colorSpace, graphicsContext, x1 + max((adjbw1 * 2 + 1) / 3, 0),
                                   y1, x2 - max((adjbw2 * 2 + 1) / 3, 0), y1 + third,
                                   s, c, SOLID, adjbw1bigthird, adjbw2bigthird);
                        drawLineForBoxSide(colorSpace, graphicsContext, x1 + max((-adjbw1 * 2 + 1) / 3, 0),
                                   y2 - third, x2 - max((-adjbw2 * 2 + 1) / 3, 0), y2,
                                   s, c, SOLID, adjbw1bigthird, adjbw2bigthird);
                        break;
                    case BSRight:
                        drawLineForBoxSide(colorSpace, graphicsContext, x1, y1 + max((adjbw1 * 2 + 1) / 3, 0),
                                   x1 + third, y2 - max((adjbw2 * 2 + 1) / 3, 0),
                                   s, c, SOLID, adjbw1bigthird, adjbw2bigthird);
                        drawLineForBoxSide(colorSpace, graphicsContext, x2 - third, y1 + max((-adjbw1 * 2 + 1) / 3, 0),
                                   x2, y2 - max((-adjbw2 * 2 + 1) / 3, 0),
                                   s, c, SOLID, adjbw1bigthird, adjbw2bigthird);
                        break;
                    default:
                        break;
                }
            }
            break;
        }
        case RIDGE:
        case GROOVE:
        {
            EBorderStyle s1;
            EBorderStyle s2;
            if (style == GROOVE) {
                s1 = INSET;
                s2 = OUTSET;
            } else {
                s1 = OUTSET;
                s2 = INSET;
            }

            int adjbw1bighalf = ((adjbw1 > 0) ? adjbw1 + 1 : adjbw1 - 1) / 2;
            int adjbw2bighalf = ((adjbw2 > 0) ? adjbw2 + 1 : adjbw2 - 1) / 2;

            switch (s) {
                case BSTop:
                    drawLineForBoxSide(colorSpace, graphicsContext, x1 + max(-adjbw1, 0) / 2, y1, x2 - max(-adjbw2, 0) / 2, (y1 + y2 + 1) / 2,
                               s, c, s1, adjbw1bighalf, adjbw2bighalf);
                    drawLineForBoxSide(colorSpace, graphicsContext, x1 + max(adjbw1 + 1, 0) / 2, (y1 + y2 + 1) / 2, x2 - max(adjbw2 + 1, 0) / 2, y2,
                               s, c, s2, adjbw1 / 2, adjbw2 / 2);
                    break;
                case BSLeft:
                    drawLineForBoxSide(colorSpace, graphicsContext, x1, y1 + max(-adjbw1, 0) / 2, (x1 + x2 + 1) / 2, y2 - max(-adjbw2, 0) / 2,
                               s, c, s1, adjbw1bighalf, adjbw2bighalf);
                    drawLineForBoxSide(colorSpace, graphicsContext, (x1 + x2 + 1) / 2, y1 + max(adjbw1 + 1, 0) / 2, x2, y2 - max(adjbw2 + 1, 0) / 2,
                               s, c, s2, adjbw1 / 2, adjbw2 / 2);
                    break;
                case BSBottom:
                    drawLineForBoxSide(colorSpace, graphicsContext, x1 + max(adjbw1, 0) / 2, y1, x2 - max(adjbw2, 0) / 2, (y1 + y2 + 1) / 2,
                               s, c, s2, adjbw1bighalf, adjbw2bighalf);
                    drawLineForBoxSide(colorSpace, graphicsContext, x1 + max(-adjbw1 + 1, 0) / 2, (y1 + y2 + 1) / 2, x2 - max(-adjbw2 + 1, 0) / 2, y2,
                               s, c, s1, adjbw1 / 2, adjbw2 / 2);
                    break;
                case BSRight:
                    drawLineForBoxSide(colorSpace, graphicsContext, x1, y1 + max(adjbw1, 0) / 2, (x1 + x2 + 1) / 2, y2 - max(adjbw2, 0) / 2,
                               s, c, s2, adjbw1bighalf, adjbw2bighalf);
                    drawLineForBoxSide(colorSpace, graphicsContext, (x1 + x2 + 1) / 2, y1 + max(-adjbw1 + 1, 0) / 2, x2, y2 - max(-adjbw2 + 1, 0) / 2,
                               s, c, s1, adjbw1 / 2, adjbw2 / 2);
                    break;
            }
            break;
        }
        case INSET:
            if (s == BSTop || s == BSLeft)
                c = c.dark();
            // fall through
        case OUTSET:
            if (style == OUTSET && (s == BSBottom || s == BSRight))
                c = c.dark();
            // fall through
        case SOLID: {
            graphicsContext->setStrokeStyle(NoStroke);
            graphicsContext->setFillColor(c, colorSpace);

            ASSERT(x2 >= x1);
            ASSERT(y2 >= y1);
            if (!adjbw1 && !adjbw2) {
                graphicsContext->drawRect(IntRect(x1, y1, x2 - x1, y2 - y1));
                return;
            }

            FloatPoint quad[4];
            switch (s) {
                case BSTop:
                    quad[0] = FloatPoint(x1 + max(-adjbw1, 0), y1);
                    quad[1] = FloatPoint(x1 + max(adjbw1, 0), y2);
                    quad[2] = FloatPoint(x2 - max(adjbw2, 0), y2);
                    quad[3] = FloatPoint(x2 - max(-adjbw2, 0), y1);
                    break;
                case BSBottom:
                    quad[0] = FloatPoint(x1 + max(adjbw1, 0), y1);
                    quad[1] = FloatPoint(x1 + max(-adjbw1, 0), y2);
                    quad[2] = FloatPoint(x2 - max(-adjbw2, 0), y2);
                    quad[3] = FloatPoint(x2 - max(adjbw2, 0), y1);
                    break;
                case BSLeft:
                    quad[0] = FloatPoint(x1, y1 + max(-adjbw1, 0));
                    quad[1] = FloatPoint(x1, y2 - max(-adjbw2, 0));
                    quad[2] = FloatPoint(x2, y2 - max(adjbw2, 0));
                    quad[3] = FloatPoint(x2, y1 + max(adjbw1, 0));
                    break;
                case BSRight:
                    quad[0] = FloatPoint(x1, y1 + max(adjbw1, 0));
                    quad[1] = FloatPoint(x1, y2 - max(adjbw2, 0));
                    quad[2] = FloatPoint(x2, y2 - max(-adjbw2, 0));
                    quad[3] = FloatPoint(x2, y1 + max(-adjbw1, 0));
                    break;
            }
            graphicsContext->drawConvexPolygon(4, quad);
            break;
        }
    }
}

static void clipBorderSidePolygon(GraphicsContext* graphicsContext, const IntRect& box,
  const IntSize& topLeft, const IntSize& topRight, const IntSize& bottomLeft, const IntSize& bottomRight,
  const BoxSide side, bool firstEdgeMatches, bool secondEdgeMatches,
  int borderLeftWidth, int borderRightWidth, int borderTopWidth, int borderBottomWidth
  /*const RenderStyle* style*/)
{
    FloatPoint quad[4];
    int tx = box.x();
    int ty = box.y();
    int w = box.width();
    int h = box.height();

    // For each side, create an array of FloatPoints where each point is based on whichever value in each corner
    // is larger -- the radius width/height or the border width/height -- as appropriate.
    switch (side) {
    case BSTop:
        quad[0] = FloatPoint(tx, ty);
        quad[1] = FloatPoint(
            tx + max(topLeft.width(), (int) borderLeftWidth),
            ty + max(topLeft.height(), (int) borderTopWidth));
        quad[2] = FloatPoint(
            tx + w - max(topRight.width(), (int) borderRightWidth),
            ty + max(topRight.height(), (int)borderTopWidth));
        quad[3] = FloatPoint(tx + w, ty);
        break;
    case BSLeft:
        quad[0] = FloatPoint(tx, ty);
        quad[1] = FloatPoint(
            tx + max(topLeft.width(), (int) borderLeftWidth),
            ty + max(topLeft.height(), (int) borderTopWidth));
        quad[2] = FloatPoint(
            tx + max(bottomLeft.width(), (int) borderLeftWidth),
            ty + h - max(bottomLeft.height(), (int)borderBottomWidth));
        quad[3] = FloatPoint(tx, ty + h);
        break;
    case BSBottom:
        quad[0] = FloatPoint(tx, ty + h);
        quad[1] = FloatPoint(
            tx + max(bottomLeft.width(), (int) borderLeftWidth),
            ty + h - max(bottomLeft.height(), (int)borderBottomWidth));
        quad[2] = FloatPoint(
            tx + w - max(bottomRight.width(), (int) borderRightWidth),
            ty + h - max(bottomRight.height(), (int)borderBottomWidth));
        quad[3] = FloatPoint(tx + w, ty + h);
        break;
    case BSRight:
        quad[0] = FloatPoint(tx + w, ty);
        quad[1] = FloatPoint(
            tx + w - max(topRight.width(), (int) borderRightWidth),
            ty + max(topRight.height(), (int) borderTopWidth));
        quad[2] = FloatPoint(
            tx + w - max(bottomRight.width(), (int) borderRightWidth),
            ty + h - max(bottomRight.height(), (int)borderBottomWidth));
        quad[3] = FloatPoint(tx + w, ty + h);
        break;
    default:
        break;
    }

    // If the border matches both of its adjacent sides, don't anti-alias the clip, and
    // if neither side matches, anti-alias the clip.
    if (firstEdgeMatches == secondEdgeMatches) {
        graphicsContext->clipConvexPolygon(4, quad, !firstEdgeMatches);
        return;
    }

    FloatPoint firstQuad[4];
    firstQuad[0] = quad[0];
    firstQuad[1] = quad[1];
    firstQuad[2] = side == BSTop || side == BSBottom ? FloatPoint(quad[3].x(), quad[2].y())
        : FloatPoint(quad[2].x(), quad[3].y());
    firstQuad[3] = quad[3];
    graphicsContext->clipConvexPolygon(4, firstQuad, !firstEdgeMatches);

    FloatPoint secondQuad[4];
    secondQuad[0] = quad[0];
    secondQuad[1] = side == BSTop || side == BSBottom ? FloatPoint(quad[0].x(), quad[1].y())
        : FloatPoint(quad[1].x(), quad[0].y());
    secondQuad[2] = quad[2];
    secondQuad[3] = quad[3];
    graphicsContext->clipConvexPolygon(4, secondQuad, !secondEdgeMatches);
}



static void constrainCornerRadiiForRect(const IntRect& r, IntSize& topLeft, IntSize& topRight, IntSize& bottomLeft, IntSize& bottomRight)
{
    // Constrain corner radii using CSS3 rules:
    // http://www.w3.org/TR/css3-background/#the-border-radius

    float factor = 1;
    unsigned radiiSum;

    // top
    radiiSum = static_cast<unsigned>(topLeft.width()) + static_cast<unsigned>(topRight.width()); // Casts to avoid integer overflow.
    if (radiiSum > static_cast<unsigned>(r.width()))
        factor = min(static_cast<float>(r.width()) / radiiSum, factor);

    // bottom
    radiiSum = static_cast<unsigned>(bottomLeft.width()) + static_cast<unsigned>(bottomRight.width());
    if (radiiSum > static_cast<unsigned>(r.width()))
        factor = min(static_cast<float>(r.width()) / radiiSum, factor);

    // left
    radiiSum = static_cast<unsigned>(topLeft.height()) + static_cast<unsigned>(bottomLeft.height());
    if (radiiSum > static_cast<unsigned>(r.height()))
        factor = min(static_cast<float>(r.height()) / radiiSum, factor);

    // right
    radiiSum = static_cast<unsigned>(topRight.height()) + static_cast<unsigned>(bottomRight.height());
    if (radiiSum > static_cast<unsigned>(r.height()))
        factor = min(static_cast<float>(r.height()) / radiiSum, factor);

    // Scale all radii by f if necessary.
    if (factor < 1) {
        // If either radius on a corner becomes zero, reset both radii on that corner.
        topLeft.scale(factor);
        if (!topLeft.width() || !topLeft.height())
            topLeft = IntSize();
        topRight.scale(factor);
        if (!topRight.width() || !topRight.height())
            topRight = IntSize();
        bottomLeft.scale(factor);
        if (!bottomLeft.width() || !bottomLeft.height())
            bottomLeft = IntSize();
        bottomRight.scale(factor);
        if (!bottomRight.width() || !bottomRight.height())
            bottomRight = IntSize();
    }
}

static void getBorderRadiiForRect(
  const IntRect& r,
  const litehtml::borders& borders,
  IntSize& topLeft, IntSize& topRight, IntSize& bottomLeft, IntSize& bottomRight)
{
    /*topLeft = IntSize(surround->border.topLeft().width().calcValue(r.width()), surround->border.topLeft().height().calcValue(r.height()));
    topRight = IntSize(surround->border.topRight().width().calcValue(r.width()), surround->border.topRight().height().calcValue(r.height()));

    bottomLeft = IntSize(surround->border.bottomLeft().width().calcValue(r.width()), surround->border.bottomLeft().height().calcValue(r.height()));
    bottomRight = IntSize(surround->border.bottomRight().width().calcValue(r.width()), surround->border.bottomRight().height().calcValue(r.height()));

    constrainCornerRadiiForRect(r, topLeft, topRight, bottomLeft, bottomRight);*/

    topLeft = IntSize(borders.radius.top_left_x, borders.radius.top_left_y);
    topRight = IntSize(borders.radius.top_right_x, borders.radius.top_right_y);

    bottomLeft = IntSize(borders.radius.bottom_left_x, borders.radius.bottom_left_y);
    bottomRight = IntSize(borders.radius.bottom_right_x, borders.radius.bottom_right_y);

    constrainCornerRadiiForRect(r, topLeft, topRight, bottomLeft, bottomRight);
}

static void getInnerBorderRadiiForRectWithBorderWidths(const IntRect& innerRect,
const litehtml::borders& borders,
unsigned short topWidth, unsigned short bottomWidth, unsigned short leftWidth, unsigned short rightWidth,
IntSize& innerTopLeft, IntSize& innerTopRight, IntSize& innerBottomLeft, IntSize& innerBottomRight)
//const
{
    /*innerTopLeft = IntSize(surround->border.topLeft().width().calcValue(innerRect.width()), surround->border.topLeft().height().calcValue(innerRect.height()));
    innerTopRight = IntSize(surround->border.topRight().width().calcValue(innerRect.width()), surround->border.topRight().height().calcValue(innerRect.height()));
    innerBottomLeft = IntSize(surround->border.bottomLeft().width().calcValue(innerRect.width()), surround->border.bottomLeft().height().calcValue(innerRect.height()));
    innerBottomRight = IntSize(surround->border.bottomRight().width().calcValue(innerRect.width()), surround->border.bottomRight().height().calcValue(innerRect.height()));


    innerTopLeft.setWidth(max(0, innerTopLeft.width() - leftWidth));
    innerTopLeft.setHeight(max(0, innerTopLeft.height() - topWidth));

    innerTopRight.setWidth(max(0, innerTopRight.width() - rightWidth));
    innerTopRight.setHeight(max(0, innerTopRight.height() - topWidth));

    innerBottomLeft.setWidth(max(0, innerBottomLeft.width() - leftWidth));
    innerBottomLeft.setHeight(max(0, innerBottomLeft.height() - bottomWidth));

    innerBottomRight.setWidth(max(0, innerBottomRight.width() - rightWidth));
    innerBottomRight.setHeight(max(0, innerBottomRight.height() - bottomWidth));

    constrainCornerRadiiForRect(innerRect, innerTopLeft, innerTopRight, innerBottomLeft, innerBottomRight);
*/
    innerTopLeft = IntSize(borders.radius.top_left_x, borders.radius.top_left_y);
    innerTopRight = IntSize(borders.radius.top_right_x, borders.radius.top_right_y);
    innerBottomLeft = IntSize(borders.radius.bottom_left_x, borders.radius.bottom_left_y);
    innerBottomRight = IntSize(borders.radius.bottom_right_x, borders.radius.bottom_right_y);

    innerTopLeft.setWidth(max(0, innerTopLeft.width() - leftWidth));
    innerTopLeft.setHeight(max(0, innerTopLeft.height() - topWidth));

    innerTopRight.setWidth(max(0, innerTopRight.width() - rightWidth));
    innerTopRight.setHeight(max(0, innerTopRight.height() - topWidth));

    innerBottomLeft.setWidth(max(0, innerBottomLeft.width() - leftWidth));
    innerBottomLeft.setHeight(max(0, innerBottomLeft.height() - bottomWidth));

    innerBottomRight.setWidth(max(0, innerBottomRight.width() - rightWidth));
    innerBottomRight.setHeight(max(0, innerBottomRight.height() - bottomWidth));

    constrainCornerRadiiForRect(innerRect, innerTopLeft, innerTopRight, innerBottomLeft, innerBottomRight);
}

static void drawBoxSideFromPath(GraphicsContext* graphicsContext, IntRect borderRect, Path borderPath,
                                    float thickness, float drawThickness,
                                    BoxSide s,
                                    const litehtml::borders& borders,
                                    //const RenderStyle* style,
                                    ColorSpace colorSpace,
                                    Color c, EBorderStyle borderStyle)
{
    if (thickness <= 0)
        return;

    if (borderStyle == DOUBLE && thickness < 3)
        borderStyle = SOLID;

    switch (borderStyle) {
    case BNONE:
    case BHIDDEN:
        return;
    case DOTTED:
    case DASHED: {
        graphicsContext->setStrokeColor(c, colorSpace);

        // The stroke is doubled here because the provided path is the
        // outside edge of the border so half the stroke is clipped off.
        // The extra multiplier is so that the clipping mask can antialias
        // the edges to prevent jaggies.
        graphicsContext->setStrokeThickness(drawThickness * 2 * 1.1f);
        graphicsContext->setStrokeStyle(borderStyle == DASHED ? DashedStroke : DottedStroke);

        // If the number of dashes that fit in the path is odd and non-integral then we
        // will have an awkwardly-sized dash at the end of the path. To try to avoid that
        // here, we simply make the whitespace dashes ever so slightly bigger.
        // FIXME: This could be even better if we tried to manipulate the dash offset
        // and possibly the whiteSpaceWidth to get the corners dash-symmetrical.
        float patWidth = thickness * ((borderStyle == DASHED) ? 3.0f : 1.0f);
        float whiteSpaceWidth = patWidth;
        float numberOfDashes = borderPath.length() / patWidth;
        bool evenNumberOfFullDashes = !((int)numberOfDashes % 2);
        bool integralNumberOfDashes = !(numberOfDashes - (int)numberOfDashes);
        if (!evenNumberOfFullDashes && !integralNumberOfDashes) {
            float numberOfWhitespaceDashes = numberOfDashes / 2;
            whiteSpaceWidth += (patWidth  / numberOfWhitespaceDashes);
        }

        DashArray lineDash;
        lineDash.append(patWidth);
        lineDash.append(whiteSpaceWidth);
        graphicsContext->setLineDash(lineDash, patWidth);
        graphicsContext->addPath(borderPath);
        graphicsContext->strokePath();
        return;
    }
    case DOUBLE: {
        int outerBorderTopWidth = borders.top.width / 3;
        int outerBorderRightWidth = borders.right.width / 3;
        int outerBorderBottomWidth = borders.bottom.width / 3;
        int outerBorderLeftWidth = borders.left.width / 3;

        int innerBorderTopWidth = borders.top.width * 2 / 3;
        int innerBorderRightWidth = borders.right.width * 2 / 3;
        int innerBorderBottomWidth = borders.bottom.width * 2 / 3;
        int innerBorderLeftWidth = borders.left.width * 2 / 3;

        // We need certain integer rounding results
        if (borders.top.width % 3 == 2)
            outerBorderTopWidth += 1;
        if (borders.right.width % 3 == 2)
            outerBorderRightWidth += 1;
        if (borders.bottom.width % 3 == 2)
            outerBorderBottomWidth += 1;
        if (borders.left.width % 3 == 2)
            outerBorderLeftWidth += 1;

        if (borders.top.width % 3 == 1)
            innerBorderTopWidth += 1;
        if (borders.right.width % 3 == 1)
            innerBorderRightWidth += 1;
        if (borders.bottom.width % 3 == 1)
            innerBorderBottomWidth += 1;
        if (borders.left.width % 3 == 1)
            innerBorderLeftWidth += 1;

        // Get the inner border rects for both the outer border line and the inner border line
        IntRect outerBorderInnerRect = borderInnerRect(borderRect, outerBorderTopWidth, outerBorderBottomWidth,
            outerBorderLeftWidth, outerBorderRightWidth);
        IntRect innerBorderInnerRect = borderInnerRect(borderRect, innerBorderTopWidth, innerBorderBottomWidth,
            innerBorderLeftWidth, innerBorderRightWidth);

        // Get the inner radii for the outer border line
        IntSize outerBorderTopLeftInnerRadius, outerBorderTopRightInnerRadius, outerBorderBottomLeftInnerRadius,
            outerBorderBottomRightInnerRadius;
        getInnerBorderRadiiForRectWithBorderWidths(outerBorderInnerRect, borders,
            outerBorderTopWidth, outerBorderBottomWidth,
            outerBorderLeftWidth, outerBorderRightWidth, outerBorderTopLeftInnerRadius, outerBorderTopRightInnerRadius,
            outerBorderBottomLeftInnerRadius, outerBorderBottomRightInnerRadius);

        // Get the inner radii for the inner border line
        IntSize innerBorderTopLeftInnerRadius, innerBorderTopRightInnerRadius, innerBorderBottomLeftInnerRadius,
            innerBorderBottomRightInnerRadius;
        getInnerBorderRadiiForRectWithBorderWidths(innerBorderInnerRect, borders,
            innerBorderTopWidth, innerBorderBottomWidth,
            innerBorderLeftWidth, innerBorderRightWidth, innerBorderTopLeftInnerRadius, innerBorderTopRightInnerRadius,
            innerBorderBottomLeftInnerRadius, innerBorderBottomRightInnerRadius);

        // Draw inner border line
        graphicsContext->save();
        graphicsContext->addRoundedRectClip(innerBorderInnerRect, innerBorderTopLeftInnerRadius,
            innerBorderTopRightInnerRadius, innerBorderBottomLeftInnerRadius, innerBorderBottomRightInnerRadius);
        drawBoxSideFromPath(graphicsContext, borderRect, borderPath, thickness, drawThickness, s, borders, colorSpace, c, SOLID);
        graphicsContext->restore();

        // Draw outer border line
        graphicsContext->save();
        graphicsContext->clipOutRoundedRect(outerBorderInnerRect, outerBorderTopLeftInnerRadius,
            outerBorderTopRightInnerRadius, outerBorderBottomLeftInnerRadius, outerBorderBottomRightInnerRadius);
        drawBoxSideFromPath(graphicsContext, borderRect, borderPath, thickness, drawThickness, s, borders, colorSpace, c, SOLID);
        graphicsContext->restore();

        return;
    }
    case RIDGE:
    case GROOVE:
    {
        EBorderStyle s1;
        EBorderStyle s2;
        if (borderStyle == GROOVE) {
            s1 = INSET;
            s2 = OUTSET;
        } else {
            s1 = OUTSET;
            s2 = INSET;
        }

        IntRect halfBorderRect = borderInnerRect(borderRect, borders.left.width / 2, borders.bottom.width / 2,
            borders.left.width / 2, borders.right.width / 2);

        IntSize topLeftHalfRadius, topRightHalfRadius, bottomLeftHalfRadius, bottomRightHalfRadius;
        getInnerBorderRadiiForRectWithBorderWidths(halfBorderRect, borders,
            borders.left.width / 2, borders.bottom.width / 2, borders.left.width / 2, borders.right.width / 2,
            topLeftHalfRadius, topRightHalfRadius, bottomLeftHalfRadius, bottomRightHalfRadius);

        // Paint full border
        drawBoxSideFromPath(graphicsContext, borderRect, borderPath, thickness, drawThickness, s, borders, colorSpace, c, s1);

        // Paint inner only
        graphicsContext->save();
        graphicsContext->addRoundedRectClip(halfBorderRect, topLeftHalfRadius, topRightHalfRadius,
            bottomLeftHalfRadius, bottomRightHalfRadius);
        drawBoxSideFromPath(graphicsContext, borderRect, borderPath, thickness, drawThickness, s, borders, colorSpace, c, s2);
        graphicsContext->restore();

        return;
    }
    case INSET:
        if (s == BSTop || s == BSLeft)
            c = c.dark();
        break;
    case OUTSET:
        if (s == BSBottom || s == BSRight)
            c = c.dark();
        break;
    default:
        break;
    }

    graphicsContext->setStrokeStyle(NoStroke);

    graphicsContext->setFillColor(c, colorSpace);

    graphicsContext->drawRect(borderRect);

        // TODO: css gradient
        // https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/WebCore/css/CSSGradientValue.cpp#L72
        // https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/WebCore/css/CSSParser.cpp#L4705
        ///qDebug() << "graphicsContext!";
        /*graphicsContext->save();
        //graphicsContext->setStrokeStyle(SolidStroke);
        IntPoint sliderTopLeft(borderRect.x(), borderRect.y());
        IntPoint sliderTopRight(borderRect.x()+borderRect.width(), borderRect.y()+borderRect.y());
        sliderTopRight.move(borderRect.x(), borderRect.y()+borderRect.y());
        RefPtr<Gradient> gradient = Gradient::create(sliderTopLeft, sliderTopRight);
        Color startColor(0, 0, 255, 255);// = object->style()->visitedDependentColor(CSSPropertyColor);
        Color endColor(0, 255, 0, 255);// = object->style()->visitedDependentColor(CSSPropertyColor);
        gradient->addColorStop(0.0, startColor);
        gradient->addColorStop(1.0, endColor);
        graphicsContext->setFillGradient(gradient);
        graphicsContext->setStrokeGradient(gradient);
        //graphicsContext->setStrokeStyle(NoStroke);
        graphicsContext->setStrokeGradient(gradient);
        graphicsContext->setFillGradient(gradient);
        graphicsContext->fillRect(borderRect);
        graphicsContext->restore();*/
}

static bool paintNinePieceImage(GraphicsContext* graphicsContext,
  const litehtml::borders& borders,
  bool fitToBorder, //style->borderImage() == ninePieceImage;
  int tx, int ty, int w, int h,
  //const RenderStyle* style,
  ColorSpace colorSpace,
  //const NinePieceImage& ninePieceImage,
  CompositeOperator op)
{
    QImage* img = getImage( borders.image.image_path.c_str(), borders.image.baseurl.c_str() );
    if (!img) {
      return false;
    }

    //RenderObject* clientForBackgroundImage = backgroundObject ? backgroundObject : this;
    //Image* image = bg->image(clientForBackgroundImage, tileSize);
    //bool useLowQualityScaling = false;//shouldPaintAtLowQuality(graphicsContext, image, tileSize);
    //graphicsContext->drawTiledImage(image, colorSpace, destRect, phase, tileSize, compositeOp, useLowQualityScaling);

    /*IntRect destRect;
    IntPoint phase;
    IntSize tileSize;

    calculateBackgroundImageGeometry(
      maxRootWidth, maxRootHeight,
      //bgLayer,
      offsetX, offsetY,
      bg_paint,
      tx, ty, w, h, destRect, phase, tileSize);

    IntPoint destOrigin = destRect.location();

    //destRect.intersect(paintInfo.rect);
    // don`t draw background outside of element
    intersect_clip(destRect);

    QImage imscaled = img->scaled(tileSize.width(), tileSize.height(), Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
            //graphicsContext->platformContext()->drawLine(0, 0, 400, 400);
            if (!destRect.isEmpty()) {
                phase += destRect.location() - destOrigin;
                //CompositeOperator compositeOp = op == CompositeSourceOver ? bgLayer->composite() : op;
                CompositeOperator compositeOp = op; // TODO

                // https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/WebCore/rendering/style/FillLayer.h#L67
                //StyleImage bg;  // is FillLayer->image()
                QPixmap pixmap;
                if (pixmap.convertFromImage(imscaled)) {

                  StillImage stillImage ( pixmap );
                  Image* image = &stillImage;

                  //RenderObject* clientForBackgroundImage = backgroundObject ? backgroundObject : this;
                  //Image* image = bg->image(clientForBackgroundImage, tileSize);
                  bool useLowQualityScaling = false;//shouldPaintAtLowQuality(graphicsContext, image, tileSize);
                  graphicsContext->drawTiledImage(image, colorSpace, destRect, phase, tileSize, compositeOp, useLowQualityScaling);
                } else {
                  qWarning() << "error converting image to pixmap";
                }
            }

    */

    /*StyleImage* styleImage = ninePieceImage.image();
    if (!styleImage)
        return false;

    if (!styleImage->isLoaded())
        return true; // Never paint a nine-piece image incrementally, but don't paint the fallback borders either.

    if (!styleImage->canRender(style->effectiveZoom()))
        return false;

    // FIXME: border-image is broken with full page zooming when tiling has to happen, since the tiling function
    // doesn't have any understanding of the zoom that is in effect on the tile.
    styleImage->setImageContainerSize(IntSize(w, h));
    IntSize imageSize = styleImage->imageSize(this, 1.0f);*/
    IntSize imageSize = IntSize(borders.image.image_size.width, borders.image.image_size.height);

    if (borders.image.width > 1) { // default 1
      imageSize.setWidth(borders.image.width);
    }

    //IntSize imageSize = IntSize(img->width(), img->height());

    int imageWidth = imageSize.width();
    int imageHeight = imageSize.height();

    // TODO >>>
    int topSliceCalc = /*borders.image.slice/100.0 * */imageSize.height();//borders.top.width;
    if (borders.image.slice) {
      topSliceCalc *= (double)borders.image.slice/100.0;
    }
    int bottomSliceCalc = /*borders.image.slice/100.0 * */imageSize.height();//borders.bottom.width;
    if (borders.image.slice) {
      bottomSliceCalc *= (double)borders.image.slice/100.0;
    }
    int leftSliceCalc = /*borders.image.slice/100.0 * */imageSize.width();//borders.left.width;
    if (borders.image.slice) {
      leftSliceCalc *= (double)borders.image.slice/100.0;
    }
    int rightSliceCalc = /*borders.image.slice/100.0 * */imageSize.width();//borders.right.width;
    if (borders.image.slice) {
      rightSliceCalc *= (double)borders.image.slice/100.0;
    }
    //int topSliceCalc =  borders.top.width;
    //int bottomSliceCalc = borders.bottom.width;
    //int leftSliceCalc = borders.left.width;
    //int rightSliceCalc = borders.right.width;

    int topSlice = std::min(imageSize.height(), topSliceCalc);//min(imageHeight, ninePieceImage.slices().top().calcValue(imageHeight));
    int bottomSlice = std::min(imageSize.height(), bottomSliceCalc);//min(imageHeight, ninePieceImage.slices().bottom().calcValue(imageHeight));
    int leftSlice = std::min(imageSize.width(), leftSliceCalc);//min(imageWidth, ninePieceImage.slices().left().calcValue(imageWidth));
    int rightSlice = std::min(imageSize.width(), rightSliceCalc);//min(imageWidth, ninePieceImage.slices().right().calcValue(imageWidth));

    //ENinePieceImageRule hRule = ninePieceImage.horizontalRule();
    //ENinePieceImageRule vRule = ninePieceImage.verticalRule();
    // TODO
    Image::TileRule hRule = Image::TileRule::StretchTile;
    //borders.image.repeat == litehtml::background_repeat::background_repeat_repeat ? Image::TileRule::RoundTile : Image::TileRule::StretchTile;
    Image::TileRule vRule = Image::TileRule::StretchTile;

    /*if(!borders.image.css_prop.empty()) {
      //qDebug() << "drawLeft borders.left.width" << borders.left.width;
      //qDebug() << "drawLeft borders.top.width" << borders.top.width;
      //qDebug() << "drawLeft borders.right.width" << borders.right.width;
      //qDebug() << "drawLeft borders.bottom.width" << borders.bottom.width;
      qDebug() << "borders.image.css_prop" << borders.image.css_prop.c_str();
      qDebug() << "borders.image.slice" << borders.image.slice;
      qDebug() << "borders.image.width" << borders.image.width;
      qDebug() << "borders.image.repeat" << borders.image.repeat;
      qDebug() << "borders.image baseurl" << borders.image.baseurl.c_str();
      qDebug() << "borders.image image_path" << borders.image.image_path.c_str();
      qDebug() << "borders.image image_size" << borders.image.image_size.width  << borders.image.image_size.height;
    }*/

    int leftWidth = fitToBorder ? borders.left.width : leftSlice; //fitToBorder ? style->borderLeftWidth() : leftSlice;
    int topWidth = fitToBorder ? borders.top.width : topSlice; //fitToBorder ? style->borderTopWidth() : topSlice;
    int rightWidth = fitToBorder ? borders.right.width : rightSlice; //fitToBorder ? style->borderRightWidth() : rightSlice;
    int bottomWidth = fitToBorder ? borders.bottom.width : bottomSlice; //fitToBorder ? style->borderBottomWidth() : bottomSlice;

    bool drawLeft = leftSlice > 0 && leftWidth > 0;
    bool drawTop = topSlice > 0 && topWidth > 0;
    bool drawRight = rightSlice > 0 && rightWidth > 0;
    bool drawBottom = bottomSlice > 0 && bottomWidth > 0;
    bool drawMiddle = (imageWidth - leftSlice - rightSlice) > 0 && (w - leftWidth - rightWidth) > 0 &&
                      (imageHeight - topSlice - bottomSlice) > 0 && (h - topWidth - bottomWidth) > 0;



    //Image* image = styleImage->image(this, imageSize);
    //ColorSpace colorSpace = style->colorSpace();

    if (drawLeft) {
        // Paint the top and bottom left corners.

        // The top left corner rect is (tx, ty, leftWidth, topWidth)
        // The rect to use from within the image is obtained from our slice, and is (0, 0, leftSlice, topSlice)
        if (drawTop)
        {
          IntRect tileSize = IntRect(0, 0, imageWidth, imageHeight);
          QImage imscaledLeft = img->scaled(tileSize.width(), tileSize.height(), Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
          QPixmap pixmap;
          if (!pixmap.convertFromImage(imscaledLeft)) {
            qWarning() << "error converting image to pixmap";
            return false;
          }
          StillImage stillImage ( pixmap );
          Image* image = &stillImage;
          // Have to scale and tile into the border rect.
          graphicsContext->drawImage(image, colorSpace, IntRect(tx, ty, leftWidth, topWidth),
                                       IntRect(0, 0, leftSlice, topSlice), op);
        }

        // The bottom left corner rect is (tx, ty + h - bottomWidth, leftWidth, bottomWidth)
        // The rect to use from within the image is (0, imageHeight - bottomSlice, leftSlice, botomSlice)
        if (drawBottom)
        {
          IntRect tileSize = IntRect(0, 0, imageWidth, imageHeight);
          QImage imscaledLeft = img->scaled(tileSize.width(), tileSize.height(), Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
          QPixmap pixmap;
          if (!pixmap.convertFromImage(imscaledLeft)) {
            qWarning() << "error converting image to pixmap";
            return false;
          }
          StillImage stillImage ( pixmap );
          Image* image = &stillImage;
          // Have to scale and tile into the border rect.
          graphicsContext->drawImage(image, colorSpace, IntRect(tx, ty + h - bottomWidth, leftWidth, bottomWidth),
                                       IntRect(0, imageHeight - bottomSlice, leftSlice, bottomSlice), op);

        }

        // Paint the left edge.
        {
          IntRect tileSize = IntRect(0, 0, imageWidth, imageHeight);
          QImage imscaledLeft = img->scaled(tileSize.width(), tileSize.height(), Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
          QPixmap pixmap;
          if (!pixmap.convertFromImage(imscaledLeft)) {
            qWarning() << "error converting image to pixmap";
            return false;
          }
          StillImage stillImage ( pixmap );
          Image* image = &stillImage;
          // Have to scale and tile into the border rect.
          graphicsContext->drawTiledImage(image, colorSpace, IntRect(tx, ty + topWidth, leftWidth,
                                          h - topWidth - bottomWidth),
                                          IntRect(0, topSlice, leftSlice, imageHeight - topSlice - bottomSlice),
                                          Image::StretchTile, (Image::TileRule)vRule, op);
        }
    }

    if (drawRight) {
        // Paint the top and bottom right corners
        // The top right corner rect is (tx + w - rightWidth, ty, rightWidth, topWidth)
        // The rect to use from within the image is obtained from our slice, and is (imageWidth - rightSlice, 0, rightSlice, topSlice)
        if (drawTop)
        {
          IntRect tileSize = IntRect(0, 0, imageWidth, imageHeight);
          QImage imscaledLeft = img->scaled(tileSize.width(), tileSize.height(), Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
          QPixmap pixmap;
          if (!pixmap.convertFromImage(imscaledLeft)) {
            qWarning() << "error converting image to pixmap";
            return false;
          }
          StillImage stillImage ( pixmap );
          Image* image = &stillImage;
          // Have to scale and tile into the border rect.
          graphicsContext->drawImage(image, colorSpace, IntRect(tx + w - rightWidth, ty, rightWidth, topWidth),
                                     IntRect(imageWidth - rightSlice, 0, rightSlice, topSlice), op);
        }

        // The bottom right corner rect is (tx + w - rightWidth, ty + h - bottomWidth, rightWidth, bottomWidth)
        // The rect to use from within the image is (imageWidth - rightSlice, imageHeight - bottomSlice, rightSlice, bottomSlice)
        if (drawBottom)
        {
          IntRect tileSize = IntRect(0, 0, imageWidth, imageHeight);
          QImage imscaledLeft = img->scaled(tileSize.width(), tileSize.height(), Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
          QPixmap pixmap;
          if (!pixmap.convertFromImage(imscaledLeft)) {
            qWarning() << "error converting image to pixmap";
            return false;
          }
          StillImage stillImage ( pixmap );
          Image* image = &stillImage;
          // Have to scale and tile into the border rect.
          graphicsContext->drawImage(image, colorSpace, IntRect(tx + w - rightWidth, ty + h - bottomWidth, rightWidth, bottomWidth),
                                     IntRect(imageWidth - rightSlice, imageHeight - bottomSlice, rightSlice, bottomSlice), op);
        }
        // Paint the right edge.
        {
          /*IntRect tileSize = IntRect(tx + w - rightWidth, ty + topWidth, rightWidth,
                                          h - topWidth - bottomWidth);*/
          IntRect tileSize = IntRect(0, 0, imageWidth, imageHeight);
          QImage imscaledLeft = img->scaled(tileSize.width(), tileSize.height(), Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
          QPixmap pixmap;
          if (!pixmap.convertFromImage(imscaledLeft)) {
            qWarning() << "error converting image to pixmap";
            return false;
          }
          StillImage stillImage ( pixmap );
          Image* image = &stillImage;
          // Have to scale and tile into the border rect.
          graphicsContext->drawTiledImage(image, colorSpace, IntRect(tx + w - rightWidth, ty + topWidth, rightWidth,
                                          h - topWidth - bottomWidth),
                                          IntRect(imageWidth - rightSlice, topSlice, rightSlice, imageHeight - topSlice - bottomSlice),
                                          Image::StretchTile, (Image::TileRule)vRule, op);
        }
    }

    // Paint the top edge.
    if (drawTop)
    {
      IntRect tileSize = IntRect(0, 0, imageWidth, imageHeight);
      QImage imscaledLeft = img->scaled(tileSize.width(), tileSize.height(), Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
      QPixmap pixmap;
      if (!pixmap.convertFromImage(imscaledLeft)) {
        qWarning() << "error converting image to pixmap";
        return false;
      }
      StillImage stillImage ( pixmap );
      Image* image = &stillImage;
      // Have to scale and tile into the border rect.
      graphicsContext->drawTiledImage(image, colorSpace, IntRect(tx + leftWidth, ty, w - leftWidth - rightWidth, topWidth),
                                      IntRect(leftSlice, 0, imageWidth - rightSlice - leftSlice, topSlice),
                                      (Image::TileRule)hRule, Image::StretchTile, op);
    }

    // Paint the bottom edge.
    if (drawBottom)
    {
      IntRect tileSize = IntRect(0, 0, imageWidth, imageHeight);
      QImage imscaledLeft = img->scaled(tileSize.width(), tileSize.height(), Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
      QPixmap pixmap;
      if (!pixmap.convertFromImage(imscaledLeft)) {
        qWarning() << "error converting image to pixmap";
        return false;
      }
      StillImage stillImage ( pixmap );
      Image* image = &stillImage;
      // Have to scale and tile into the border rect.
      graphicsContext->drawTiledImage(image, colorSpace, IntRect(tx + leftWidth, ty + h - bottomWidth,
                                      w - leftWidth - rightWidth, bottomWidth),
                                      IntRect(leftSlice, imageHeight - bottomSlice, imageWidth - rightSlice - leftSlice, bottomSlice),
                                      (Image::TileRule)hRule, Image::StretchTile, op);
    }
    // Paint the middle.
    if (drawMiddle)
    {
      IntRect tileSize = IntRect(0, 0, imageWidth, imageHeight);
      QImage imscaledLeft = img->scaled(tileSize.width(), tileSize.height(), Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
      QPixmap pixmap;
      if (!pixmap.convertFromImage(imscaledLeft)) {
        qWarning() << "error converting image to pixmap";
        return false;
      }
      StillImage stillImage ( pixmap );
      Image* image = &stillImage;
      // Have to scale and tile into the border rect.
      graphicsContext->drawTiledImage(image, colorSpace, IntRect(tx + leftWidth, ty + topWidth, w - leftWidth - rightWidth,
                                      h - topWidth - bottomWidth),
                                      IntRect(leftSlice, topSlice, imageWidth - rightSlice - leftSlice, imageHeight - topSlice - bottomSlice),
                                      (Image::TileRule)hRule, (Image::TileRule)vRule, op);
    }
    return true;
}

static void paintBorder(
  ColorSpace colorSpace,
  const Color& topColor,  const Color& bottomColor,
  const Color& leftColor, const Color& rightColor,
  EBorderStyle topStyle,
  EBorderStyle bottomStyle,
  EBorderStyle leftStyle,
  EBorderStyle rightStyle,
  IntSize topLeftRadius, IntSize topRightRadius, IntSize bottomLeftRadius, IntSize bottomRightRadius,
  IntRect innerBorderRect,
  IntSize innerTopLeftRadius, IntSize innerTopRightRadius, IntSize innerBottomLeftRadius, IntSize innerBottomRightRadius,
  const litehtml::borders& borders,
  bool topTransparent, bool bottomTransparent, bool rightTransparent, bool leftTransparent,
  bool renderTop, bool renderLeft, bool renderRight, bool renderBottom,
  bool hasBorderRadius,
  GraphicsContext* graphicsContext,
  int tx, int ty, int w, int h,
  CompositeOperator op,
  bool begin = true, bool end = true)
{
    // TODO
    // https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/WebCore/rendering/RenderBoxModelObject.cpp#L880
    if (paintNinePieceImage(graphicsContext, borders,
      true,
      tx, ty, w, h,
      colorSpace, op)) {
      //style, borderImage()))
      return;
    }

    if (graphicsContext->paintingDisabled())
        return;

    bool renderRadii = false;
    Path roundedPath;
    IntSize topLeft, topRight, bottomLeft, bottomRight;
    IntRect borderRect(tx, ty, w, h);

    if (hasBorderRadius) {
        if (begin) {
            topLeft = topLeftRadius;
            bottomLeft = bottomLeftRadius;
        }
        if (end) {
            topRight = topRightRadius;
            bottomRight = bottomRightRadius;
        }

        renderRadii = true;

        // Clip to the inner and outer radii rects.
        graphicsContext->save();
        graphicsContext->addRoundedRectClip(borderRect, topLeft, topRight, bottomLeft, bottomRight);
        graphicsContext->clipOutRoundedRect(innerBorderRect, innerTopLeftRadius, innerTopRightRadius, innerBottomLeftRadius, innerBottomRightRadius);

        roundedPath.addRoundedRect(borderRect, topLeft, topRight, bottomLeft, bottomRight);
        graphicsContext->addPath(roundedPath);
    }

    bool upperLeftBorderStylesMatch = renderLeft && (topStyle == leftStyle) && (topColor == leftColor);
    bool upperRightBorderStylesMatch = renderRight && (topStyle == rightStyle) && (topColor == rightColor) && (topStyle != OUTSET) && (topStyle != RIDGE) && (topStyle != INSET) && (topStyle != GROOVE);
    bool lowerLeftBorderStylesMatch = renderLeft && (bottomStyle == leftStyle) && (bottomColor == leftColor) && (bottomStyle != OUTSET) && (bottomStyle != RIDGE) && (bottomStyle != INSET) && (bottomStyle != GROOVE);
    bool lowerRightBorderStylesMatch = renderRight && (bottomStyle == rightStyle) && (bottomColor == rightColor);

    if (renderTop) {
        int x = tx;
        int x2 = tx + w;

        if (renderRadii && borderWillArcInnerEdge(topLeft, topRight, borders.left.width, borders.right.width, borders.top.width)) {
            graphicsContext->save();
            clipBorderSidePolygon(graphicsContext, borderRect, topLeft, topRight, bottomLeft, bottomRight, BSTop, upperLeftBorderStylesMatch, upperRightBorderStylesMatch, borders.left.width, borders.right.width, borders.top.width, borders.bottom.width);
            float thickness = max(max(borders.top.width, borders.left.width), borders.right.width);
            drawBoxSideFromPath(graphicsContext, borderRect, roundedPath, borders.top.width, thickness, BSTop, borders, colorSpace, topColor, topStyle);
            graphicsContext->restore();
        } else {
            bool ignoreLeft = (topColor == leftColor && topTransparent == leftTransparent && topStyle >= OUTSET
                && (leftStyle == DOTTED || leftStyle == DASHED || leftStyle == SOLID || leftStyle == OUTSET));
            bool ignoreRight = (topColor == rightColor && topTransparent == rightTransparent && topStyle >= OUTSET
                && (rightStyle == DOTTED || rightStyle == DASHED || rightStyle == SOLID || rightStyle == INSET));

            drawLineForBoxSide(colorSpace, graphicsContext, x, ty, x2, ty + borders.top.width, BSTop, topColor, topStyle,
                    ignoreLeft ? 0 : borders.left.width, ignoreRight ? 0 : borders.right.width);
        }
    }

    if (renderBottom) {
        int x = tx;
        int x2 = tx + w;

        if (renderRadii && borderWillArcInnerEdge(bottomLeft, bottomRight, borders.left.width, borders.right.width, borders.bottom.width)) {
            graphicsContext->save();
            clipBorderSidePolygon(graphicsContext, borderRect, topLeft, topRight, bottomLeft, bottomRight, BSBottom, lowerLeftBorderStylesMatch, lowerRightBorderStylesMatch, borders.left.width, borders.right.width, borders.top.width, borders.bottom.width);
            float thickness = max(max(borders.bottom.width, borders.left.width), borders.right.width);
            drawBoxSideFromPath(graphicsContext, borderRect, roundedPath, borders.bottom.width, thickness, BSBottom, borders, colorSpace, bottomColor, bottomStyle);
            graphicsContext->restore();
        } else {
            bool ignoreLeft = (bottomColor == leftColor && bottomTransparent == leftTransparent && bottomStyle >= OUTSET
                && (leftStyle == DOTTED || leftStyle == DASHED || leftStyle == SOLID || leftStyle == OUTSET));

            bool ignoreRight = (bottomColor == rightColor && bottomTransparent == rightTransparent && bottomStyle >= OUTSET
                && (rightStyle == DOTTED || rightStyle == DASHED || rightStyle == SOLID || rightStyle == INSET));

            drawLineForBoxSide(colorSpace, graphicsContext, x, ty + h - borders.bottom.width, x2, ty + h, BSBottom, bottomColor,
                        bottomStyle, ignoreLeft ? 0 : borders.left.width,
                        ignoreRight ? 0 : borders.right.width);
        }
    }

    if (renderLeft) {
        int y = ty;
        int y2 = ty + h;

        if (renderRadii && borderWillArcInnerEdge(bottomLeft, topLeft, borders.bottom.width, borders.top.width, borders.left.width)) {
            graphicsContext->save();
            clipBorderSidePolygon(graphicsContext, borderRect, topLeft, topRight, bottomLeft, bottomRight, BSLeft, upperLeftBorderStylesMatch, lowerLeftBorderStylesMatch, borders.left.width, borders.right.width, borders.top.width, borders.bottom.width);
            float thickness = max(max(borders.left.width, borders.top.width), borders.bottom.width);
            drawBoxSideFromPath(graphicsContext, borderRect, roundedPath, borders.left.width, thickness, BSLeft, borders, colorSpace, leftColor, leftStyle);
            graphicsContext->restore();
        } else {
            bool ignoreTop = (topColor == leftColor && topTransparent == leftTransparent && leftStyle >= OUTSET
                && (topStyle == DOTTED || topStyle == DASHED || topStyle == SOLID || topStyle == OUTSET));

            bool ignoreBottom = (bottomColor == leftColor && bottomTransparent == leftTransparent && leftStyle >= OUTSET
                && (bottomStyle == DOTTED || bottomStyle == DASHED || bottomStyle == SOLID || bottomStyle == INSET));

            drawLineForBoxSide(colorSpace, graphicsContext, tx, y, tx + borders.left.width, y2, BSLeft, leftColor,
                        leftStyle, ignoreTop ? 0 : borders.top.width, ignoreBottom ? 0 : borders.bottom.width);
        }
    }

    if (renderRight) {
        if (renderRadii && borderWillArcInnerEdge(bottomRight, topRight, borders.bottom.width, borders.top.width, borders.right.width)) {
            graphicsContext->save();
            clipBorderSidePolygon(graphicsContext, borderRect, topLeft, topRight, bottomLeft, bottomRight, BSRight, upperRightBorderStylesMatch, lowerRightBorderStylesMatch, borders.left.width, borders.right.width, borders.top.width, borders.bottom.width);
            float thickness = max(max(borders.right.width, borders.top.width), borders.bottom.width);
            drawBoxSideFromPath(graphicsContext, borderRect, roundedPath, borders.right.width, thickness, BSRight, borders, colorSpace, rightColor, rightStyle);
            graphicsContext->restore();
        } else {
            bool ignoreTop = ((topColor == rightColor) && (topTransparent == rightTransparent)
                && (rightStyle >= DOTTED || rightStyle == INSET)
                && (topStyle == DOTTED || topStyle == DASHED || topStyle == SOLID || topStyle == OUTSET));

            bool ignoreBottom = ((bottomColor == rightColor) && (bottomTransparent == rightTransparent)
                && (rightStyle >= DOTTED || rightStyle == INSET)
                && (bottomStyle == DOTTED || bottomStyle == DASHED || bottomStyle == SOLID || bottomStyle == INSET));

            int y = ty;
            int y2 = ty + h;

            drawLineForBoxSide(colorSpace, graphicsContext, tx + w - borders.right.width, y, tx + w, y2, BSRight, rightColor,
                rightStyle, ignoreTop ? 0 : borders.top.width,
                ignoreBottom ? 0 : borders.bottom.width);
        }
    }

    if (renderRadii)
        graphicsContext->restore();
}

EBorderStyle toEBorderStyle(const litehtml::border& border) {
  switch (border.style) {
    case litehtml::border_style::border_style_none:
      return EBorderStyle::BNONE;
    case litehtml::border_style::border_style_inset:
      return EBorderStyle::INSET;
    case litehtml::border_style::border_style_ridge:
      return EBorderStyle::RIDGE;
    case litehtml::border_style::border_style_solid:
      return EBorderStyle::SOLID;
    case litehtml::border_style::border_style_dashed:
      return EBorderStyle::DASHED;
    case litehtml::border_style::border_style_dotted:
      return EBorderStyle::DOTTED;
    case litehtml::border_style::border_style_double:
      return EBorderStyle::DOUBLE;
    case litehtml::border_style::border_style_groove:
      return EBorderStyle::GROOVE;
    case litehtml::border_style::border_style_hidden:
      return EBorderStyle::BHIDDEN;
    case litehtml::border_style::border_style_outset:
      return EBorderStyle::OUTSET;
  }
}

// https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/WebCore/rendering/RenderBoxModelObject.cpp#L1002
void container_qt5::draw_borders(litehtml::uint_ptr hdc, const litehtml::borders& borders, const litehtml::position& draw_pos, bool root)
{
    if (root) {
      // we are in <html>, so no borders here
      return;
    }

    // borders.top.image // TODO: add border-image to litehtml

    GraphicsContext* graphicsContext = (GraphicsContext*) hdc;
    if(!graphicsContext) {
      return;
    }
    QPainter *painter = (QPainter *) graphicsContext->platformContext();
    if(!painter) {
      return;
    }

    bool topTransparent = borders.top.color.alpha > 0;
    bool bottomTransparent = borders.bottom.color.alpha > 0;
    bool rightTransparent = borders.right.color.alpha > 0;
    bool leftTransparent = borders.left.color.alpha > 0;

    bool renderTop = borders.top.style > litehtml::border_style_hidden;
    bool renderLeft = borders.left.style > litehtml::border_style_hidden;
    bool renderRight = borders.right.style > litehtml::border_style_hidden;
    bool renderBottom = borders.bottom.style > litehtml::border_style_hidden;

    const Color& topColor = toColor(borders.top.color);
    const Color& bottomColor = toColor(borders.bottom.color);
    const Color& leftColor = toColor(borders.left.color);
    const Color& rightColor = toColor(borders.right.color);

    QRect  borderArea = getRect(draw_pos);

    IntRect borderRect(borderArea.x(), borderArea.y(), borderArea.width(), borderArea.height());

    IntSize topLeftRadius, topRightRadius, bottomLeftRadius, bottomRightRadius;
    getBorderRadiiForRect(borderRect, borders, topLeftRadius, topRightRadius, bottomLeftRadius, bottomRightRadius);

    IntRect innerBorderRect = borderInnerRect(borderRect, borders.top.width, borders.bottom.width,
              borders.left.width, borders.right.width);

    IntSize innerTopLeftRadius, innerTopRightRadius, innerBottomLeftRadius, innerBottomRightRadius;

    getInnerBorderRadiiForRectWithBorderWidths(innerBorderRect, borders,
      borders.top.width, borders.bottom.width, borders.left.width, borders.right.width,
      innerTopLeftRadius, innerTopRightRadius,
              innerBottomLeftRadius, innerBottomRightRadius);

    bool hasBorderRadius = true;

    graphicsContext->save();

    painter->setRenderHint(QPainter::Antialiasing);

    apply_clip( painter );

    paintBorder(sRGBColorSpace,
      topColor, bottomColor, leftColor, rightColor,
      toEBorderStyle(borders.top),
      toEBorderStyle(borders.bottom),
      toEBorderStyle(borders.left),
      toEBorderStyle(borders.right),
      topLeftRadius, topRightRadius, bottomLeftRadius, bottomRightRadius, // <<<
      innerBorderRect,
      innerTopLeftRadius, innerTopRightRadius, innerBottomLeftRadius, innerBottomRightRadius,
      borders,
      topTransparent, bottomTransparent, rightTransparent, leftTransparent,
      renderTop, renderLeft, renderRight, renderBottom,
      hasBorderRadius,
      graphicsContext,
      draw_pos.x, draw_pos.y, draw_pos.width, draw_pos.height,
        //CompositeClear,
        //CompositeCopy,
        CompositeSourceOver,
        //CompositeSourceIn,
        //CompositeSourceOut,
        //CompositeSourceAtop,
        //CompositeDestinationOver,
        //CompositeDestinationIn,
        //CompositeDestinationOut,
        //CompositeDestinationAtop,
        //CompositeXOR,
        //CompositePlusDarker,
        //CompositeHighlight,
        //CompositePlusLighter,
      true, true
    );

      // TODO: css gradient
      // https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/WebCore/css/CSSGradientValue.cpp#L72
        /*        graphicsContext->save();
    //graphicsContext->setStrokeStyle(SolidStroke);
    IntPoint sliderTopLeft(0, 0);
    IntPoint sliderTopRight(400, 400);
    sliderTopRight.move(0, 400);
    RefPtr<Gradient> gradient = Gradient::create(sliderTopLeft, sliderTopRight);
    Color startColor(0, 0, 255, 255);// = object->style()->visitedDependentColor(CSSPropertyColor);
    Color endColor(0, 255, 0, 255);// = object->style()->visitedDependentColor(CSSPropertyColor);
    gradient->addColorStop(0.0, startColor);
    gradient->addColorStop(1.0, endColor);
    graphicsContext->setFillGradient(gradient);
    graphicsContext->setStrokeGradient(gradient);

        //graphicsContext->setStrokeStyle(NoStroke);
        graphicsContext->setStrokeGradient(gradient);
        graphicsContext->setFillGradient(gradient);
        graphicsContext->fillRect(IntRect(0,0, 400, 400));
        graphicsContext->restore();*/




    graphicsContext->restore();
}

IntRect static toIntRect(const litehtml::position& pos) {
  return IntRect( pos.x, pos.y, pos.width, pos.height);
}

static void calculateBackgroundImageGeometry(
  //const FillLayer* fillLayer,
  int maxRootWidth, int maxRootHeight,
  int offsetX, int offsetY,
  const litehtml::background_paint& bg_paint,
  int tx, int ty, int w, int h,
  IntRect& destRect, IntPoint& phase, IntSize& tileSize)
{
    IntRect origin_box = toIntRect( bg_paint.origin_box );
    IntRect border_box = toIntRect( bg_paint.border_box );
    IntRect clip_box = toIntRect( bg_paint.clip_box );

    int left = 0;
    int top = 0;
    IntSize positioningAreaSize;

    // Determine the background positioning area and set destRect to the background painting area.
    // destRect will be adjusted later if the background is non-repeating.
    bool fixedAttachment = bg_paint.attachment == litehtml::background_attachment::background_attachment_fixed;//fillLayer->attachment() == FixedBackgroundAttachment;

/*#if ENABLE(FAST_MOBILE_SCROLLING)
    if (view()->frameView() && view()->frameView()->canBlitOnScroll()) {
        // As a side effect of an optimization to blit on scroll, we do not honor the CSS
        // property "background-attachment: fixed" because it may result in rendering
        // artifacts. Note, these artifacts only appear if we are blitting on scroll of
        // a page that has fixed background images.
        fixedAttachment = false;
    }
#endif*/

    if (!fixedAttachment) {
        destRect = IntRect(tx, ty, w, h);
        //positioningAreaSize = destRect.size();

        int right = 0;
        int bottom = 0;
        // Scroll and Local.
        //if (fillLayer->origin() != BorderFillBox) {
        if (origin_box != border_box) {
            // TODO >>>>>>>>>>>
            left = 0;// bg_paint.border_box.left();//borderLeft();
            right = 0;//bg_paint.border_box.right();
            top = 0;//bg_paint.border_box.top();
            bottom = 0;//bg_paint.border_box.bottom();
            //if (origin_box == ContentFillBox) {
            if (origin_box == clip_box) {
                // TODO >>>>>>>>>>>
                left += 0;//bg_paint.origin_box.left();//paddingLeft();
                right +=  0;//bg_paint.origin_box.right();//paddingRight();
                top +=  0;//bg_paint.origin_box.top();//paddingTop();
                bottom +=  0;//bg_paint.origin_box.bottom();//paddingBottom();
            }
        }

        // The background of the box generated by the root element covers the entire canvas including
        // its margins. Since those were added in already, we have to factor them out when computing
        // the background positioning area.
        if (bg_paint.is_root) {
            // TODO >>>>>>>>>>>

            //positioningAreaSize = IntSize(toRenderBox(this)->width() - left - right, toRenderBox(this)->height() - top - bottom);
            positioningAreaSize = IntSize(maxRootWidth - left - right, maxRootHeight  - top - bottom);
            // TODO >>>>>>>>>>>
            left += 0;//bg_paint.origin_box.left();//marginLeft();
            top += 0;//bg_paint.origin_box.top();;
        } else
            positioningAreaSize = IntSize(w - left - right, h - top - bottom);

    } else {
        destRect = clip_box;//viewRect();
        positioningAreaSize = destRect.size();
    }

    // TODO >>>>>>>>>>>
    tileSize = IntSize(bg_paint.image_size.width, bg_paint.image_size.height);
    //tileSize = calculateFillTileSize(fillLayer, positioningAreaSize);

    //EFillRepeat backgroundRepeatX = fillLayer->repeatX();
    //EFillRepeat backgroundRepeatY = fillLayer->repeatY();

    //int xPosition = fillLayer->xPosition().calcMinValue(positioningAreaSize.width() - tileSize.width(), true);
    //int xPosition = std::min(bg_paint.position_x, positioningAreaSize.width() - tileSize.width());
    //int xPosition = std::min(/*offsetX+*/bg_paint.position_x+clip_box.x(), positioningAreaSize.width() - tileSize.width());
    int xPosition = 0; // TODO
    //if (backgroundRepeatX == RepeatFill)
    if (bg_paint.repeat == litehtml::background_repeat_repeat_x
          || bg_paint.repeat == litehtml::background_repeat_repeat)
        phase.setX(tileSize.width() ? tileSize.width() - (xPosition + left) % tileSize.width() : 0);
    else {
        destRect.move(max(xPosition + left, 0), 0);
        phase.setX(-min(xPosition + left, 0));
        destRect.setWidth(tileSize.width() + min(xPosition + left, 0));
    }

    //int yPosition = fillLayer->yPosition().calcMinValue(positioningAreaSize.height() - tileSize.height(), true);
    //int yPosition = std::min(bg_paint.position_y, positioningAreaSize.height() - tileSize.height());
    //int yPosition = std::min(/*offsetY+*/bg_paint.position_y+clip_box.y(), positioningAreaSize.height() - tileSize.height());
    int yPosition = 0; // TODO

    //if (backgroundRepeatY == RepeatFill)
    if (bg_paint.repeat == litehtml::background_repeat_repeat_y
          || bg_paint.repeat == litehtml::background_repeat_repeat)
        phase.setY(tileSize.height() ? tileSize.height() - (yPosition + top) % tileSize.height() : 0);
    else {
        destRect.move(0, max(yPosition + top, 0));
        phase.setY(-min(yPosition + top, 0));
        destRect.setHeight(tileSize.height() + min(yPosition + top, 0));
    }

    if (fixedAttachment)
        phase.move(max(tx - destRect.x(), 0), max(ty - destRect.y(), 0));

    destRect.intersect(IntRect(tx, ty, w, h));
}

static void paintFillLayerExtended(
  int maxRootWidth, int maxRootHeight,
  GraphicsContext* graphicsContext, const Color& c
  //, const FillLayer* bgLayer
  , bool hasBorderRadius
  , int offsetX, int offsetY
  , const litehtml::background_paint& bg_paint
  , ColorSpace colorSpace
  //, int tx, int ty, int w, int h, InlineFlowBox* box,
  , CompositeOperator op
  //, RenderObject* backgroundObject
)
{
    /*QPoint clip_a(bg_paint.clip_box.left(), bg_paint.clip_box.top());
    QPoint clip_b(bg_paint.clip_box.right(), bg_paint.clip_box.bottom());*/

    /*IntRect origin_box = toIntRect( bg_paint.origin_box );
    IntRect border_box = toIntRect( bg_paint.border_box );
    IntRect clip_box = toIntRect( bg_paint.clip_box );*/

    if (graphicsContext->paintingDisabled())
        return;

    // /// \note with offsetX
    int tx = /*offsetX+*/bg_paint.clip_box.left();
    //qDebug() << bg.position_x << bg.clip_box.left();

    // /// \note with offsetY
    int ty = /*offsetY+*/bg_paint.clip_box.top();

    int w = bg_paint.clip_box.width;
    int h = bg_paint.clip_box.height;

    bool includeLeftEdge = true;//box ? box->includeLogicalLeftEdge() : true;
    bool includeRightEdge = true;//box ? box->includeLogicalRightEdge() : true;
    int bLeft =  bg_paint.border_box.left();//includeLeftEdge ? borderLeft() : 0;
    int bRight = bg_paint.border_box.right();//includeRightEdge ? borderRight() : 0;
    int pLeft = bg_paint.origin_box.left();//includeLeftEdge ? paddingLeft() : 0;
    int pRight = bg_paint.origin_box.right();//includeRightEdge ? paddingRight() : 0;

    /*if(bg_paint.is_root) {
          tx = 0;
          ty = 0;
          w = 5000;//maxRootWidth;
          h = 5000;//maxRootHeight;
          qDebug() << "isRoot" << bg_paint.color.red;

        IntRect rect(tx, ty,
          w, h);
          graphicsContext->fillRect(rect, c, colorSpace);
        } else {
        return;
        }*/

   bool clippedToBorderRadius = false;
   if (hasBorderRadius) {
        IntRect borderRect(tx, ty,
          w, h);

        if (borderRect.isEmpty())
            return;

        graphicsContext->save();

        litehtml::borders border_radiuses;
        border_radiuses.radius.top_left_x = bg_paint.border_radius.top_left_x;
        border_radiuses.radius.top_left_y = bg_paint.border_radius.top_left_y;
        border_radiuses.radius.top_right_x = bg_paint.border_radius.top_right_x;
        border_radiuses.radius.top_right_y = bg_paint.border_radius.top_right_y;
        border_radiuses.radius.bottom_left_x = bg_paint.border_radius.bottom_left_x;
        border_radiuses.radius.bottom_left_y = bg_paint.border_radius.bottom_left_y;
        border_radiuses.radius.bottom_right_x = bg_paint.border_radius.bottom_right_x;
        border_radiuses.radius.bottom_right_y = bg_paint.border_radius.bottom_right_y;

        IntSize topLeft, topRight, bottomLeft, bottomRight;
        getBorderRadiiForRect(borderRect,
          border_radiuses,
          topLeft, topRight, bottomLeft, bottomRight);

        graphicsContext->addRoundedRectClip(borderRect, includeLeftEdge ? topLeft : IntSize(),
                                                includeRightEdge ? topRight : IntSize(),
                                                includeLeftEdge ? bottomLeft : IntSize(),
                                                includeRightEdge ? bottomRight : IntSize());
        clippedToBorderRadius = true;
    }

    bool hasOverflowClip = w > 0 && h > 0;
    // TODO: add local background-attachment to litehtml
    // see https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/WebCore/css/CSSStyleSelector.cpp#L5747
    bool clippedWithLocalScrolling = false;
    //bool clippedWithLocalScrolling = hasOverflowClip && bg_paint.attachment == litehtml::background_attachment::background_attachment_fixed;
    //bg.hasOverflowClip() && bgLayer->attachment() == LocalBackgroundAttachment;
    if (clippedWithLocalScrolling) {
        // Clip to the overflow area.
        graphicsContext->save();

        // https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/WebCore/rendering/RenderBoxModelObject.cpp#L538
        // https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/WebCore/rendering/RenderBox.cpp#L1043
        FloatRect overflowClipRect = FloatRect(bg_paint.clip_box.left(), bg_paint.clip_box.top(), bg_paint.clip_box.width, bg_paint.clip_box.height);

        //graphicsContext->clip(toRenderBox(this)->overflowClipRect(tx, ty));
        graphicsContext->clip(overflowClipRect);

        // Now adjust our tx, ty, w, h to reflect a scrolled content box with borders at the ends.
        IntSize offset(bg_paint.origin_box.width, bg_paint.origin_box.height);//layer()->scrolledContentOffset();
        tx -= offset.width();
        ty -= offset.height();
        //w = bLeft + layer()->scrollWidth() + bRight;
        //h = borderTop() + layer()->scrollHeight() + borderBottom();
        int scrollWidth = w;
        w = bLeft + scrollWidth + bRight;
        int scrollHeight = h;
        w = bg_paint.border_box.top() + scrollHeight +  bg_paint.border_box.bottom();
    }

     // TODO
    /*int borderTop = bg_paint.border_box.top();
    int borderBottom = bg_paint.border_box.bottom();
    int paddingTop = bg_paint.origin_box.top();
    int paddingBottom = bg_paint.origin_box.bottom();

    //if (bgLayer->clip() == PaddingFillBox || bgLayer->clip() == ContentFillBox) {
        // Clip to the padding or content boxes as necessary.
        // TODO
        bool includePadding = true;//bgLayer->clip() == ContentFillBox;
        int x = tx + bLeft + (includePadding ? pLeft : 0);
        int y = ty + borderTop + (includePadding ? paddingTop : 0);
        int width = w - bLeft - bRight - (includePadding ? pLeft + pRight : 0);
        int height = h - borderTop - borderBottom - (includePadding ? paddingTop + paddingBottom : 0);
        graphicsContext->save();
        graphicsContext->clip(IntRect(x, y, width, height));

} else if (bgLayer->clip() == TextFillBox) {
        // We have to draw our text into a mask that can then be used to clip background drawing.
        // First figure out how big the mask has to be.  It should be no bigger than what we need
        // to actually render, so we should intersect the dirty rect with the border box of the background.
        IntRect maskRect(tx, ty, w, h);
        maskRect.intersect(paintInfo.rect);

        // Now create the mask.
        OwnPtr<ImageBuffer> maskImage = ImageBuffer::create(maskRect.size());
        if (!maskImage)
            return;

        GraphicsContext* maskImageContext = maskImage->context();
        maskImageContext->translate(-maskRect.x(), -maskRect.y());

        // Now add the text to the clip.  We do this by painting using a special paint phase that signals to
        // InlineTextBoxes that they should just add their contents to the clip.
        PaintInfo info(maskImageContext, maskRect, PaintPhaseTextClip, true, 0, 0);
        if (box)
            box->paint(info, tx - box->x(), ty - box->y());
        else {
            int x = isBox() ? toRenderBox(this)->x() : 0;
            int y = isBox() ? toRenderBox(this)->y() : 0;
            paint(info, tx - x, ty - y);
        }

        // The mask has been created.  Now we just need to clip to it.
        graphicsContext->save();
        graphicsContext->clipToImageBuffer(maskImage.get(), maskRect);
    }*/

    //StyleImage* bg = bgLayer->image();
    bool shouldPaintBackgroundImage = !bg_paint.image.empty() && bg_paint.image_size.width > 0 && bg_paint.image_size.height > 0;//bg && bg->canRender(style()->effectiveZoom());
    Color bgColor = c;

/*
    // When this style flag is set, change existing background colors and images to a solid white background.
    // If there's no bg color or image, leave it untouched to avoid affecting transparency.
    // We don't try to avoid loading the background images, because this style flag is only set
    // when printing, and at that point we've already loaded the background images anyway. (To avoid
    // loading the background images we'd have to do this check when applying styles rather than
    // while rendering.)
    if (style()->forceBackgroundsToWhite()) {
        // Note that we can't reuse this variable below because the bgColor might be changed
        bool shouldPaintBackgroundColor = !bgLayer->next() && bgColor.isValid() && bgColor.alpha() > 0;
        if (shouldPaintBackgroundImage || shouldPaintBackgroundColor) {
            bgColor = Color::white;
            shouldPaintBackgroundImage = false;
        }
    }*/

    // is_root is true for the root item (<body>) so you have
    //  to apply the background to entire client area/window.
    bool isRoot = bg_paint.is_root;//this->isRoot();

    // Only fill with a base color (e.g., white) if we're the root document, since iframes/frames with
    // no background in the child document should show the parent's background.
    bool isOpaqueRoot = isRoot;
    /*if (isRoot) {
        isOpaqueRoot = true;
        if (!bgLayer->next() && !(bgColor.isValid() && bgColor.alpha() == 255) && view()->frameView()) {
            Element* ownerElement = document()->ownerElement();
            if (ownerElement) {
                if (!ownerElement->hasTagName(frameTag)) {
                    // Locate the <body> element using the DOM.  This is easier than trying
                    // to crawl around a render tree with potential :before/:after content and
                    // anonymous blocks created by inline <body> tags etc.  We can locate the <body>
                    // render object very easily via the DOM.
                    HTMLElement* body = document()->body();
                    if (body) {
                        // Can't scroll a frameset document anyway.
                        isOpaqueRoot = body->hasLocalName(framesetTag);
                    }
#if ENABLE(SVG)
                    else {
                        // SVG documents and XML documents with SVG root nodes are transparent.
                        isOpaqueRoot = !document()->hasSVGRootNode();
                    }
#endif
                }
            } else
                isOpaqueRoot = !view()->frameView()->isTransparent();
        }
        view()->frameView()->setContentIsOpaque(isOpaqueRoot);
    }*/

    // Paint the color first underneath all images.
    //if (!bgLayer->next())
    {

        /*if(isRoot) {
          tx = 0;
          ty = 0;
          w = 5000;//maxRootWidth;
          h = 5000;//maxRootHeight;
          qDebug() << "isRoot" << bg_paint.color.red;
        }*/

        // IntRect rect(tx, ty, w, h);
        //IntRect rect(bg.position_x, bg.position_y, bg.clip_box.width, bg.clip_box.height);
        IntRect rect(tx, ty,
          w, h);


        // don`t draw background outside of element
        intersect_clip(rect);

        //rect.intersect(paintInfo.rect);
        //apply_clip( graphicsContext->platformContext() );

        // If we have an alpha and we are painting the root element, go ahead and blend with the base background color.
        if (isOpaqueRoot) {
            Color baseColor = bgColor;//toColor(bg.color);//view()->frameView()->baseBackgroundColor();
            if (baseColor.alpha() > 0) {
                graphicsContext->save();
                graphicsContext->setCompositeOperation(CompositeCopy);
                //graphicsContext->fillRect(rect, baseColor, style()->colorSpace());
                graphicsContext->fillRect(rect, baseColor, colorSpace);
                graphicsContext->restore();
            } else
                graphicsContext->clearRect(rect);
        }

        if (bgColor.isValid() && bgColor.alpha() > 0)
            graphicsContext->fillRect(rect, bgColor, colorSpace);
    }

    // no progressive loading of the background image
    if (shouldPaintBackgroundImage) {
        //QRect clipRect(clip_a, clip_b);

        // TODO: https://github.com/nem0/lumixengine_html/blob/e2da43e704ad0ad474c8ecafc16fcae4a51e8aff/src/editor/plugins.cpp#L162
        //Crc32 crc32;
        //auto iter = m_images.find(crc32.calculateFromData(bg.image.c_str()));
        auto iter = m_imagesCache.find(bg_paint.image.c_str());
        if (iter == m_imagesCache.end()) {
          shouldPaintBackgroundImage = false;
        }

        if (shouldPaintBackgroundImage) { // <<< TODO
          QImage* img = getImage( bg_paint.image.c_str(), bg_paint.baseurl.c_str() );
          if (!img) {
            shouldPaintBackgroundImage = false;
          }

          if (shouldPaintBackgroundImage) { // <<< TODO

            IntRect destRect;
            IntPoint phase;
            IntSize tileSize;

            calculateBackgroundImageGeometry(
              maxRootWidth, maxRootHeight,
              //bgLayer,
              offsetX, offsetY,
              bg_paint,
              tx, ty, w, h, destRect, phase, tileSize);

            IntPoint destOrigin = destRect.location();

            //destRect.intersect(paintInfo.rect);
            // don`t draw background outside of element
            intersect_clip(destRect);

            QImage imscaled = img->scaled(tileSize.width(), tileSize.height(), Qt::IgnoreAspectRatio,Qt::SmoothTransformation);

            //QBrush *brushTiledImage = new QBrush(imscaled);
            //brushTiledImage->setStyle(Qt::BrushStyle::TexturePattern);

            //graphicsContext->platformContext()->setBackground(*brushTiledImage);
            //graphicsContext->platformContext()->drawImage(
             // bg_paint.clip_box.x, bg_paint.clip_box.y, imscaled, 0, 0, bg_paint.clip_box.x - bg_paint.clip_box.x, bg_paint.clip_box.y - bg_paint.clip_box.y );
            /*qDebug() << "drawn image" << bg_paint.image.c_str() << bg_paint.baseurl.c_str()
              << tileSize.width() << tileSize.height()
              << destRect.location().x() << destRect.location().y() << destRect.width() << destRect.height();
*/

    //apply_clip( graphicsContext->platformContext() );
    //QBrush *brushTiledImage = new QBrush(imscaled);
            /*graphicsContext->platformContext()->drawImage(
              //clip_a.x(), clip_a.y(), imscaled, 0, 0, clip_b.x() - clip_a.x(), clip_b.y() - clip_a.y() );
              destRect.x(), destRect.y(), imscaled, 0, 0, destRect.width(), destRect.height() );
*/
            //graphicsContext->platformContext()->drawLine(0, 0, 400, 400);
            if (!destRect.isEmpty()) {
                phase += destRect.location() - destOrigin;
                //CompositeOperator compositeOp = op == CompositeSourceOver ? bgLayer->composite() : op;
                CompositeOperator compositeOp = op; // TODO

                // https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/WebCore/rendering/style/FillLayer.h#L67
                //StyleImage bg;  // is FillLayer->image()
                QPixmap pixmap;
                if (pixmap.convertFromImage(imscaled)) {

                  StillImage stillImage ( pixmap );
                  Image* image = &stillImage;

                  //RenderObject* clientForBackgroundImage = backgroundObject ? backgroundObject : this;
                  //Image* image = bg->image(clientForBackgroundImage, tileSize);
                  bool useLowQualityScaling = false;//shouldPaintAtLowQuality(graphicsContext, image, tileSize);
                  graphicsContext->drawTiledImage(image, colorSpace, destRect, phase, tileSize, compositeOp, useLowQualityScaling);
                } else {
                  qWarning() << "error converting image to pixmap";
                }
            }
          }
        }
    }

    //if (bgLayer->clip() != BorderFillBox)
    /*if (clip_box != border_box)
        // Undo the background clip
        graphicsContext->restore();*/

    if (clippedToBorderRadius)
        // Undo the border radius clip
        graphicsContext->restore();

    if (clippedWithLocalScrolling) // Undo the clip for local background attachments.
        graphicsContext->restore();
}

// https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/WebCore/rendering/RenderBoxModelObject.cpp
static inline void uniformlyExpandBorderRadii(int delta, IntSize& topLeft, IntSize& topRight, IntSize& bottomLeft, IntSize& bottomRight)
{
    topLeft.expand(delta, delta);
    topLeft.clampNegativeToZero();
    topRight.expand(delta, delta);
    topRight.clampNegativeToZero();
    bottomLeft.expand(delta, delta);
    bottomLeft.clampNegativeToZero();
    bottomRight.expand(delta, delta);
    bottomRight.clampNegativeToZero();
}

// https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/WebCore/rendering/RenderBoxModelObject.cpp#L1611
static void paintBoxShadow(GraphicsContext* context,
  const litehtml::background_paint& bg_paint,
  int tx, int ty, int w, int h,
  //const RenderStyle* s,
  ShadowStyle shadowStyle,
  ColorSpace colorSpace,
  ShadowData* boxShadow,
  bool hasBorderRadius,
  bool hasOpaqueBackground, // if color.alpha == 255
  bool begin, bool end)
{
    /// \note boxShadow initialized with nullptr
    if (!boxShadow) {
      return;
    }

    // FIXME: Deal with border-image.  Would be great to use border-image as a mask.

    if (context->paintingDisabled())
        return;

    IntRect rect(tx, ty, w, h);
    IntSize topLeft;
    IntSize topRight;
    IntSize bottomLeft;
    IntSize bottomRight;

    int borderTop = bg_paint.border_box.top();
    int borderBottom = bg_paint.border_box.bottom();
    int borderLeft = bg_paint.border_box.left();
    int borderRight = bg_paint.border_box.right();

    litehtml::borders border_radiuses;
    border_radiuses.radius.top_left_x = bg_paint.border_radius.top_left_x;
    border_radiuses.radius.top_left_y = bg_paint.border_radius.top_left_y;
    border_radiuses.radius.top_right_x = bg_paint.border_radius.top_right_x;
    border_radiuses.radius.top_right_y = bg_paint.border_radius.top_right_y;
    border_radiuses.radius.bottom_left_x = bg_paint.border_radius.bottom_left_x;
    border_radiuses.radius.bottom_left_y = bg_paint.border_radius.bottom_left_y;
    border_radiuses.radius.bottom_right_x = bg_paint.border_radius.bottom_right_x;
    border_radiuses.radius.bottom_right_y = bg_paint.border_radius.bottom_right_y;

    //bool hasBorderRadius = s->hasBorderRadius();
    if (hasBorderRadius && (begin || end)) {
        IntSize topLeftRadius, topRightRadius, bottomLeftRadius, bottomRightRadius;
        getBorderRadiiForRect(rect, border_radiuses, topLeftRadius, topRightRadius, bottomLeftRadius, bottomRightRadius);

        if (begin) {
            if (shadowStyle == Inset) {
                topLeftRadius.expand(-borderLeft, -borderTop);
                topLeftRadius.clampNegativeToZero();
                bottomLeftRadius.expand(-borderLeft, -borderBottom);
                bottomLeftRadius.clampNegativeToZero();
            }
            topLeft = topLeftRadius;
            bottomLeft = bottomLeftRadius;
        }
        if (end) {
            if (shadowStyle == Inset) {
                topRightRadius.expand(-borderRight, -borderTop);
                topRightRadius.clampNegativeToZero();
                bottomRightRadius.expand(-borderRight, -borderBottom);
                bottomRightRadius.clampNegativeToZero();
            }
            topRight = topRightRadius;
            bottomRight = bottomRightRadius;
        }
    }

    if (shadowStyle == Inset) {
        rect.move(begin ? borderLeft : 0, borderTop);
        rect.setWidth(rect.width() - (begin ? borderLeft : 0) - (end ? borderRight : 0));
        rect.setHeight(rect.height() - borderTop - borderBottom);
    }

    //bool hasOpaqueBackground = s->visitedDependentColor(CSSPropertyBackgroundColor).isValid()
    //  && s->visitedDependentColor(CSSPropertyBackgroundColor).alpha() == 255;
    for (const ShadowData* shadow = boxShadow/*s->boxShadow()*/; shadow; shadow = shadow->next()) {
        if (shadow->style() != shadowStyle)
            continue;

        IntSize shadowOffset(shadow->x(), shadow->y());
        int shadowBlur = shadow->blur();
        int shadowSpread = shadow->spread();
        const Color& shadowColor = shadow->color();

    //if (shadow)
    //  qDebug() << "shadow shadowColor " << shadowColor.red()<<shadowColor.green()<< shadowColor.blue()<< shadowColor.alpha();

        if (shadow->style() == Normal) {
            IntRect fillRect(rect);
            fillRect.inflate(shadowSpread);
            if (fillRect.isEmpty())
                continue;

            IntRect shadowRect(rect);
            shadowRect.inflate(shadowBlur + shadowSpread);
            shadowRect.move(shadowOffset);

            context->save();
            context->clip(shadowRect);

            // Move the fill just outside the clip, adding 1 pixel separation so that the fill does not
            // bleed in (due to antialiasing) if the context is transformed.
            IntSize extraOffset(w + max(0, shadowOffset.width()) + shadowBlur + 2 * shadowSpread + 1, 0);
            shadowOffset -= extraOffset;
            fillRect.move(extraOffset);

            context->setShadow(shadowOffset, shadowBlur, shadowColor, colorSpace);
            if (hasBorderRadius) {
                IntRect rectToClipOut = rect;
                IntSize topLeftToClipOut = topLeft;
                IntSize topRightToClipOut = topRight;
                IntSize bottomLeftToClipOut = bottomLeft;
                IntSize bottomRightToClipOut = bottomRight;

                if (shadowSpread < 0)
                    uniformlyExpandBorderRadii(shadowSpread, topLeft, topRight, bottomLeft, bottomRight);

                // If the box is opaque, it is unnecessary to clip it out. However, doing so saves time
                // when painting the shadow. On the other hand, it introduces subpixel gaps along the
                // corners. Those are avoided by insetting the clipping path by one pixel.
                if (hasOpaqueBackground) {
                    rectToClipOut.inflate(-1);
                    uniformlyExpandBorderRadii(-1, topLeftToClipOut, topRightToClipOut, bottomLeftToClipOut, bottomRightToClipOut);
                }

                if (!rectToClipOut.isEmpty())
                    context->clipOutRoundedRect(rectToClipOut, topLeftToClipOut, topRightToClipOut, bottomLeftToClipOut, bottomRightToClipOut);
                context->fillRoundedRect(fillRect, topLeft, topRight, bottomLeft, bottomRight, Color::black, colorSpace);
            } else {
                IntRect rectToClipOut = rect;

                // If the box is opaque, it is unnecessary to clip it out. However, doing so saves time
                // when painting the shadow. On the other hand, it introduces subpixel gaps along the
                // edges if they are not pixel-aligned. Those are avoided by insetting the clipping path
                // by one pixel.
                if (hasOpaqueBackground) {
                    AffineTransform currentTransformation = context->getCTM();
                    if (currentTransformation.a() != 1 || (currentTransformation.d() != 1 && currentTransformation.d() != -1)
                            || currentTransformation.b() || currentTransformation.c())
                        rectToClipOut.inflate(-1);
                }

                if (!rectToClipOut.isEmpty())
                    context->clipOut(rectToClipOut);
                context->fillRect(fillRect, Color::black, colorSpace);
            }

            context->restore();
        } else {
            // Inset shadow.
            IntRect holeRect(rect);
            holeRect.inflate(-shadowSpread);

            if (holeRect.isEmpty()) {
                if (hasBorderRadius)
                    context->fillRoundedRect(rect, topLeft, topRight, bottomLeft, bottomRight, shadowColor, colorSpace);
                else
                    context->fillRect(rect, shadowColor, colorSpace);
                continue;
            }
            if (!begin) {
                holeRect.move(-max(shadowOffset.width(), 0) - shadowBlur, 0);
                holeRect.setWidth(holeRect.width() + max(shadowOffset.width(), 0) + shadowBlur);
            }
            if (!end)
                holeRect.setWidth(holeRect.width() - min(shadowOffset.width(), 0) + shadowBlur);

            Color fillColor(shadowColor.red(), shadowColor.green(), shadowColor.blue(), 255);

            IntRect outerRect(rect);
            outerRect.inflateX(w - 2 * shadowSpread);
            outerRect.inflateY(h - 2 * shadowSpread);

            context->save();

            Path path;
            if (hasBorderRadius) {
                path.addRoundedRect(rect, topLeft, topRight, bottomLeft, bottomRight);
                context->clip(path);
                path.clear();
            } else
                context->clip(rect);

            IntSize extraOffset(2 * w + max(0, shadowOffset.width()) + shadowBlur - 2 * shadowSpread + 1, 0);
            context->translate(extraOffset.width(), extraOffset.height());
            shadowOffset -= extraOffset;

            path.addRect(outerRect);

            if (hasBorderRadius) {
                if (shadowSpread > 0)
                    uniformlyExpandBorderRadii(-shadowSpread, topLeft, topRight, bottomLeft, bottomRight);
                path.addRoundedRect(holeRect, topLeft, topRight, bottomLeft, bottomRight);
            } else
                path.addRect(holeRect);

            context->beginPath();
            context->addPath(path);

            context->setFillRule(RULE_EVENODD);
            context->setFillColor(fillColor, colorSpace);
            context->setShadow(shadowOffset, shadowBlur, shadowColor, colorSpace);
            context->fillPath();

            context->restore();
        }
    }
}

// https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/WebCore/rendering/RenderBoxModelObject.cpp#L502
// https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/WebCore/rendering/RenderBox.cpp#L786
// https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/WebCore/rendering/style/RenderStyle.h#L563
void container_qt5::draw_background(litehtml::uint_ptr hdc, const litehtml::background_paint& bg_paint)
{
    Q_ASSERT(hdc);

    //qDebug() << "draw_background" << bg.color.red;

    GraphicsContext* graphicsContext = (GraphicsContext*) hdc;
    if(!graphicsContext) {
      return;
    }
    QPainter *painter = (QPainter *) graphicsContext->platformContext();
    if(!painter) {
      return;
    }

    ColorSpace colorSpace = sRGBColorSpace;

    bool hasBorderRadius = bg_paint.border_radius.top_left_x > 0
      ||  bg_paint.border_radius.top_left_y > 0
      ||  bg_paint.border_radius.top_right_x > 0
      ||  bg_paint.border_radius.top_right_y > 0
      ||  bg_paint.border_radius.bottom_left_x > 0
      ||  bg_paint.border_radius.bottom_left_y > 0
      ||  bg_paint.border_radius.bottom_right_x > 0
      ||  bg_paint.border_radius.bottom_right_y > 0;

    Color color = toColor(bg_paint.color);

    bool hasOpaqueBackground = color.isValid() && color.alpha() == 255;

    ShadowStyle shadowStyle = ShadowStyle::Normal;
    // https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/WebCore/css/CSSStyleSelector.cpp#L4832
    ShadowData* boxShadow = bg_paint.box_shadow;//new ShadowData(5, 5, 50, 50, shadowStyle, color);//Color(255,0,255,255) );//color.isValid() ? color : Color::transparent);

    //boxShadow->setNext()
    //boxShadow->set

    // /// \note with offsetX
    int tx = /*offsetX+*/bg_paint.clip_box.left();
    //qDebug() << bg.position_x << bg.clip_box.left();

    // /// \note with offsetY
    int ty = /*offsetY+*/bg_paint.clip_box.top();

    int w = bg_paint.clip_box.width;
    int h = bg_paint.clip_box.height;

    // border-fit can adjust where we paint our border and background.  If set, we snugly fit our line box descendants.  (The iChat
    // balloon layout is an example of this).
    // borderFitAdjust(tx, width);

    // FIXME: Should eventually give the theme control over whether the box shadow should paint, since controls could have
    // custom shadows of their own.
    /// \note skips all nested shadows with different ShadowStyle
    paintBoxShadow(graphicsContext, bg_paint,
      tx, ty, w, h,
      ShadowStyle::Normal, // force ShadowStyle::Normal
      colorSpace,
      boxShadow,
      hasBorderRadius,
      hasOpaqueBackground,
      true,
      true);

    // TODO: multiple layers
    // https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/WebCore/rendering/RenderBox.cpp#L786
    paintFillLayerExtended(
      _doc->width(), _doc->height(),
      graphicsContext,
      color,
      hasBorderRadius,
      offsetX, offsetY,
      bg_paint,
      colorSpace,
      CompositeSourceOver);

    /// \note skips all nested shadows with different ShadowStyle
    paintBoxShadow(graphicsContext, bg_paint,
      tx, ty, w, h,
      ShadowStyle::Inset, // force ShadowStyle::Inset
      colorSpace,
      boxShadow,
      hasBorderRadius,
      hasOpaqueBackground,
      true,
      true);

/*
    painter->save();

    apply_clip( painter );

    // clip_box. Defines the position of the clipping box. See the background-clip CSS property.
    QPoint clip_a(offsetX + bg.clip_box.left(), offsetY + bg.clip_box.top());
    QPoint clip_b(offsetX + bg.clip_box.right(), offsetY + bg.clip_box.bottom());
    QRect clipRect(clip_a, clip_b);

    // The image contains the background image URL. URL can be relative.
    // Use the baseurl member to find the base URL for image.
    if (bg.image.empty())
    {
      // This member defines the background-color CSS property.
      painter->fillRect(clipRect, QBrush(getColor(bg.color)) );

      // border_box
      painter->fillRect(bg.border_box.x, bg.border_box.y, bg.border_box.width, bg.border_box.height, QBrush(getColor(bg.color)) );

      //pnt->fillRect(0, 0, 2000, 2000, QBrush(QColor(100,100,100,100)) );
      painter->restore();
      return;
    }


    QImage* img = getImage( bg.image.c_str(), bg.baseurl.c_str() );
    if (!img) {
      painter->restore();
      return;
    }

    //auto img = iter.value();

    auto imscaled = img->scaled(clip_b.x() - clip_a.x(), clip_b.y() - clip_a.y(), Qt::IgnoreAspectRatio,Qt::SmoothTransformation);

    QBrush *brushTiledImage = new QBrush(imscaled);
    brushTiledImage->setStyle(Qt::BrushStyle::TexturePattern);


        //pnt->drawImage( bg.position_x, bg.position_y, *img, 0, 0, img->width(), img->height() );
        painter->drawImage( clip_a.x(), clip_a.y(), imscaled, 0, 0, clip_b.x() - clip_a.x(), clip_b.y() - clip_a.y() );
        //win->DrawList->AddImage(img.textureId, clip_a, clip_b);
*/

//#define old_bg_drawer
#ifdef old_bg_drawer

    //QPainter* painter = (QPainter*)hdc;
    //QPainter ptr(m_owner);
    //QPainter* painter = &ptr;

    painter->save();

    apply_clip( painter );

    // clip_box. Defines the position of the clipping box. See the background-clip CSS property.
    QPoint clip_a(offsetX + bg.clip_box.left(), offsetY + bg.clip_box.top());
    QPoint clip_b(offsetX + bg.clip_box.right(), offsetY + bg.clip_box.bottom());
    QRect clipRect(clip_a, clip_b);

    if( clip_a.x() >= clip_b.x() || clip_a.y() >= clip_b.y() ) {
      // see example https://github.com/marcj/Pesto/blob/master/src/pesto/demo/example3.cpp
      painter->restore();
      return;
    }

    // The image contains the background image URL. URL can be relative.
    // Use the baseurl member to find the base URL for image.
    if (bg.image.empty())
    {
      // This member defines the background-color CSS property.
      painter->fillRect(clipRect, QBrush(getColor(bg.color)) );

      // border_box
      painter->fillRect(bg.border_box.x, bg.border_box.y, bg.border_box.width, bg.border_box.height, QBrush(getColor(bg.color)) );

      //pnt->fillRect(0, 0, 2000, 2000, QBrush(QColor(100,100,100,100)) );
      painter->restore();
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
      // TODO  - m_Scroll.y()
      painter->restore();
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
      painter->fillRect(clipRect, dummyColor );
      painter->restore();
      return;
    }

    QImage* img = getImage( bg.image.c_str(), bg.baseurl.c_str() );
    if (!img) {
      painter->fillRect(clipRect, dummyColor );
      painter->restore();
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
        painter->drawImage( clip_a.x(), clip_a.y(), imscaled, 0, 0, clip_b.x() - clip_a.x(), clip_b.y() - clip_a.y() );
        //win->DrawList->AddImage(img.textureId, clip_a, clip_b);
        break;
      }
      case litehtml::background_repeat_repeat_x:
      {
        painter->setBackground(*brushTiledImage);
        painter->drawImage( clip_a.x(), clip_a.y(), imscaled, 0, 0, clip_b.x() - clip_a.x(), clip_b.y() - clip_a.y() );
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
        painter->setBackground(*brushTiledImage);
        painter->drawImage( clip_a.x(), clip_a.y(), imscaled, 0, 0, clip_b.x() - clip_a.x(), clip_b.y() - clip_a.y() );
        //ImVec2 uv(0, (clip_b.y - clip_a.y) / img.h);
        //glBindTexture(GL_TEXTURE_2D, my_opengl_texture);
        //ImGui::GetWindowDrawList()->PushTextureID(img.textureId);
        //win->DrawList->AddImage(img.textureId, clip_a, clip_b, ImVec2(0, 0), uv);
        //ImGui::GetWindowDrawList()->PopTextureID();
        break;
      }
      case litehtml::background_repeat_repeat:
      {
        painter->setBackground(*brushTiledImage);
        painter->drawImage( clip_a.x(), clip_a.y(), imscaled, 0, 0, clip_b.x() - clip_a.x(), clip_b.y() - clip_a.y() );
        //ImVec2 uv((clip_b.x - clip_a.x) / img.w, (clip_b.y - clip_a.y) / img.h);
        //glBindTexture(GL_TEXTURE_2D, my_opengl_texture);
        //ImGui::GetWindowDrawList()->PushTextureID(img.textureId);
        //win->DrawList->AddImage(img.textureId, clip_a, clip_b, ImVec2(0, 0), uv);
        //ImGui::GetWindowDrawList()->PopTextureID();
        break;
      }
    }

    painter->restore();
#endif
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

  if( m_imagesCache.find(fname.toUtf8())==m_imagesCache.end() ) {
    QImage*	img = new QImage( );
    if( img->load( fname ) ) {
      m_imagesCache[fname.toUtf8()] = img;
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

    //QPainter *painter = (QPainter *) hdc;

    GraphicsContext* graphicsContext = (GraphicsContext*) hdc;
    if(!graphicsContext) {
      return;
    }
    QPainter *painter = (QPainter *) graphicsContext->platformContext();
    if(!painter) {
      return;
    }

    painter->save();

    apply_clip( painter );

    QRect position = getRect(marker.pos);
    QColor color = getColor(marker.color);

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
    //QPainter *painter = (QPainter *) hdc;

    GraphicsContext* graphicsContext = (GraphicsContext*) hdc;
    if(!graphicsContext) {
      return;
    }
    QPainter *painter = (QPainter *) graphicsContext->platformContext();
    if(!painter) {
      return;
    }

    painter->save();

    apply_clip(painter);

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

    painter->restore();
}

int container_qt5::text_width(const litehtml::tchar_t* text, litehtml::uint_ptr hFont)
{
    if (!hFont) {
      qDebug() << __FUNCTION__ << " can`t get text width for empty font";
      return getDefaultFontSize();
    }

    //qDebug() << __FUNCTION__;
    QFont *font = (QFont *) hFont;
    QFontMetrics metrics(*font);
    QString txt(text);
    //qDebug() << "For" << txt << metrics.boundingRect(txt);
    /*if (txt == " ") {
        return metrics.boundingRect("x").width();
    }
    return metrics.boundingRect(txt).width();*/
    return metrics.width( text );
}

void container_qt5::delete_font(litehtml::uint_ptr hFont)
{
    //qDebug() << __FUNCTION__;
    QFont *font = (QFont *) hFont;

    if (font) {
      // TODO: qt auto deletes fonts
      //delete font;
    }
}

litehtml::uint_ptr container_qt5::create_font(const litehtml::tchar_t* faceName, int size, int weight, litehtml::font_style italic, unsigned int decoration, litehtml::font_metrics* fm)
{
    if (!faceName || !fm) {
      return nullptr;
    }

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
    : QWidget(parent)
{
  setMouseTracking(true);

  // Within the body of a constructor or destructor,
  // there is a valid object of the class currently being constructed
  container = new container_qt5(this);
}

litehtmlWidget::~litehtmlWidget()
{

}

void litehtmlWidget::paintEvent(QPaintEvent *event)
{
    //container->setDrawArea(QRect(0, 0, width(), height()));
    //container->setSize(width(), height());

    QPainter painter(this);
    container->repaint(painter);
}

static QRect adoptQMouseEventCoords(QMouseEvent *event, int offsetX, int offsetY) {
  return QRect(offsetX+event->x(), offsetY+event->y(), offsetX+event->x(), offsetY+event->y());
}

void litehtmlWidget::mouseMoveEvent(QMouseEvent *event)
{
    litehtml::position::vector redraw_boxes;

    QRect adopted = adoptQMouseEventCoords(event, -container->getScroll().x(), -container->getScroll().y());

    container->setLastMouseCoords(adopted.x(), adopted.y(), adopted.x(), adopted.y());

    if (container->getDocument()->on_mouse_over(adopted.x(), adopted.y(), adopted.x(), adopted.y(), redraw_boxes)) {
      repaint();
    }

    // allows resizing of painting area
    container->_doc->media_changed();
}

void litehtmlWidget::mousePressEvent(QMouseEvent *event)
{
    litehtml::position::vector redraw_boxes;

    QRect adopted = adoptQMouseEventCoords(event, -container->getScroll().x(), -container->getScroll().y());

    container->setLastMouseCoords(adopted.x(), adopted.y(), adopted.x(), adopted.y());

    if (container->getDocument()->on_lbutton_down(adopted.x(), adopted.y(), adopted.x(), adopted.y(), redraw_boxes)) {
      repaint();
    }

    // allows resizing of painting area
    container->_doc->media_changed();
}

void litehtmlWidget::mouseReleaseEvent(QMouseEvent *event)
{
    litehtml::position::vector redraw_boxes;

    QRect adopted = adoptQMouseEventCoords(event, -container->getScroll().x(), -container->getScroll().y());

    container->setLastMouseCoords(adopted.x(), adopted.y(), adopted.x(), adopted.y());

    if (container->getDocument()->on_lbutton_up(adopted.x(), adopted.y(), adopted.x(), adopted.y(), redraw_boxes)) {
      repaint();
    }

    // allows resizing of painting area
    container->_doc->media_changed();
}

void litehtmlWidget::resizeEvent(QResizeEvent *event)
{
  // allows resizing of painting area
  container->_doc->media_changed();

  /*QRect	rc = rect( );

  container->_doc->render( rc.width() );
  container->_doc->render( rc.width() );*/
}

void litehtmlWidget::wheelEvent(QWheelEvent *event)
{

}
