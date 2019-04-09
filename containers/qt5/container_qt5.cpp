#include "container_qt5.h"
#include "types.h"
#include <QDebug>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QMouseEvent>
#include <QDesktopServices>

#include "fontcache.h"
#include <cmath>

// https://github.com/Nanquitas/3DS_eBook_Reader/blob/51f1fedc2565de36253104a01f4689db00c35991/source/Litehtml3DSContainer.cpp#L18
#define PPI 132.1


static litehtml::position::vector	m_clips;

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

    QRect	rc = m_owner->rect();

    //_doc->render(m_drawArea.width());

    /// \note don`t allow render size < 1
    _doc->render(std::max(rc.width(), 1));

    litehtml::position clipPos;
    clipPos.width 	= rc.width();
    clipPos.height 	= rc.height();
    clipPos.x 		= rc.x();
    clipPos.y 		= rc.y();

    _doc->draw(static_cast<litehtml::uint_ptr>(&painter), getScroll().x(), getScroll().y(), &clipPos);

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
    /*//qDebug() << "get_client_rect";
    // No scroll yet
    client.move_to(0, 0);
    client.width = m_drawArea.width();
    client.height = m_drawArea.height();

    //qDebug() << "==> " << client.width << "x" << client.height;*/

    QRect	rc = m_owner->rect();
    client.x = rc.left();
    client.y = rc.top();
    client.width = rc.width();
    client.height = rc.height();
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

#define DEGREES(t) ((t) * 180.0 / M_PI)

#define FloatPoint QPointF

#ifndef M_PI
const double piDouble = 3.14159265358979323846;
const float piFloat = 3.14159265358979323846f;
#else
const double piDouble = M_PI;
const float piFloat = static_cast<float>(M_PI);
#endif

static void
addArc(QPainter* painter, QPainterPath& path, const FloatPoint& p, float r, float sar, float ear, bool anticlockwise)
{
    qreal xc = p.x();
    qreal yc = p.y();
    qreal radius = r;


    //### HACK
    // In Qt we don't switch the coordinate system for degrees
    // and still use the 0,0 as bottom left for degrees so we need
    // to switch
    sar = -sar;
    ear = -ear;
    anticlockwise = !anticlockwise;
    //end hack

    float sa = DEGREES(sar);
    float ea = DEGREES(ear);

    double span = 0;

    double xs = xc - radius;
    double ys = yc - radius;
    double width  = radius*2;
    double height = radius*2;

    if ((!anticlockwise && (ea - sa >= 360)) || (anticlockwise && (sa - ea >= 360)))
        // If the anticlockwise argument is false and endAngle-startAngle is equal to or greater than 2*PI, or, if the
        // anticlockwise argument is true and startAngle-endAngle is equal to or greater than 2*PI, then the arc is the whole
        // circumference of this circle.
        span = 360;
    else {
        if (!anticlockwise && (ea < sa))
            span += 360;
        else if (anticlockwise && (sa < ea))
            span -= 360;

        // this is also due to switched coordinate system
        // we would end up with a 0 span instead of 360
        if (!(qFuzzyCompare(span + (ea - sa) + 1, 1.0)
            && qFuzzyCompare(qAbs(span), 360.0))) {
            // mod 360
            span += (ea - sa) - (static_cast<int>((ea - sa) / 360)) * 360;
        }
    }

    // If the path is empty, move to where the arc will start to avoid painting a line from (0,0)
    // NOTE: QPainterPath::isEmpty() won't work here since it ignores a lone MoveToElement
    if (!path.elementCount())
        path.arcMoveTo(xs, ys, width, height, sa);
    else if (!radius) {
        path.lineTo(xc, yc);
        return;
    }

    path.arcTo(xs, ys, width, height, sa, span);
}

