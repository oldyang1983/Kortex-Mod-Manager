#include "stdafx.h"
#include "InfoSection.h"
#include "ModPackageProject.h"
#include <Kortex/Application.hpp>
#include "Utility/KAux.h"
#include <Kortex/ModManager.hpp>

namespace Kortex::PackageProject
{
	InfoSection::InfoSection(ModPackageProject& project)
		:ProjectSection(project)
	{
	}
	InfoSection::~InfoSection()
	{
	}
}