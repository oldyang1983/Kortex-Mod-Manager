#pragma once
#include "stdafx.h"
#include "Application/Resources/ImageResourceID.h"
#include "Utility/KLabeledValue.h"
#include "Utility/KWithBitmap.h"
#include "Utility/KAux.h"
#include <KxFramework/KxVersion.h>
class KPackageProject;

namespace Kortex
{
	class FileTreeNode;
	class ModTagStore;
	class ModSourceStore;

	class IGameMod: public RTTI::IInterface<IGameMod>
	{
		friend class IModManager;
		friend class IModDispatcher;

		public:
			using Vector = std::vector<std::unique_ptr<IGameMod>>;
			using RefVector = std::vector<IGameMod*>;
			using CRefVector = std::vector<const IGameMod*>;

		private:
			intptr_t m_Priority = -1;

		protected:
			void SetPriority(intptr_t value)
			{
				m_Priority = value;
			}
			void ModPriority(intptr_t value)
			{
				m_Priority += value;
			}

		public:
			static wxString GetSignatureFromID(const wxString& id);

		public:
			virtual bool IsOK() const = 0;
			virtual bool Save() = 0;

			virtual bool LoadUsingSignature(const wxString& signature) = 0;
			virtual bool LoadUsingID(const wxString& id);
			virtual bool CreateFromProject(const KPackageProject& config);
			
			intptr_t GetPriority() const
			{
				return m_Priority;
			}
			virtual intptr_t GetDisplayOrder() const
			{
				return m_Priority;
			}

			virtual wxString GetSignature() const = 0;
			virtual wxString GetID() const = 0;
			virtual void SetID(const wxString& id) = 0;
			
			virtual wxString GetName() const = 0;
			virtual void SetName(const wxString& value) = 0;
			wxString GetSafeName() const;
			
			virtual wxString GetAuthor() const = 0;
			virtual void SetAuthor(const wxString& value) = 0;

			virtual KxVersion GetVersion() const = 0;
			virtual void SetVersion(const KxVersion& value) = 0;

			virtual wxString GetDescription() const = 0;
			virtual void SetDescription(const wxString& value) = 0;

			virtual wxDateTime GetInstallTime() const = 0;
			virtual void SetInstallTime(const wxDateTime& date) = 0;

			virtual wxDateTime GetUninstallTime() const = 0;
			virtual void SetUninstallTime(const wxDateTime& date) = 0;

			virtual const ModSourceStore& GetModSourceStore() const = 0;
			virtual ModSourceStore& GetModSourceStore() = 0;

			virtual const ModTagStore& GetTagStore() const = 0;
			virtual ModTagStore& GetTagStore() = 0;

			virtual wxString GetPackageFile() const = 0;
			virtual void SetPackageFile(const wxString& value) = 0;
			bool IsPackageFileExist() const;
			
			virtual const FileTreeNode& GetFileTree() const = 0;
			virtual void ClearFileTree() = 0;
			virtual void UpdateFileTree() = 0;

			virtual bool IsActive() const = 0;
			virtual void SetActive(bool value) = 0;
			virtual bool IsInstalled() const = 0;

			virtual bool HasColor() const = 0;
			virtual KxColor GetColor() const = 0;
			virtual void SetColor(const KxColor& color) = 0;
			virtual ResourceID GetIcon() const = 0;

			virtual bool IsLinkedMod() const = 0;
			virtual void UnlinkLocation() = 0;
			virtual void LinkLocation(const wxString& path) = 0;

			wxString GetRootDir() const;
			wxString GetDescriptionFile() const;
			wxString GetInfoFile() const;
			wxString GetImageFile() const;
			wxString GetDefaultModFilesDir() const;
			virtual wxString GetModFilesDir() const = 0;
	};
}