// see
// https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/WebCore/platform/graphics/qt/PathQt.cpp#L201
// https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/WebCore/platform/graphics/cairo/PathCairo.cpp#L146
static void addArcTo(QPainter* painter, QPainterPath& path, const FloatPoint& p1, const FloatPoint& p2, float radius)
{
    FloatPoint p0(path.currentPosition());

    FloatPoint p1p0((p0.x() - p1.x()), (p0.y() - p1.y()));
    FloatPoint p1p2((p2.x() - p1.x()), (p2.y() - p1.y()));
    float p1p0_length = sqrtf(p1p0.x() * p1p0.x() + p1p0.y() * p1p0.y());
    float p1p2_length = sqrtf(p1p2.x() * p1p2.x() + p1p2.y() * p1p2.y());

    double cos_phi = (p1p0.x() * p1p2.x() + p1p0.y() * p1p2.y()) / (p1p0_length * p1p2_length);

    // The points p0, p1, and p2 are on the same straight line (HTML5, 4.8.11.1.8)
    // We could have used areCollinear() here, but since we're reusing
    // the variables computed above later on we keep this logic.
    if (qFuzzyCompare(qAbs(cos_phi), 1.0)) {
        path.lineTo(p1);
        return;
    }

    float tangent = radius / tan(acos(cos_phi) / 2);
    float factor_p1p0 = tangent / p1p0_length;
    FloatPoint t_p1p0((p1.x() + factor_p1p0 * p1p0.x()), (p1.y() + factor_p1p0 * p1p0.y()));

    FloatPoint orth_p1p0(p1p0.y(), -p1p0.x());
    float orth_p1p0_length = sqrt(orth_p1p0.x() * orth_p1p0.x() + orth_p1p0.y() * orth_p1p0.y());
    float factor_ra = radius / orth_p1p0_length;

    // angle between orth_p1p0 and p1p2 to get the right vector orthographic to p1p0
    double cos_alpha = (orth_p1p0.x() * p1p2.x() + orth_p1p0.y() * p1p2.y()) / (orth_p1p0_length * p1p2_length);
    if (cos_alpha < 0.f)
        orth_p1p0 = FloatPoint(-orth_p1p0.x(), -orth_p1p0.y());

    FloatPoint p((t_p1p0.x() + factor_ra * orth_p1p0.x()), (t_p1p0.y() + factor_ra * orth_p1p0.y()));

    // calculate angles for addArc
    orth_p1p0 = FloatPoint(-orth_p1p0.x(), -orth_p1p0.y());
    float sa = acos(orth_p1p0.x() / orth_p1p0_length);
    if (orth_p1p0.y() < 0.f)
        sa = 2 * piDouble - sa;

    // anticlockwise logic
    bool anticlockwise = false;

    float factor_p1p2 = tangent / p1p2_length;
    FloatPoint t_p1p2((p1.x() + factor_p1p2 * p1p2.x()), (p1.y() + factor_p1p2 * p1p2.y()));
    FloatPoint orth_p1p2((t_p1p2.x() - p.x()), (t_p1p2.y() - p.y()));
    float orth_p1p2_length = sqrtf(orth_p1p2.x() * orth_p1p2.x() + orth_p1p2.y() * orth_p1p2.y());
    float ea = acos(orth_p1p2.x() / orth_p1p2_length);
    if (orth_p1p2.y() < 0)
        ea = 2 * piDouble - ea;
    if ((sa > ea) && ((sa - ea) < piDouble))
        anticlockwise = true;
    if ((sa < ea) && ((ea - sa) > piDouble))
        anticlockwise = true;

    path.lineTo(t_p1p0);

    addArc(painter, path, p, radius, sa, ea, anticlockwise);
}


static void add_path_arc(QPainter* painter, QPainterPath& path, double x, double y, double rx, double ry, double a1, double a2, bool neg)
{
            /*QRectF r2(
            draw_pos.left() + r_left,
              draw_pos.bottom() - r_left,
              r_left,
              r_left);
            QPainterPath path;
            path.arcTo(r1,start_angle,end_angle);
            path.arcTo(r2,end_angle,start_angle);
            // and finally fill it
            painter->fillPath(path, QBrush(getColor(borders.bottom.color)));*/


  if(rx > 0 && ry > 0)
  {

    path.translate(x, y);
    //path.scale(1, ry / rx);
    path.translate(-x, -y);

    /*cairo_save(cr);

    cairo_translate(cr, x, y);
    cairo_scale(cr, 1, ry / rx);
    cairo_translate(cr, -x, -y);*/

    if(neg)
    {
      //cairo_arc_negative(cr, x, y, rx, a1, a2);
            QRectF r1(
            x, y, rx, ry
            );
      path.arcTo(r1,a1, a2);
    } else
    {
            QRectF r1(
            x, y, rx, ry
            );
      path.arcTo(r1,a1, a2);
      //cairo_arc(cr, x, y, rx, a1, a2);
    }

    //cairo_restore(cr);
  } else
  {
    path.moveTo(x, y);
    //cairo_move_to(cr, x, y);
  }
}

