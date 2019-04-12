#include "html.h"
#include "background.h"

litehtml::background::background(void)
{
	m_attachment	= background_attachment_scroll;
	m_repeat		= background_repeat_repeat;
	m_clip			= background_box_border;
	m_origin		= background_box_padding;
	m_color.alpha	= 0;
	m_color.red		= 0;
	m_color.green	= 0;
	m_color.blue	= 0;
  m_box_shadow = nullptr;
}

litehtml::background::background( const background& val )
{
	m_image			= val.m_image;
	m_baseurl		= val.m_baseurl;
	m_color			= val.m_color;
	m_attachment	= val.m_attachment;
	m_position		= val.m_position;
	m_repeat		= val.m_repeat;
	m_clip			= val.m_clip;
	m_origin		= val.m_origin;
  m_box_shadow		= val.m_box_shadow;
}

litehtml::background::~background(void)
{
}

litehtml::background& litehtml::background::operator=( const background& val )
{
	m_image			= val.m_image;
	m_baseurl		= val.m_baseurl;
	m_color			= val.m_color;
	m_attachment	= val.m_attachment;
	m_position		= val.m_position;
	m_repeat		= val.m_repeat;
	m_clip			= val.m_clip;
	m_origin		= val.m_origin;
  m_box_shadow		= val.m_box_shadow;
	return *this;
}


litehtml::background_paint::background_paint() : color(0, 0, 0, 0)
{
	position_x		= 0;
	position_y		= 0;
	attachment		= background_attachment_scroll;
	repeat			= background_repeat_repeat;
	is_root			= false;
  box_shadow = nullptr;
}

litehtml::background_paint::background_paint( const background_paint& val )
{
	image			= val.image;
	baseurl			= val.baseurl;
	attachment		= val.attachment;
	repeat			= val.repeat;
	color			= val.color;
	clip_box		= val.clip_box;
	origin_box		= val.origin_box;
	border_box		= val.border_box;
	border_radius	= val.border_radius;
	image_size		= val.image_size;
	position_x		= val.position_x;
	position_y		= val.position_y;
	is_root			= val.is_root;
  box_shadow			= val.box_shadow;
}

void litehtml::background_paint::operator=( const background& val )
{
	attachment	= val.m_attachment;
	baseurl		= val.m_baseurl;
	image		= val.m_image;
	repeat		= val.m_repeat;
	color		= val.m_color;
  box_shadow		= val.m_box_shadow;
}
