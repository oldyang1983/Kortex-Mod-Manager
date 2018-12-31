#pragma once
#include "stdafx.h"
#include "Utility/KDataViewListModel.h"
#include "IPluginManager.h"
#include "BethesdaPluginManager.h"

namespace Kortex::PluginManager
{
	class IDisplayModel;
	class PluginViewDNDObject;

	class PluginViewModel:
		public KxDataViewVectorListModelEx<IGamePlugin::Vector, KxDataViewListModelEx>,
		public KxDataViewModelExDragDropEnabled<PluginViewDNDObject>
	{
		private:
			IGamePlugin::Vector& m_Entries;
			wxString m_SearchMask;

		private:
			virtual void OnInitControl() override;

			virtual void GetChildren(const KxDataViewItem& item, KxDataViewItem::Vector& children) const override;
			virtual void GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const override;
			virtual bool SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column) override;
			virtual bool IsEditorEnabledByRow(size_t row, const KxDataViewColumn* column) const override;
			virtual bool IsEnabledByRow(size_t row, const KxDataViewColumn* column) const override;
			virtual bool GetItemAttributesByRow(size_t row, const KxDataViewColumn* column, KxDataViewItemAttributes& attributes, KxDataViewCellState cellState) const override;
			virtual bool HasDefaultCompare() const override
			{
				return true;
			}
			virtual bool CompareByRow(size_t row1, size_t row2, const KxDataViewColumn* column) const override;

			void OnSelectItem(KxDataViewEvent& event);
			void OnActivateItem(KxDataViewEvent& event);
			void OnContextMenu(KxDataViewEvent& event);

			virtual KxDataViewCtrl* GetViewCtrl() const override
			{
				return GetView();
			}
			virtual bool OnDragItems(KxDataViewEventDND& event) override;
			virtual bool OnDropItems(KxDataViewEventDND& event) override;
			bool CanDragDropNow() const;

		public:
			PluginViewModel();

		public:
			void ChangeNotify();
			void SetDataVector();
			void SetDataVector(IGamePlugin::Vector& array);
			IDisplayModel* GetDisplayModel() const;

			const IGamePlugin* GetDataEntry(size_t index) const
			{
				return index < m_Entries.size() ? &*m_Entries[index] : nullptr;
			}
			IGamePlugin* GetDataEntry(size_t index)
			{
				return index < m_Entries.size() ? &*m_Entries[index] : nullptr;
			}
			KxDataViewItem GetItemByEntry(const IGamePlugin* entry) const
			{
				auto it = std::find_if(m_Entries.begin(), m_Entries.end(), [entry](const auto& v)
				{
					return v.get() == entry;
				});
				if (it != m_Entries.end())
				{
					return GetItem(std::distance(m_Entries.begin(), it));
				}
				return KxDataViewItem();
			}

			IGamePlugin* GetSelectedPlugin()
			{
				return GetDataEntry(GetSelectedPluginIndex());
			}
			int GetSelectedPluginIndex() const
			{
				size_t index = GetRow(GetView()->GetSelection());
				return index != (size_t)-1 ? index : (int)-1;
			}
			
			void SetAllEnabled(bool value);
			void UpdateUI();
			bool SetSearchMask(const wxString& mask);
	};
}

namespace Kortex::PluginManager
{
	class PluginViewDNDObject: public KxDataViewModelExDragDropData
	{
		private:
			IGamePlugin::RefVector m_Entries;

		public:
			PluginViewDNDObject(size_t count = 0)
			{
				m_Entries.reserve(count);
			}

		public:
			const IGamePlugin::RefVector& GetEntries() const
			{
				return m_Entries;
			}
			void AddEntry(IGamePlugin* entry)
			{
				m_Entries.push_back(entry);
			}
	};
}