void container_qt5::draw_borders(litehtml::uint_ptr hdc, const litehtml::borders& borders, const litehtml::position& draw_pos, bool root)
{
    //qDebug() << __FUNCTION__ << " for root = " << root;
    QPainter *painter = (QPainter *) hdc;

    painter->save();

    apply_clip( painter );

    QRect area = getRect(draw_pos);
    if (root) {
        // we are in <html>, so no borders here
        //painter->setPen(Qt::NoPen);
        //painter->fillRect(area, QBrush(getColor(borders.top.color)) );
    } else {
#ifdef nope
        if (borders.top.style != litehtml::border_style_none) {
            setPenForBorder(painter, borders.top);
            painter->drawLine(area.topLeft(), area.topRight());
        }
        if (borders.bottom.style != litehtml::border_style_none) {
            setPenForBorder(painter, borders.bottom);
            painter->drawLine(area.bottomLeft(), area.bottomRight());

            //QPainterPath path;
            //path.addRoundedRect(QRectF(10, 10, 100, 50), 10, 10);
        }
        if (borders.left.style != litehtml::border_style_none) {
            setPenForBorder(painter, borders.left);
            painter->drawLine(area.topLeft(), area.bottomLeft());
        }
        if (borders.right.style != litehtml::border_style_none) {
            setPenForBorder(painter, borders.right);
            painter->drawLine(area.topRight(), area.bottomRight());
        }


        /*int bdr_top		= 0;
        int bdr_bottom	= 0;
        int bdr_left	= 0;
        int bdr_right	= 0;

        if(borders.top.width != 0 && borders.top.style > litehtml::border_style_hidden)
        {
          bdr_top = (int) borders.top.width;
        }
        if(borders.bottom.width != 0 && borders.bottom.style > litehtml::border_style_hidden)
        {
          bdr_bottom = (int) borders.bottom.width;
        }
        if(borders.left.width != 0 && borders.left.style > litehtml::border_style_hidden)
        {
          bdr_left = (int) borders.left.width;
        }
        if(borders.right.width != 0 && borders.right.style > litehtml::border_style_hidden)
        {
          bdr_right = (int) borders.right.width;
        }

        if (bdr_bottom)
        {
          QPoint aFrom(offsetX + draw_pos.left(), offsetY + draw_pos.bottom());
          QPoint bTo(offsetX + draw_pos.right(), offsetY + draw_pos.bottom());
          QColor color(borders.bottom.color.red, borders.bottom.color.green, borders.bottom.color.blue, borders.bottom.color.alpha);
          setPenForBorder(painter, borders.bottom);
          for (int x = 0; x < bdr_bottom; x++)
          {
            //win->DrawList->AddLine(aFrom, bTo, color);
            painter->drawLine(aFrom, bTo);
            aFrom.setY(aFrom.y()+1);
            bTo.setY(bTo.y()+1);
          }
        }

        if (bdr_top)
        {
          QPoint aFrom(offsetX + draw_pos.left(), offsetY + draw_pos.top());
          QPoint bTo(offsetX + draw_pos.right(), offsetY + draw_pos.top());
          QColor color(borders.top.color.red, borders.top.color.green, borders.top.color.blue, borders.top.color.alpha);
          setPenForBorder(painter, borders.top);
          for (int x = 0; x < bdr_top; x++)
          {
            //win->DrawList->AddLine(aFrom, bTo, color);
            painter->drawLine(aFrom, bTo);
            aFrom.setY(aFrom.y()+1);
            bTo.setY(bTo.y()+1);
          }
        }

        if (bdr_right)
        {
          QPoint aFrom(offsetX + draw_pos.right(), offsetY + draw_pos.top());
          QPoint bTo(offsetX + draw_pos.right(), offsetY + draw_pos.bottom());
          QColor color(borders.right.color.red, borders.right.color.green, borders.right.color.blue, borders.right.color.alpha);
          setPenForBorder(painter, borders.right);
          for (int x = 0; x < bdr_right; x++)
          {
            //win->DrawList->AddLine(aFrom, bTo, color);
            painter->drawLine(aFrom, bTo);
            aFrom.setX(aFrom.x()-1);
            bTo.setX(bTo.x()-1);
          }
        }

        if (bdr_left)
        {
          QPoint aFrom(offsetX + draw_pos.left(), offsetY + draw_pos.top());
          QPoint bTo(offsetX + draw_pos.left(), offsetY + draw_pos.bottom());
          QColor color(borders.left.color.red, borders.left.color.green, borders.left.color.blue, borders.left.color.alpha);
          setPenForBorder(painter, borders.left);
          for (int x = 0; x < bdr_left; x++)
          {
            //win->DrawList->AddLine(aFrom, bTo, color);
            painter->drawLine(aFrom, bTo);
            aFrom.setX(aFrom.x()+1);
            bTo.setX(bTo.x()+1);
          }
        }*/
#else

        int bdr_top		= 0;
        int bdr_bottom	= 0;
        int bdr_left	= 0;
        int bdr_right	= 0;

        if(borders.top.width != 0 && borders.top.style > litehtml::border_style_hidden)
        {
          bdr_top = (int) borders.top.width;
        }
        if(borders.bottom.width != 0 && borders.bottom.style > litehtml::border_style_hidden)
        {
          bdr_bottom = (int) borders.bottom.width;
        }
        if(borders.left.width != 0 && borders.left.style > litehtml::border_style_hidden)
        {
          bdr_left = (int) borders.left.width;
        }
        if(borders.right.width != 0 && borders.right.style > litehtml::border_style_hidden)
        {
          bdr_right = (int) borders.right.width;
        }

        // draw right border
        if(bdr_right)
        {
          QPainterPath path;
          //set_color(cr, borders.right.color);

          double r_top	= borders.radius.top_right_x;
          double r_bottom	= borders.radius.bottom_right_x;

          if(r_top)
          {
            double end_angle	= 2 * M_PI;
            double start_angle	= end_angle - M_PI / 2.0  / ((double) bdr_top / (double) bdr_right + 1);

            {
              painter->save();
              auto rx = r_top - bdr_right;
              auto ry = r_top - bdr_right + (bdr_right - bdr_top);
              painter->scale(1, ry / rx);
              addArc(painter, path,
                FloatPoint(draw_pos.right() - r_top,
        draw_pos.top() + r_top),
                rx,
                end_angle,
                start_angle, true);
              painter->restore();
            }
            {
              painter->save();
              auto rx = r_top;
              auto ry = r_top;
              painter->scale(1, ry / rx);
              addArc(painter, path,
                FloatPoint(draw_pos.right() - r_top,
        draw_pos.top() + r_top),
                rx,
                start_angle,
                end_angle, false);
              painter->restore();
            }
          } else
          {
            path.moveTo(draw_pos.right() - bdr_right, draw_pos.top() + bdr_top);
            path.lineTo(draw_pos.right(), draw_pos.top());
          }

          if(r_bottom)
          {
            //cairo_line_to(cr, draw_pos.right(),	draw_pos.bottom() - r_bottom);
            //setPenForBorder(painter, borders.bottom);
            //painter->drawLine(area.bottomLeft(), area.bottomRight());

            double start_angle	= 0;
            double end_angle	= start_angle + M_PI / 2.0  / ((double) bdr_bottom / (double) bdr_right + 1);

            {
              painter->save();
              auto rx = r_bottom;
              auto ry = r_bottom;
              painter->scale(1, ry / rx);
              addArc(painter, path,
                FloatPoint(draw_pos.right() - r_bottom,
        draw_pos.bottom() - r_bottom),
                rx,
                start_angle,
                end_angle, false);
              painter->restore();
            }
            {
              painter->save();
              auto rx = r_bottom - bdr_right;
              auto ry = r_bottom - bdr_right + (bdr_right - bdr_bottom);
              painter->scale(1, ry / rx);
              addArc(painter, path,
                FloatPoint(draw_pos.right() - r_bottom,
        draw_pos.bottom() - r_bottom),
                rx,
                end_angle,
                start_angle, true);
              painter->restore();
            }
          } else
          {
            path.lineTo(draw_pos.right(),	draw_pos.bottom());
            path.lineTo(draw_pos.right() - bdr_right,	draw_pos.bottom() - bdr_bottom);
          }
          //cairo_fill(cr);
          painter->fillPath(path, QBrush(getColor(borders.right.color)));
        }


        // draw bottom border
        if(bdr_bottom)
        {
          QPainterPath path;
          //set_color(cr, borders.bottom.color);

          double r_left	= borders.radius.bottom_left_x;
          double r_right	= borders.radius.bottom_right_x;

          if(r_left)
          {
            double start_angle	= M_PI / 2.0;
            double end_angle	= start_angle + M_PI / 2.0  / ((double) bdr_left / (double) bdr_bottom + 1);
            {
              painter->save();
              auto rx = r_left - bdr_bottom + (bdr_bottom - bdr_left);
              auto ry = r_left - bdr_bottom;
              painter->scale(1, ry / rx);
              addArc(painter, path,
                FloatPoint(draw_pos.left() + r_left,
                  draw_pos.bottom() - r_left),
                rx,
                start_angle,
                end_angle, false);
              painter->restore();
            }
            {
              painter->save();
              auto rx = r_left;
              auto ry = r_left;
              painter->scale(1, ry / rx);
              addArc(painter, path,
                FloatPoint(draw_pos.left() + r_left,
                  draw_pos.bottom() - r_left),
                rx,
                end_angle,
                start_angle, true);
              painter->restore();
            }

          } else
          {
            //cairo_move_to(cr, draw_pos.left(), draw_pos.bottom());
            //cairo_line_to(cr, draw_pos.left() + bdr_left, draw_pos.bottom() - bdr_bottom);
            path.moveTo(draw_pos.left(), draw_pos.bottom());
            path.lineTo(draw_pos.left() + bdr_left, draw_pos.bottom() - bdr_bottom);
          }

          if(r_right)
          {
            //cairo_line_to(cr, draw_pos.right() - r_right,	draw_pos.bottom());

            double end_angle	= M_PI / 2.0;
            double start_angle	= end_angle - M_PI / 2.0  / ((double) bdr_right / (double) bdr_bottom + 1);
            {
              painter->save();
              auto rx = r_right;
              auto ry = r_right;
              painter->scale(1, ry / rx);
              addArc(painter, path,
                FloatPoint(draw_pos.right() - r_right,
                  draw_pos.bottom() - r_right),
                rx,
                end_angle,
                start_angle, true);
              painter->restore();
            }
            {
              painter->save();
              auto rx = r_right - bdr_bottom + (bdr_bottom - bdr_right);
              auto ry = r_right - bdr_bottom;
              painter->scale(1, ry / rx);
              addArc(painter, path,
                FloatPoint(draw_pos.right() - r_right,
                  draw_pos.bottom() - r_right),
                rx,
                start_angle,
                end_angle, false);
              painter->restore();
            }

          } else
          {
            //cairo_line_to(cr, draw_pos.right() - bdr_right,	draw_pos.bottom() - bdr_bottom);
            //cairo_line_to(cr, draw_pos.right(),	draw_pos.bottom());
            path.lineTo( draw_pos.right() - bdr_right,	draw_pos.bottom() - bdr_bottom);
            path.lineTo( draw_pos.right(),	draw_pos.bottom());
          }

          //cairo_fill(cr);
          painter->fillPath(path, QBrush(getColor(borders.bottom.color)));
        }

        // draw top border
        if(bdr_top)
        {
          QPainterPath path;
          //set_color(cr, borders.top.color);

          double r_left	= borders.radius.top_left_x;
          double r_right	= borders.radius.top_right_x;

          if(r_left)
          {
            double end_angle	= M_PI * 3.0 / 2.0;
            double start_angle	= end_angle - M_PI / 2.0  / ((double) bdr_left / (double) bdr_top + 1);

            {
              painter->save();
              auto rx = r_left;
              auto ry = r_left;
              painter->scale(1, ry / rx);
              addArc(painter, path,
                FloatPoint(draw_pos.left() + r_left,
                draw_pos.top() + r_left),
                rx,
                end_angle,
                start_angle, true);
              painter->restore();
            }
            {
              painter->save();
              auto rx = r_left - bdr_top + (bdr_top - bdr_left);
              auto ry = r_left - bdr_top;
              painter->scale(1, ry / rx);
              addArc(painter, path,
                FloatPoint(draw_pos.left() + r_left,
                draw_pos.top() + r_left),
                rx,
                start_angle,
                end_angle, false);
              painter->restore();
            }


          } else
          {
            //cairo_move_to(cr, draw_pos.left(), draw_pos.top());
            //cairo_line_to(cr, draw_pos.left() + bdr_left, draw_pos.top() + bdr_top);
            //setPenForBorder(painter, borders.top);
            //painter->drawLine(
            //  QPoint(draw_pos.left(), draw_pos.top()),
            //  QPoint(draw_pos.left() + bdr_left, draw_pos.top() + bdr_top));
            path.moveTo(draw_pos.left(), draw_pos.top());
            path.lineTo(draw_pos.left() + bdr_left, draw_pos.top() + bdr_top);
          }

          if(r_right)
          {
            //cairo_line_to(cr, draw_pos.right() - r_right,	draw_pos.top() + bdr_top);

            double start_angle	= M_PI * 3.0 / 2.0;
            double end_angle	= start_angle + M_PI / 2.0  / ((double) bdr_right / (double) bdr_top + 1);

            {
              painter->save();
              auto rx = r_right - bdr_top + (bdr_top - bdr_right);
              auto ry = r_right - bdr_top;
              painter->scale(1, ry / rx);
              addArc(painter, path,
                FloatPoint(draw_pos.right() - r_right,
                draw_pos.top() + r_right),
                rx,
                start_angle,
                end_angle, false);
              painter->restore();
            }
            {
              painter->save();
              auto rx = r_right;
              auto ry = r_right;
              painter->scale(1, ry / rx);
              addArc(painter, path,
                FloatPoint(draw_pos.right() - r_right,
                draw_pos.top() + r_right),
                rx,
                end_angle,
                start_angle, true);
              painter->restore();
            }
          } else
          {
            //cairo_line_to(cr, draw_pos.right() - bdr_right,	draw_pos.top() + bdr_top);
            //cairo_line_to(cr, draw_pos.right(),	draw_pos.top());
            //setPenForBorder(painter, borders.top);
            //painter->drawLine(
            //  QPoint(draw_pos.right() - bdr_right,	draw_pos.top() + bdr_top),
            //  QPoint(draw_pos.right(),	draw_pos.top()));
            ////path.moveTo(draw_pos.left(), draw_pos.top());
            path.lineTo(draw_pos.right() - bdr_right,	draw_pos.top() + bdr_top);
            path.lineTo(draw_pos.right(),	draw_pos.top());
          }

          painter->fillPath(path, QBrush(getColor(borders.top.color)));

          //cairo_fill(cr);
        }

        // draw left border
        if(bdr_left)
        {
          QPainterPath path;
          //set_color(cr, borders.left.color);

          double r_top	= borders.radius.top_left_x;
          double r_bottom	= borders.radius.bottom_left_x;

          if(r_top)
          {
            double start_angle	= M_PI;
            double end_angle	= start_angle + M_PI / 2.0  / ((double) bdr_top / (double) bdr_left + 1);

            {
              painter->save();
              auto rx = r_top - bdr_left;
              auto ry = r_top - bdr_left + (bdr_left - bdr_top);
              painter->scale(1, ry / rx);
              addArc(painter, path,
                FloatPoint(draw_pos.left() + r_top,
                  draw_pos.top() + r_top),
                rx,
                start_angle,
                end_angle, false);
              painter->restore();
            }
            {
              painter->save();
              auto rx = r_top;
              auto ry = r_top;
              painter->scale(1, ry / rx);
              addArc(painter, path,
                FloatPoint(draw_pos.left() + r_top,
        draw_pos.top() + r_top),
                rx,
                end_angle,
                start_angle, true);
              painter->restore();
            }
          } else
          {
            //cairo_move_to(cr, draw_pos.left() + bdr_left, draw_pos.top() + bdr_top);
            //cairo_line_to(cr, draw_pos.left(), draw_pos.top());
            path.moveTo(draw_pos.left() + bdr_left, draw_pos.top() + bdr_top);
            path.lineTo(draw_pos.left(), draw_pos.top());
          }

          if(r_bottom)
          {
            //cairo_line_to(cr, draw_pos.left(),	draw_pos.bottom() - r_bottom);

            double end_angle	= M_PI;
            double start_angle	= end_angle - M_PI / 2.0  / ((double) bdr_bottom / (double) bdr_left + 1);

            {
              painter->save();
              auto rx = r_bottom;
              auto ry = r_bottom;
              //path.translate(x, y);
              painter->scale(1, ry / rx);
              //path.translate(-x, -y);
              addArc(painter, path,
                FloatPoint(draw_pos.left() + r_bottom,
                  draw_pos.bottom() - r_bottom),
                rx,
                end_angle,
                start_angle, true);
              painter->restore();
            }
            {
              painter->save();
              auto rx = r_bottom - bdr_left;
              auto ry = r_bottom - bdr_left + (bdr_left - bdr_bottom);
              painter->scale(1, ry / rx);
              addArc(painter, path,
                FloatPoint(draw_pos.left() + r_bottom,
                  draw_pos.bottom() - r_bottom),
                rx,
                start_angle,
                end_angle, false);
              painter->restore();
            }
          } else
          {
            //cairo_line_to(cr, draw_pos.left(),	draw_pos.bottom());
            //cairo_line_to(cr, draw_pos.left() + bdr_left,	draw_pos.bottom() - bdr_bottom);
            path.lineTo(draw_pos.left(),	draw_pos.bottom());
            path.lineTo(draw_pos.left() + bdr_left,	draw_pos.bottom() - bdr_bottom);
          }

          //cairo_fill(cr);
          painter->fillPath(path, QBrush(getColor(borders.left.color)));
        }
        //cairo_restore(cr);

#endif
    }

    painter->restore();
}

