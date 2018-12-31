#pragma once
#include "stdafx.h"
#include "IGameProfile.h"
#include "Utility/KImageProvider.h"
#include "Utility/KDataViewListModel.h"
#include "Utility/KLabeledValue.h"
#include <KxFramework/KxStdDialog.h>
class KxButton;
class KxCheckBox;

namespace Kortex
{
	class KProfileEditor: public KxDataViewVectorListModelEx<IGameProfile::Vector, KxDataViewListModelEx>
	{
		private:
			bool m_IsModified = false;
			wxString m_NewCurrentProfile;

		protected:
			virtual void OnInitControl() override;

			virtual void GetEditorValueByRow(wxAny& data, size_t row, const KxDataViewColumn* column) const override;
			virtual void GetValueByRow(wxAny& data, size_t row, const KxDataViewColumn* column) const override;
			virtual bool SetValueByRow(const wxAny& data, size_t row, const KxDataViewColumn* column) override;
			virtual bool IsEnabledByRow(size_t row, const KxDataViewColumn* column) const override;

			void OnActivate(KxDataViewEvent& event);
			void MarkModified()
			{
				m_IsModified = true;
			}
			void SetNewProfile(const wxString& id)
			{
				m_NewCurrentProfile = id;
			}

		public:
			KProfileEditor();

		public:
			bool IsModified() const
			{
				return m_IsModified;
			}
			const wxString& GetNewProfile() const
			{
				return m_NewCurrentProfile;
			}

			const IGameProfile* GetDataEntry(size_t i) const
			{
				if (i < GetItemCount())
				{
					return &*GetDataVector()->at(i);
				}
				return nullptr;
			}
			IGameProfile* GetDataEntry(size_t i)
			{
				if (i < GetItemCount())
				{
					return &*GetDataVector()->at(i);
				}
				return nullptr;
			}
	};
}

namespace Kortex
{
	class KModListManagerEditorDialog: public KxStdDialog, public KProfileEditor
	{
		private:
			wxWindow* m_ViewPane = nullptr;
			KxButton* m_AddButton = nullptr;
			KxButton* m_CopyButton = nullptr;
			KxButton* m_RemoveButton = nullptr;

		private:
			virtual wxWindow* GetDialogMainCtrl() const override
			{
				return m_ViewPane;
			}
		
			void OnSelectItem(KxDataViewEvent& event);
			void OnAddList(wxCommandEvent& event);
			void OnCopyList(wxCommandEvent& event);
			void OnRemoveList(wxCommandEvent& event);

		public:
			KModListManagerEditorDialog(wxWindow* parent);
			virtual ~KModListManagerEditorDialog();
	};
}
