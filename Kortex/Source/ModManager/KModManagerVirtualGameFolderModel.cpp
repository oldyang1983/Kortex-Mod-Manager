#include "stdafx.h"
#include "KModManagerVirtualGameFolderModel.h"
#include "KModManager.h"
#include "KModManagerDispatcher.h"
#include "KModManagerWorkspace.h"
#include "KFileTreeNode.h"
#include "KModEntry.h"
#include "KComparator.h"
#include "KAux.h"
#include <KxFramework/KxShell.h>

enum ColumnID
{
	Name,
	PartOf,
	Size,
	ModificationDate,
};

void KModManagerVirtualGameFolderModel::OnInitControl()
{
	/* View */
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_SELECTED, &KModManagerVirtualGameFolderModel::OnSelectItem, this);
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &KModManagerVirtualGameFolderModel::OnActivateItem, this);
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &KModManagerVirtualGameFolderModel::OnContextMenu, this);
	GetView()->Bind(KxEVT_DATAVIEW_ITEM_EXPANDING, &KModManagerVirtualGameFolderModel::OnExpandingItem, this);
	GetView()->Bind(KxEVT_DATAVIEW_COLUMN_HEADER_RIGHT_CLICK, [this](KxDataViewEvent& event)
	{
		KxMenu menu;
		if (GetView()->CreateColumnSelectionMenu(menu))
		{
			GetView()->OnColumnSelectionMenu(menu);
		}
	});
	GetView()->SetIndent(20);

	/* Columns */
	KxDataViewColumnFlags flags = KxDV_COL_DEFAULT_FLAGS|KxDV_COL_SORTABLE;
	{
		auto info = GetView()->AppendColumn<KxDataViewBitmapTextRenderer>(T("Generic.Name"), ColumnID::Name, KxDATAVIEW_CELL_INERT, 300, flags);
	}
	{
		auto info = GetView()->AppendColumn<KxDataViewTextRenderer>(T("Generic.PartOf"), ColumnID::PartOf, KxDATAVIEW_CELL_INERT, 300, flags);
		info.GetColumn()->SortAscending();
	}
	GetView()->AppendColumn<KxDataViewBitmapTextRenderer>(T("Generic.Size"), ColumnID::Size, KxDATAVIEW_CELL_INERT, 100, flags);
	GetView()->AppendColumn<KxDataViewBitmapTextRenderer>(T("Generic.ModificationDate"), ColumnID::ModificationDate, KxDATAVIEW_CELL_INERT, 125, flags);
}

bool KModManagerVirtualGameFolderModel::IsContainer(const KxDataViewItem& item) const
{
	if (const ModelNode* node = GetNode(item))
	{
		return node->HasChildren();
	}
	return false;
}
KxDataViewItem KModManagerVirtualGameFolderModel::GetParent(const KxDataViewItem& item) const
{
	if (const ModelNode* node = GetNode(item))
	{
		if (node->HasParent())
		{
			return MakeItem(node->GetParent());
		}
	}
	return KxDataViewItem();
}
void KModManagerVirtualGameFolderModel::GetChildren(const KxDataViewItem& item, KxDataViewItem::Vector& children) const
{
	// Root item, read groups
	if (item.IsTreeRootItem())
	{
		children.reserve(m_TreeItems.size());
		for (const auto& node: m_TreeItems)
		{
			children.push_back(MakeItem(*node));
		}
		return;
	}

	// Group item, read entries
	if (const ModelNode* node = GetNode(item))
	{
		children.reserve(node->GetChildrenCount());
		for (const auto& childNode: node->GetChildren())
		{
			children.push_back(MakeItem(*childNode));
		}
	}
}

void KModManagerVirtualGameFolderModel::GetEditorValue(wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column) const
{
}
void KModManagerVirtualGameFolderModel::GetValue(wxAny& value, const KxDataViewItem& item, const KxDataViewColumn* column) const
{
	if (const ModelNode* node = GetNode(item))
	{
		const KFileTreeNode& fileNode = node->GetFileNode();
		switch (column->GetID())
		{
			case ColumnID::Name:
			{
				value = KxDataViewBitmapTextValue(fileNode.GetName(), fileNode.IsDirectory() ? KGetBitmap(KIMG_FOLDER) : KGetBitmap(KIMG_DOCUMENT));
				break;
			}
			case ColumnID::PartOf:
			{
				if (const KModEntry* modEntry = node->GetMod())
				{
					value = modEntry->GetName();
				}
				break;
			}
			case ColumnID::Size:
			{
				if (fileNode.IsFile())
				{
					value = KxFile::FormatFileSize(fileNode.GetFileSize(), 2);
				}
				break;
			}
			case ColumnID::ModificationDate:
			{
				value = KAux::FormatDateTime(fileNode.GetItem().GetModificationTime());
				break;
			}
		};
	}
}
bool KModManagerVirtualGameFolderModel::SetValue(const wxAny& data, const KxDataViewItem& item, const KxDataViewColumn* column)
{
	return false;
}
bool KModManagerVirtualGameFolderModel::IsEnabled(const KxDataViewItem& item, const KxDataViewColumn* column) const
{
	return true;
}

