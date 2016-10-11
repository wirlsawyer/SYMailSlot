#pragma once

class SYMailSlotManager
{
public:
	//creat
	static SYMailSlotManager* Instance(void);
	~SYMailSlotManager(void);
	//method
	bool CreateSend();
	void CloseSend();
	bool Send(char *pMsg);

	bool CreateRecv();
	void CloseRecv();
	void Recv();

	HANDLE getRecvHandle(){ return m_hMailSlot; }

	//Event
	void (*OnRecvEvent)(char *pMsg);

private:
	SYMailSlotManager(void);
	//method
	
private:	
	//creat
	static SYMailSlotManager*		m_instance;	
	//var
	HANDLE  m_hSendSlot;
	HANDLE	m_hMailSlot;
	HANDLE	m_hRecvProcThread;	
};
