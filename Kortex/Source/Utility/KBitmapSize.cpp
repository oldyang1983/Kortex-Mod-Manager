#include "stdafx.h"
#include "Utility/KBitmapSize.h"
#include "Utility/KAux.h"

namespace
{
	int ProcessMargin(int sizeValue, int margin)
	{
		return margin >= 0 ? sizeValue - margin : -1;
	}
}

KBitmapSize& KBitmapSize::FromSystemIcon()
{
	m_Width = wxSystemSettings::GetMetric(wxSYS_ICON_X);
	m_Height = wxSystemSettings::GetMetric(wxSYS_ICON_Y);

	return *this;
}
KBitmapSize& KBitmapSize::FromSystemSmallIcon()
{
	m_Width = wxSystemSettings::GetMetric(wxSYS_SMALLICON_X);
	m_Height = wxSystemSettings::GetMetric(wxSYS_SMALLICON_Y);

	return *this;
}

wxBitmap KBitmapSize::ScaleBitmapAspect(const wxBitmap& bitmap, int marginsX, int marginsY) const
{
	return KAux::ScaleImageAspect(bitmap, ProcessMargin(m_Width, marginsX), ProcessMargin(m_Height, marginsY));
}
wxBitmap KBitmapSize::ScaleBitmapStretch(const wxBitmap& bitmap, int marginsX, int marginsY) const
{
	return wxBitmap(bitmap.ConvertToImage().Rescale(ProcessMargin(m_Width, marginsX), ProcessMargin(m_Height, marginsY), wxIMAGE_QUALITY_HIGH), 32);
}
