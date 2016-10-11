#include "StdAfx.h"
#include "SYMailSlotManager.h"

#define NAME	"\\\\.\\mailslot\\ir"


SYMailSlotManager* SYMailSlotManager::m_instance = NULL;
//=================================================================================================================================
SYMailSlotManager* SYMailSlotManager::Instance()
{
	if (m_instance == NULL) {
		m_instance = new SYMailSlotManager(); 
	}
	return m_instance;
}

SYMailSlotManager::SYMailSlotManager(void)
{
	m_hSendSlot = NULL;
	m_hMailSlot = NULL;
	m_hRecvProcThread = NULL;	
	OnRecvEvent = NULL;
}

SYMailSlotManager::~SYMailSlotManager(void)
{
	CloseSend();
	CloseRecv();
}

bool SYMailSlotManager::CreateSend()
{
	if (m_hSendSlot) CloseSend();
	/* Open the blort mailslot on MyComputer */
	m_hSendSlot = CreateFileA(NAME,
		GENERIC_WRITE, 
		FILE_SHARE_READ,
		NULL, 
		OPEN_EXISTING, 
		FILE_ATTRIBUTE_NORMAL, 
		NULL); 
	if (m_hSendSlot == INVALID_HANDLE_VALUE) 
	{
		//OutputDebugString(_T("CreateFile failed: %d\n", GetLastError()));
		return false;
	}
	return true;
}

void SYMailSlotManager::CloseSend()
{
	CloseHandle(m_hSendSlot);	
	m_hSendSlot = NULL;
}

bool SYMailSlotManager::Send(char *pMsg)
{
	if (m_hSendSlot == NULL) return false;

	char *MyMessage = pMsg;
	BOOL     err;
	DWORD    numWritten;

	
	/* Write out our nul-terminated string to a record */
	err = WriteFile(m_hSendSlot, MyMessage, (DWORD)strlen(MyMessage)+1, &numWritten, NULL);

	/* See if an error */
	if (!err) {
		//OutputDebugString(_T("WriteFile error: %d\n", GetLastError()));
		return false;
	}

	/* Make sure all the bytes were written */
	else if ((DWORD)strlen(MyMessage) != (numWritten-1)) {
		//OutputDebugString(_T("WriteFile did not read the correct number of bytes!\n"));
		return false;
	}
	return true;
}
//------------------------------------------------------------------------------------------------
bool SYMailSlotManager::CreateRecv()
{
	if (m_hMailSlot != NULL) return true;

	m_hMailSlot = CreateMailslotA(NAME, 0, /*MAILSLOT_WAIT_FOREVER*/1000, NULL);
	if (m_hMailSlot == INVALID_HANDLE_VALUE) 
	{
		//OutputDebugString(_T("CreateMailslot failed: %d\n", GetLastError()));
		return false;
	}
	return true;
}

void SYMailSlotManager::CloseRecv()
{
	TerminateThread(m_hRecvProcThread, 0);
	m_hRecvProcThread = NULL;

	CloseHandle(m_hMailSlot);	
	m_hMailSlot = NULL;
}

DWORD WINAPI RecvProcThread(void *pParam)
{
	//SYMailSlotManager *pThis = (SYMailSlotManager *)pParam;

	HANDLE handle = SYMailSlotManager::Instance()->getRecvHandle();
	DWORD dwSize;
	DWORD dwMsgCnt;

	while(true)
	{
		BOOL bRet = GetMailslotInfo( handle, NULL, &dwSize, &dwMsgCnt, NULL );

		if ( !bRet || dwMsgCnt <= 0 )
			continue ;

		if ( dwSize == MAILSLOT_NO_MESSAGE )
			continue ;

		DWORD dwRead;
		char szBuf[1024];
		ReadFile(handle, szBuf, dwSize, &dwRead, NULL );

		SYMailSlotManager::Instance()->OnRecvEvent(szBuf);
		OutputDebugStringA(szBuf);		
	}
	return 0;
}

void SYMailSlotManager::Recv()
{
	if (m_hRecvProcThread != NULL) return;
	m_hRecvProcThread = CreateThread(NULL, 0, RecvProcThread, this, 0, NULL);
}