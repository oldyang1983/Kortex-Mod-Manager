#include "stdafx.h"
#include "KSaveFileBethesdaFalloutNV.h"
#include "KSaveFile.h"
#include "KApp.h"
#include <KxFramework/KxFileStream.h>

bool KSaveFileBethesdaFalloutNV::DoInitializeSaveData()
{
	KxFileStream stream(GetFilePath(), KxFileStream::Access::Read, KxFileStream::Disposition::OpenExisting, KxFileStream::Share::Read);
	if (stream.IsOk())
	{
		if (stream.ReadStringASCII(11) == wxS("FO3SAVEGAME"))
		{
			// Skip 'headerSize' field
			stream.Skip<uint32_t>();

			// Unknown, possibly file version number, always 0x30
			m_SaveVersion = stream.ReadObject<uint32_t>();

			// Skip separator
			stream.Skip<uint8_t>();

			// Read game language (seems to have fixed length)
			m_BasicInfo.emplace_back(stream.ReadStringACP(64), T("SaveManager.Info.Language"));

			// Seek to screenshort dimensions data
			stream.SeekFromStart(85);
			uint32_t width = stream.ReadObject<uint32_t>();
			stream.Skip<uint8_t>();
			uint32_t height = stream.ReadObject<uint32_t>();

			// Skip another separator
			stream.Skip<uint8_t>();

			// Read save index
			m_BasicInfo.emplace_back(std::to_string(stream.ReadObject<uint32_t>()), T("SaveManager.Info.SaveIndex"));

			auto ReadWZString = [this, &stream](const wxString& fieldName)
			{
				stream.Skip<uint8_t>();
				uint16_t length = stream.ReadObject<uint16_t>();
				stream.Skip<uint8_t>();
				m_BasicInfo.emplace_back(stream.ReadStringACP(length), T(fieldName));
			};

			// Read name
			ReadWZString("SaveManager.Info.Name");
			ReadWZString("SaveManager.Info.Karma");
			
			// Read level
			stream.Skip<uint8_t>();
			m_BasicInfo.emplace_back(std::to_string(stream.ReadObject<uint32_t>()), T("SaveManager.Info.Level"));

			// Read location
			ReadWZString("SaveManager.Info.Location");

			// Read game time
			ReadWZString("SaveManager.Info.TimeInGame");

			// Skip separator
			stream.Skip<uint8_t>();

			// Read image
			m_Bitmap = ReadBitmapRGB(stream.ReadVector<uint8_t>(width * height * 3), width, height);

			// Skip 'formVersion' and 'pluginInfoSize' fields
			stream.Skip<uint8_t, uint32_t>();

			// Read plugins list
			size_t count = stream.ReadObject<uint8_t>();
			for (size_t i = 0; i < count; i++)
			{
				stream.Skip<uint8_t>();
				uint16_t length = stream.ReadObject<uint16_t>();
				stream.Skip<uint8_t>();
				m_PluginsList.push_back(stream.ReadStringACP(length));
			}
			return true;
		}
	}
	return false;
}
