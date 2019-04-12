#ifndef LH_BACKGROUND_H
#define LH_BACKGROUND_H

#include "types.h"
#include "attributes.h"
#include "css_length.h"
#include "css_position.h"
#include "web_color.h"
#include "borders.h"
#include "shadowdata.h"

namespace litehtml
{
	class background
	{
	public:
		tstring					m_image;
		tstring					m_baseurl;
		web_color				m_color;
		background_attachment	m_attachment;
		css_position			m_position;
		background_repeat		m_repeat;
		background_box			m_clip;
		background_box			m_origin;
		css_border_radius		m_radius;
    ShadowData* m_box_shadow;

	public:
		background(void);
		background(const background& val);
		~background(void);

		background& operator=(const background& val);
	};

/*
 *

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

    bool hasOpaqueBackground = color.alpha() == 255;

    ShadowStyle shadowStyle = ShadowStyle::Normal;
    // https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/WebCore/css/CSSStyleSelector.cpp#L4832
    ShadowData* boxShadow = nullptr;//new ShadowData(5, 5, 50, 50, shadowStyle, color);//Color(255,0,255,255) );//color.isValid() ? color : Color::transparent);
    //boxShadow->setNext()
    //boxShadow->set
*/

	class background_paint
	{
	public:
		tstring					image;
		tstring					baseurl;
		background_attachment	attachment;
		background_repeat		repeat;
		web_color				color;
		position				clip_box;
		position				origin_box;
		position				border_box;
		border_radiuses			border_radius;
		size					image_size;
		int						position_x;
		int						position_y;
		bool					is_root;
    ShadowData* box_shadow;
	public:
		background_paint();
		background_paint(const background_paint& val);
		void operator=(const background& val);
	};

}

#endif  // LH_BACKGROUND_H
