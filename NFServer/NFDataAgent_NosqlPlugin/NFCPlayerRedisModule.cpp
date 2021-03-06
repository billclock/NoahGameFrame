// -------------------------------------------------------------------------
//    @FileName			:    NFCPlayerRedisModule.cpp
//    @Author           :    LvSheng.Huang
//    @Date             :    2013-10-03
//    @Module           :    NFCPlayerRedisModule
//    @Desc             :
// -------------------------------------------------------------------------
#include "NFCPlayerRedisModule.h"

NFCPlayerRedisModule::NFCPlayerRedisModule(NFIPluginManager * p)
{
	pPluginManager = p;
}

bool NFCPlayerRedisModule::Init()
{
	return true;
}

bool NFCPlayerRedisModule::Shut()
{
	return true;
}

bool NFCPlayerRedisModule::Execute()
{
	return true;
}

bool NFCPlayerRedisModule::AfterInit()
{
	m_pLogicClassModule = pPluginManager->FindModule<NFIClassModule>();
	m_pNoSqlModule = pPluginManager->FindModule<NFINoSqlModule>();
	m_pCommonRedisModule = pPluginManager->FindModule<NFICommonRedisModule>();
	m_pKernelModule = pPluginManager->FindModule<NFIKernelModule>();

	m_pKernelModule->AddClassCallBack(NFrame::Player::ThisName(), this, &NFCPlayerRedisModule::OnObjectClassEvent);
	RegisterAutoSave(NFrame::Player::ThisName());
	return true;
}

int64_t NFCPlayerRedisModule::GetPlayerCacheGameID(const NFGUID & self)
{
	NF_SHARE_PTR<NFINoSqlDriver> pNoSqlDriver = m_pNoSqlModule->GetDriverBySuitConsistent();
	if (!pNoSqlDriver)
	{
		return 0;
	}

	std::string strValue;
	int64_t nGameID = 0;
	std::string strKey = GetOnlineGameServerKey();

	if (pNoSqlDriver->HGet(strKey, self.ToString(), strValue))
	{
		try
		{
			nGameID = lexical_cast<int64_t>(strValue);
		}
		catch (...)
		{
		}
	}

	return nGameID;
}

int64_t NFCPlayerRedisModule::GetPlayerCacheProxyID(const NFGUID & self)
{
	NF_SHARE_PTR<NFINoSqlDriver> pNoSqlDriver = m_pNoSqlModule->GetDriverBySuitConsistent();
	if (!pNoSqlDriver)
	{
		return 0;
	}

	std::string strValue;
	int64_t nProxyID = 0;
	std::string strKey = GetOnlineProxyServerKey();

	if (pNoSqlDriver->HGet(strKey, self.ToString(), strValue))
	{
		try
		{
			nProxyID = lexical_cast<int64_t>(strValue);
		}
		catch (...)
		{
		}
	}

	return nProxyID;
}

bool NFCPlayerRedisModule::GetPlayerCacheGameID(const std::vector<std::string>& xList, std::vector<int64_t>& xResultList)
{
	NF_SHARE_PTR<NFINoSqlDriver> pNoSqlDriver = m_pNoSqlModule->GetDriverBySuitConsistent();
	if (!pNoSqlDriver)
	{
		return 0;
	}

	std::string strKey = GetOnlineGameServerKey();
	std::vector<std::string> strValueList;

	if (pNoSqlDriver->HMGet(strKey, xList, strValueList))
	{
		for (int i = 0; i < strValueList.size(); ++i)
		{
			int64_t nGameID = 0;

			try
			{
				nGameID = lexical_cast<int64_t>(strValueList[i]);
			}
			catch (...)
			{
			}

			xResultList.push_back(nGameID);
		}

		return true;
	}

	return false;
}

