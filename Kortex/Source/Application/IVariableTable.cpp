#include "stdafx.h"
#include "IVariableTable.h"
#include "Utility/Log.h"
#include <KxFramework/KxShell.h>
#include <KxFramework/KxSystem.h>

namespace Kortex
{
	wxString IVariableTable::Expand(const wxString& variables) const
	{
		if (!variables.IsEmpty() && !IsEmpty())
		{
			wxString out = variables;
			size_t entryStartPos = wxString::npos;
			size_t varNameStartPos = wxString::npos;
			bool isTranslationVar = false;
			bool isShellFolderVar = false;
			bool isEnvVar = false;

			for (size_t i = 0; i < out.Length(); i++)
			{
				if (out[i] == wxS('$'))
				{
					entryStartPos = i;

					// Check for 'T'
					if (i + 1 < out.Length())
					{
						auto cNext = out[i + 1];
						if (cNext == wxS('T'))
						{
							isTranslationVar = true;
							i++;
						}
					}

					// Look for 'SHF'
					if (i + 3 < out.Length())
					{
						auto c1 = out[i + 1];
						auto c2 = out[i + 2];
						auto c3 = out[i + 3];

						if (c1 == wxS('S') && c2 == wxS('H') && c3 == wxS('F'))
						{
							isShellFolderVar = true;
							i += 2;
						}
					}

					// Look for 'ENV'
					if (i + 3 < out.Length())
					{
						auto c1 = out[i + 1];
						auto c2 = out[i + 2];
						auto c3 = out[i + 3];

						if (c1 == wxS('E') && c2 == wxS('N') && c3 == wxS('V'))
						{
							isEnvVar = true;
							i += 3;
						}
					}
				}

				// We are at the beginning of the variable name
				if (entryStartPos != wxString::npos && out[i] == wxS('('))
				{
					varNameStartPos = i + 1;
				}

				// All data collected, do replace
				if (entryStartPos != wxString::npos && varNameStartPos != wxString::npos && out[i] == wxS(')'))
				{
					size_t varEntryLength = i - entryStartPos + 1;
					size_t varNameLength = i - varNameStartPos;
					wxString varName = out.Mid(varNameStartPos, varNameLength);
					if (!varName.IsEmpty())
					{
						wxString value;
						if (isTranslationVar)
						{
							value = KTr(varName);
						}
						else if (isShellFolderVar)
						{
							KxShellFolderID id = KxShell::GetShellFolderID(wxS("SHF_") + varName);
							if (id != KxSHF_MAX_ID)
							{
								value = KxShell::GetFolder(id);
							}
							else
							{
								Utility::Log::LogWarning("Can't expand shell folder variable \"%1\" (invalid variable name)", varName);
							}
						}
						else if (isEnvVar)
						{
							value = KxSystem::GetEnvironmentVariable(varName);
						}
						else
						{
							value = GetVariable(varName).AsString();
						}

						if (!value.IsEmpty())
						{
							out.replace(entryStartPos, varEntryLength, value);

							// Set 'i' value to end of replacement
							i = entryStartPos + value.Length() - 1;
						}
					}

					// Reset parser state
					entryStartPos = wxString::npos;
					varNameStartPos = wxString::npos;
					isTranslationVar = false;
					isShellFolderVar = false;
					isEnvVar = false;
				}
			}
			return out;
		}
		return variables;
	}
}
