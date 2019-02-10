#include "stdafx.h"
#include "DefaultGameConfigManager.h"
#include "Definition.h"
#include "Workspace.h"
#include <Kortex/Application.hpp>
#include <Kortex/Events.hpp>

namespace Kortex::GameConfig
{
	void DefaultGameConfigManager::LoadGroup(const KxXMLNode& definitionNode, ItemGroup& group)
	{
		KxXMLNode groupsNode = definitionNode.GetFirstChildElement(wxS("Groups"));
		for (KxXMLNode node = groupsNode.GetFirstChildElement(); node.IsOK(); node = node.GetNextSiblingElement())
		{
			if (node.GetAttribute(wxS("ID")) == group.GetID())
			{
				group.OnLoadInstance(node);
				return;
			}
		}
	}
	void DefaultGameConfigManager::OnChangeProfile(GameInstance::ProfileEvent& event)
	{
		Load();

		if (Workspace* workspace = Workspace::GetInstance())
		{
			workspace->ReloadWorkspace();
		}
	}

	void DefaultGameConfigManager::OnInit()
	{
		IConfigManager::OnInit();
		IEvent::Bind(Events::ProfileSelected, &DefaultGameConfigManager::OnChangeProfile, this);
	}
	void DefaultGameConfigManager::OnExit()
	{
		IConfigManager::OnExit();
	}
	void DefaultGameConfigManager::OnLoadInstance(IGameInstance& instance, const KxXMLNode& managerNode)
	{
		IConfigManager::OnLoadInstance(instance, managerNode);
		if (LoadTranslation(m_Translation, "GameConfig"))
		{
			m_Translator.Push(m_Translation);
		}

		const KxXMLNode definitionsNode = managerNode.GetFirstChildElement("Definitions");
		for (KxXMLNode defNode = definitionsNode.GetFirstChildElement(); defNode.IsOK(); defNode = defNode.GetNextSiblingElement())
		{
			wxString id = defNode.GetAttribute("ID");

			auto definition = std::make_unique<Definition>(*this, id, GetDefinitionFileByID(id));
			if (definition->Load())
			{
				definition->ForEachGroup([this, &defNode](ItemGroup& group)
				{
					LoadGroup(defNode, group);
				});
				m_Definitions.insert_or_assign(id, std::move(definition));
			}
		}
	}
	KWorkspace* DefaultGameConfigManager::CreateWorkspace(KMainWindow* mainWindow)
	{
		return new Workspace(mainWindow);
	}

	void DefaultGameConfigManager::OnItemChanged(GameConfig::Item& item)
	{
		m_ChangedItems.remove(&item);
		m_ChangedItems.push_back(&item);
	}
	void DefaultGameConfigManager::OnItemChangeDiscarded(GameConfig::Item& item)
	{
		m_ChangedItems.remove(&item);
	}

	void DefaultGameConfigManager::Load()
	{
		m_ChangedItems.clear();

		for (const auto&[id, definition]: m_Definitions)
		{
			definition->ForEachGroup([](ItemGroup& group)
			{
				group.ReadItems();
			});
		}
	}
	void DefaultGameConfigManager::SaveChanges()
	{
		for (GameConfig::Item* item: m_ChangedItems)
		{
			item->SaveChanges();
		}
		m_ChangedItems.clear();
	}
	void DefaultGameConfigManager::DiscardChanges()
	{
		for (GameConfig::Item* item: m_ChangedItems)
		{
			item->DiscardChanges();
		}
		m_ChangedItems.clear();
	}
	bool DefaultGameConfigManager::HasUnsavedChanges() const
	{
		return !m_ChangedItems.empty();
	}
}