bool NFCPlayerRedisModule::GetPlayerCacheProxyID(const std::vector<std::string>& xList, std::vector<int64_t>& xResultList)
{
	NF_SHARE_PTR<NFINoSqlDriver> pNoSqlDriver = m_pNoSqlModule->GetDriverBySuitConsistent();
	if (!pNoSqlDriver)
	{
		return 0;
	}

	std::string strKey = GetOnlineProxyServerKey();
	std::vector<std::string> strValueList;

	if (pNoSqlDriver->HMGet(strKey, xList, strValueList))
	{
		for (int i = 0; i < strValueList.size(); ++i)
		{
			int64_t nProxyID = 0;

			try
			{
				nProxyID = lexical_cast<int64_t>(strValueList[i]);
			}
			catch (...)
			{
			}

			xResultList.push_back(nProxyID);
		}

		return true;
	}

	return false;
}

NF_SHARE_PTR<NFIPropertyManager> NFCPlayerRedisModule::GetPlayerCacheProperty(const NFGUID& self)
{
	return m_pCommonRedisModule->GetCachePropertyInfo(self, NFrame::Player::ThisName());
}

NF_SHARE_PTR<NFIRecordManager> NFCPlayerRedisModule::GetPlayerCacheRecord(const NFGUID& self)
{
	return m_pCommonRedisModule->GetCacheRecordInfo(self, NFrame::Player::ThisName());
}

bool NFCPlayerRedisModule::SetPlayerCacheProperty(const NFGUID& self, NF_SHARE_PTR<NFIPropertyManager> pPropertyManager)
{
	if (pPropertyManager == nullptr)
	{
		return false;
	}

	if (!m_pCommonRedisModule->SetCachePropertyInfo(self, NFrame::Player::ThisName(), pPropertyManager))
	{
		return false;
	}

	return true;
}

bool NFCPlayerRedisModule::SetPlayerCacheRecord(const NFGUID& self, NF_SHARE_PTR<NFIRecordManager> pRecordManager)
{
	if (pRecordManager == nullptr)
	{
		return false;
	}

	if (!m_pCommonRedisModule->SetCacheRecordInfo(self, NFrame::Player::ThisName(), pRecordManager))
	{
		return false;
	}

	return true;
}

bool NFCPlayerRedisModule::GetAccountRoleID(const std::string & strAccount, NFGUID& xPlayerID)
{
	NF_SHARE_PTR<NFINoSqlDriver> pNoSqlDriver = m_pNoSqlModule->GetDriverBySuitConsistent();
	if (!pNoSqlDriver)
	{
		return false;
	}

	std::string strValue;

	if (pNoSqlDriver->HGet("AccountInfo_" + strAccount, "RoleID", strValue))
	{
		if (strValue == "**nonexistent-key**")
		{
			return false;
		}

		xPlayerID.FromString(strValue);
		return true;
	}

	return false;
}

bool NFCPlayerRedisModule::SavePlayerDataToCatch(const NFGUID & self)
{
	bool isProperty = SetPlayerCacheProperty(self, m_pKernelModule->GetObject(self)->GetPropertyManager());
	bool isRecord = SetPlayerCacheRecord(self, m_pKernelModule->GetObject(self)->GetRecordManager());

	return isProperty && isRecord;
}

const NFGUID NFCPlayerRedisModule::CreateRole(const std::string & strAccount, const std::string & strName)
{
	NFGUID xCacheRoleID;
	if (!GetAccountRoleID(strAccount, xCacheRoleID))
	{
		NF_SHARE_PTR<NFINoSqlDriver> pNoSqlDriver = m_pNoSqlModule->GetDriverBySuitConsistent();
		if (!pNoSqlDriver)
		{
			return xCacheRoleID;
		}

		xCacheRoleID = m_pKernelModule->CreateGUID();
		NFCDataList xArgs;
		NF_SHARE_PTR<NFIObject> pObject = m_pKernelModule->CreateObject(xCacheRoleID, 1, 0, NFrame::Player::ThisName(), "", xArgs);

		pObject->SetPropertyString(NFrame::Player::Account(), strAccount);
		pObject->SetPropertyString(NFrame::Player::Name(), strName);
		m_pKernelModule->DestroyObject(xCacheRoleID);

		if (!pNoSqlDriver->HSet("AccountInfo_" + strAccount, "RoleID", xCacheRoleID.ToString()))
		{
			return NFGUID();
		}
	}

	return xCacheRoleID;
}