bool KModManagerVirtualGameFolderModel::GetItemAttributes(const KxDataViewItem& item, const KxDataViewColumn* column, KxDataViewItemAttributes& attributes, KxDataViewCellState cellState) const
{
	const ModelNode* node = GetNode(item);
	switch (column->GetID())
	{
		case ColumnID::PartOf:
		{
			if (node->GetMod() && cellState & KxDATAVIEW_CELL_HIGHLIGHTED && column->IsHotTracked())
			{
				attributes.SetUnderlined();
				return true;
			}
			break;
		}
	};
	return false;
}
bool KModManagerVirtualGameFolderModel::Compare(const KxDataViewItem& item1, const KxDataViewItem& item2, const KxDataViewColumn* column) const
{
	const ModelNode* node1 = GetNode(item1);
	const ModelNode* node2 = GetNode(item2);
	const KFileTreeNode& fileNode1 = node1->GetFileNode();
	const KFileTreeNode& fileNode2 = node2->GetFileNode();

	// Folders first
	if (fileNode1.IsFile() != fileNode2.IsFile())
	{
		return fileNode1.IsFile() < fileNode2.IsFile();
	}

	switch (column ? column->GetID() : ColumnID::PartOf)
	{
		case ColumnID::Name:
		{
			return KComparator::KCompare(fileNode1.GetName(), fileNode2.GetName()) < 0;
		}
		case ColumnID::PartOf:
		{
			const KModEntry* mod1 = node1->GetMod();
			const KModEntry* mod2 = node2->GetMod();
			if (mod1 && mod2)
			{
				return mod1->GetOrderIndex() < mod2->GetOrderIndex();
			}
			return false;
		}
		case ColumnID::Size:
		{
			if (fileNode1.IsFile() && fileNode2.IsFile())
			{
				return fileNode1.GetFileSize() < fileNode2.GetFileSize();
			}
		}
		case ColumnID::ModificationDate:
		{
			return fileNode1.GetItem().GetModificationTime() < fileNode2.GetItem().GetModificationTime();
		}
	};
	return false;
}

void KModManagerVirtualGameFolderModel::OnSelectItem(KxDataViewEvent& event)
{
	KxDataViewItem item = event.GetItem();
	KxDataViewColumn* column = event.GetColumn();
	ModelNode* node = GetNode(item);

	if (node && column && column->GetID() == ColumnID::PartOf)
	{
		KModManagerWorkspace* workspace = KModManagerWorkspace::GetInstance();
		wxWindowUpdateLocker lock(workspace);

		const KModEntry* modEntry = node->GetMod();
		workspace->HighlightMod();

		if (modEntry && node->GetFileNode().IsFile())
		{
			workspace->HighlightMod(modEntry);
		}
	}
}
void KModManagerVirtualGameFolderModel::OnActivateItem(KxDataViewEvent& event)
{
	KxDataViewItem item = event.GetItem();
	KxDataViewColumn* column = event.GetColumn();
	ModelNode* node = GetNode(item);

	if (node && column)
	{
		if (node->GetFileNode().IsDirectory())
		{
			KxShell::Execute(GetViewTLW(), node->GetFileNode().GetFullPath(), "open");
		}
		else
		{
			KxShell::OpenFolderAndSelectItem(node->GetFileNode().GetFullPath());
		}
	}
}
void KModManagerVirtualGameFolderModel::OnContextMenu(KxDataViewEvent& event)
{
}
void KModManagerVirtualGameFolderModel::OnExpandingItem(KxDataViewEvent& event)
{
	ModelNode* rootNode = GetNode(event.GetItem());
	if (rootNode && !rootNode->IsVisited())
	{
		wxString searchPath = rootNode->GetFileNode().GetRelativePath();
		KModManagerDispatcher::FinderHash hash;
		KModManager::GetDispatcher().IterateOverMods([this, &event, rootNode, &hash, &searchPath](const KModEntry& modEntry)
		{
			for (const KFileTreeNode* node: KModManager::GetDispatcher().FindFiles(searchPath, KxFile::NullFilter, KxFS_ALL, false, &hash))
			{
				if (node->IsDirectory() || KAux::CheckSearchMask(m_SearchMask, node->GetName()))
				{
					ModelNode& modelNode = *rootNode->GetChildren().emplace_back(new ModelNode(rootNode, *node, &node->GetMod()));
					ItemAdded(event.GetItem(), MakeItem(modelNode));
				}
			}
			return false;
		}, KModManagerDispatcher::IterationOrder::Reversed);

		rootNode->MarkVisited();
	}
}

KModManagerVirtualGameFolderModel::KModManagerVirtualGameFolderModel()
{
	SetDataViewFlags(KxDataViewCtrl::DefaultStyle|KxDV_DOUBLE_CLICK_EXPAND);
}

void KModManagerVirtualGameFolderModel::RefreshItems()
{
	m_TreeItems.clear();
	ItemsCleared();

	KModManagerDispatcher::FinderHash hash;
	KModManager::GetDispatcher().IterateOverMods([this, &hash](const KModEntry& modEntry)
	{
		for (const KFileTreeNode* node: KModManager::GetDispatcher().FindFiles(modEntry, KxFile::NullFilter, KxFS_ALL, false, &hash))
		{
			if (node->IsDirectory() || KAux::CheckSearchMask(m_SearchMask, node->GetName()))
			{
				ModelNode& modelNode = *m_TreeItems.emplace_back(new ModelNode(NULL, *node, &modEntry));
				ItemAdded(MakeItem(modelNode));
			}
		}
		return false;
	}, KModManagerDispatcher::IterationOrder::Reversed);

	// Reset scrolling
	GetView()->Scroll(0, 0);
}
bool KModManagerVirtualGameFolderModel::SetSearchMask(const wxString& mask)
{
	return KAux::SetSearchMask(m_SearchMask, mask);
}