void container_qt5::draw_background(litehtml::uint_ptr hdc, const litehtml::background_paint& bg)
{
    Q_ASSERT(hdc);

    //qDebug() << "draw_background" << bg.color.red;

    QPainter* painter = (QPainter*)hdc;
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
      return;
    }

    QImage* img = getImage( bg.image.c_str(), bg.baseurl.c_str() );
    if (!img) {
      painter->fillRect(clipRect, dummyColor );
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
    QPainter *painter = (QPainter *) hdc;

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
    delete(font);
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

void litehtmlWidget::mouseMoveEvent(QMouseEvent *event)
{
    litehtml::position::vector redraw_boxes;
    container->setLastMouseCoords(event->x(), event->y(), event->x(), event->y());

    if (container->getDocument()->on_mouse_over(event->x(), event->y(), event->x(), event->y(), redraw_boxes)) {
      repaint();
    }
}

void litehtmlWidget::mousePressEvent(QMouseEvent *event)
{
    litehtml::position::vector redraw_boxes;
    container->setLastMouseCoords(event->x(), event->y(), event->x(), event->y());

    if (container->getDocument()->on_lbutton_down(event->x(), event->y(), event->x(), event->y(), redraw_boxes)) {
      repaint();
    }
}

void litehtmlWidget::mouseReleaseEvent(QMouseEvent *event)
{
    litehtml::position::vector redraw_boxes;
    container->setLastMouseCoords(event->x(), event->y(), event->x(), event->y());

    if (container->getDocument()->on_lbutton_up(event->x(), event->y(), event->x(), event->y(), redraw_boxes)) {
      repaint();
    }
}

void litehtmlWidget::resizeEvent(QResizeEvent *event)
{
  container->_doc->media_changed();

  /*QRect	rc = rect( );

  container->_doc->render( rc.width() );
  container->_doc->render( rc.width() );*/
}

void litehtmlWidget::wheelEvent(QWheelEvent *event)
{

}