const bool NFCPlayerRedisModule::DeleteRole(const std::string & strAccount, const NFGUID xID)
{
	return false;
}

std::string NFCPlayerRedisModule::GetOnlineGameServerKey()
{
	return "OnlineGameKey";
}

std::string NFCPlayerRedisModule::GetOnlineProxyServerKey()
{
	return "OnlineProxyKey";
}

bool NFCPlayerRedisModule::RegisterAutoSave(const std::string & strClassName)
{
	return m_pKernelModule->AddClassCallBack(strClassName, this, &NFCPlayerRedisModule::OnObjectClassEvent);;
}

const bool NFCPlayerRedisModule::AttachData(const NFGUID & self)
{
	NF_SHARE_PTR<NFIObject> pObject = m_pKernelModule->GetObject(self);
	if (!pObject)
	{
		return false;
	}
	NF_SHARE_PTR<NFIPropertyManager> pProManager = pObject->GetPropertyManager();
	NF_SHARE_PTR<NFIRecordManager> pRecordManager = pObject->GetRecordManager();

	NFMsg::ObjectPropertyList xPbPropertyList;
	if (m_pCommonRedisModule->GetCachePropertyListPB(self, NFrame::Player::ThisName(), xPbPropertyList))
	{
		m_pCommonRedisModule->ConvertPBToPropertyManager(xPbPropertyList, pProManager);
	}

	NFMsg::ObjectRecordList xPbRecordList;
	if (m_pCommonRedisModule->GetCacheRecordListPB(self, NFrame::Player::ThisName(), xPbRecordList))
	{
		m_pCommonRedisModule->ConvertPBToRecordManager(xPbRecordList, pRecordManager);
	}

	return true;
}

int NFCPlayerRedisModule::OnObjectClassEvent(const NFGUID & self, const std::string & strClassName, const CLASS_OBJECT_EVENT eClassEvent, const NFIDataList & var)
{
	if (CLASS_OBJECT_EVENT::COE_DESTROY == eClassEvent)
	{
		OnOffline(self);
		NFINT64 xT1 = NFGetTime();
		m_pKernelModule->SetPropertyInt(self, NFrame::Player::LastOfflineTime(), xT1);

		const NFINT64& xT2 = m_pKernelModule->GetPropertyInt(self, NFrame::Player::OnlineTime());

		NFINT64 totalTime = m_pKernelModule->GetPropertyInt(self, NFrame::Player::TotalTime());
		m_pKernelModule->SetPropertyInt(self, NFrame::Player::TotalTime(), totalTime + xT1 - xT2);

		SavePlayerDataToCatch(self);
	}
	else if (CLASS_OBJECT_EVENT::COE_CREATE_LOADDATA == eClassEvent)
	{
		OnOnline(self);
		AttachData(self);
		// 加入上线信息
		m_pKernelModule->SetPropertyInt(self, NFrame::Player::OnlineTime(), NFGetTime());
		int nOnlineCount = m_pKernelModule->GetPropertyInt(self, NFrame::Player::OnlineCount());
		m_pKernelModule->SetPropertyInt(self, NFrame::Player::OnlineCount(), (nOnlineCount + 1));
	}

	return 0;
}

void NFCPlayerRedisModule::OnOnline(const NFGUID & self)
{
	const int nGateID = m_pKernelModule->GetPropertyInt(self, NFrame::Player::GateID());
	const int nServerID = m_pKernelModule->GetPropertyInt(self, NFrame::Player::GameID());
}

void NFCPlayerRedisModule::OnOffline(const NFGUID & self)
{
	const int nGateID = 0;
	const int nServerID = 0;
}
