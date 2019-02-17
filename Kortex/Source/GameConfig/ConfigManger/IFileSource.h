#pragma once
#include "stdafx.h"
#include "Common.h"
#include "DataType.h"

namespace Kortex::GameConfig
{
	class IFileSource: public RTTI::IInterface<IFileSource>
	{
		protected:
			wxString ResolveFSLocation(const wxString& path) const;

		public:
			virtual wxString GetFileName() const = 0;
			virtual wxString GetFilePath() const = 0;

			wxString GetResolvedFileName() const;
			wxString GetResolvedFilePath() const;
	};
}