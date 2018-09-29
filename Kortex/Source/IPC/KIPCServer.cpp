#include "stdafx.h"
#include "KIPCServer.h"
#include "KIPCConnection.h"
#include "KApp.h"
#include "VFS/KVFSService.h"
#include "VFS/KVFSMirror.h"
#include "VFS/KVFSConvergence.h"
#include <KxFramework/KxFile.h>

KxSingletonPtr_Define(KIPCServer);

wxString KIPCServer::GetClientFileName()
{
	wxString path = "Kortex";
	if (KxSystem::Is64Bit())
	{
		path += " x64";
	}
	path += ".exe";
	return path;
}

wxConnectionBase* KIPCServer::OnAcceptConnection(const wxString& topic)
{
	if (topic == KIPC::GetTopic())
	{
		m_Connection = new KIPCConnection(this);
		return m_Connection;
	}
	return NULL;
}
void KIPCServer::OnDisconnect()
{
	KApp::Get().ExitApp();
}

void KIPCServer::OnAcceptRequest(const KIPCRequestNS::InitVFSService& config)
{
	m_Service = std::make_unique<KVFSService>();
	if (m_Service->Init())
	{
		if (!m_Service->IsInstalled())
		{
			// Notify install begins
			if (!m_Service->Install())
			{
				// Notify install failed
			}
		}
		else
		{
			// Reconfigure service
			m_Service->Install();
		}
		m_Service->Start();
	}
	else
	{
		// Init failed
	}
}
void KIPCServer::OnAcceptRequest(const KIPCRequestNS::UninstallVFSService& config)
{
	if (m_Service)
	{
		m_Service->Stop();
		m_Service->Uninstall();
	}
}

void KIPCServer::OnAcceptRequest(const KIPCRequestNS::CreateConvergenceVFS& config)
{
	m_Convergence = std::make_unique<KVFSConvergence>(GetServiceVFS(), config.GetMountPoint(), config.GetWriteTarget());
	m_Convergence->SetCanDeleteInVirtualFolder(config.CanDeleteInVirtualFolder());

	// Make sure folders are exist
	KxFile(config.GetMountPoint()).CreateFolder();
	KxFile(config.GetWriteTarget()).CreateFolder();
}
void KIPCServer::OnAcceptRequest(const KIPCRequestNS::AddConvergenceVirtualFolder& config)
{
	m_Convergence->AddVirtualFolder(config.GetPath());
}
void KIPCServer::OnAcceptRequest(const KIPCRequestNS::ClearConvergenceVirtualFolders& config)
{
	m_Convergence->ClearVirtualFolders();
}
void KIPCServer::OnAcceptRequest(const KIPCRequestNS::BuildConvergenceIndex& config)
{
	m_Convergence->BuildDispatcherIndex();
}

void KIPCServer::OnAcceptRequest(const KIPCRequestNS::BeginConvergenceIndex& config)
{
	m_ConvergenceIndex.clear();
}
void KIPCServer::OnAcceptRequest(const KIPCRequestNS::CommitConvergenceIndex& config)
{
	KVFSConvergence::ExternalDispatcherIndexT index;
	index.reserve(m_ConvergenceIndex.size());

	for (const auto& value: m_ConvergenceIndex)
	{
		index.emplace_back(value.first.wc_str(), value.second.wc_str());
	}
	m_Convergence->SetDispatcherIndex(index);

	m_ConvergenceIndex.clear();
	m_ConvergenceIndex.shrink_to_fit();
}
void KIPCServer::OnAcceptRequest(const KIPCRequestNS::AddConvergenceIndex& config)
{
	m_ConvergenceIndex.push_back(std::make_pair(config.GetRequestPath(), config.GetTargetPath()));
}

void KIPCServer::OnAcceptRequest(const KIPCRequestNS::CreateMirrorVFS& config)
{
	m_MirrorVFSList.emplace_back(std::make_unique<KVFSMirror>(GetServiceVFS(), config.GetTarget(), config.GetSource()));
}
void KIPCServer::OnAcceptRequest(const KIPCRequestNS::ClearMirrorVFSList& config)
{
	m_MirrorVFSList.clear();
}

void KIPCServer::OnAcceptRequest(const KIPCRequestNS::ToggleVFS& config)
{
	config.ShouldEnable() ? ToggleVFS() : DisableVFS();
}

void KIPCServer::OnVFSUnmounted(KxBroadcastEvent& event)
{
	if (!m_ManualDisablingInProgress)
	{
		// Unmount all other VFS's
		DisableVFS();

		if (m_Connection)
		{
			m_Connection->SendToClient(KIPCRequestNS::VFSStateChanged(false, event.GetInt()));
		}
	}
}
void KIPCServer::ReportMountError(int code)
{
	KxBroadcastEvent(KEVT_VFS_UNMOUNTED).Send();
}

KIPCServer::KIPCServer()
	:m_IsCreated(Create(KIPC::GetServiceName()))
{
	KxBroadcastEvent::Bind(KEVT_VFS_UNMOUNTED, &KIPCServer::OnVFSUnmounted, this);
}
KIPCServer::~KIPCServer()
{
	DisableVFS();
}

bool KIPCServer::IsVFSEnabled() const
{
	if (m_Convergence && m_Convergence->IsMounted())
	{
		return true;
	}
	for (auto& mirror: m_MirrorVFSList)
	{
		if (mirror->IsMounted())
		{
			return true;
		}
	}
	return false;
}
int KIPCServer::ToggleVFS()
{
	if (!IsVFSEnabled())
	{
		int status = KVFSService::GetSuccessCode();
		if (m_Convergence && !m_Convergence->IsMounted())
		{
			status = m_Convergence->Mount();
			if (!KVFSService::IsSuccessCode(status))
			{
				ReportMountError(status);
				return status;
			}
		}

		if (KVFSService::IsSuccessCode(status))
		{
			for (auto& mirror: m_MirrorVFSList)
			{
				if (!mirror->IsMounted())
				{
					KxFile(mirror->GetSource()).CreateFolder();
					KxFile(mirror->GetMountPoint()).CreateFolder();
					status = mirror->Mount();

					if (!KVFSService::IsSuccessCode(status))
					{
						ReportMountError(status);
						return status;
					}
				}
			}
		}

		// Notify client
		if (m_Connection)
		{
			m_Connection->SendToClient(KIPCRequestNS::VFSStateChanged(IsVFSEnabled(), status));
		}
		return status;
	}
	else
	{
		return KVFS_STATUS_NOT_STARTED;
	}
}
bool KIPCServer::DisableVFS()
{
	m_ManualDisablingInProgress = true;

	bool b1 = false;
	if (m_Convergence)
	{
		b1 = m_Convergence->UnMount();
	}

	bool b2 = false;
	for (auto& mirror: m_MirrorVFSList)
	{
		b2 = mirror->UnMount();
	}

	m_ManualDisablingInProgress = false;
	return b1 && b2;
}
