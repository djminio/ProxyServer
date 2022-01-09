#ifndef __RECV_MSG_H
#define __RECV_MSG_H

class CConnection;

void __stdcall OnDisconnectUser(DWORD dwConnectionIndex);
void __stdcall OnDisconnectServer(DWORD dwConnectionIndex);
void __stdcall ReceivedMsgUser(DWORD dwConnectionIndex,char* pMsg,DWORD dwLength);
void __stdcall ReceivedMsgServer(DWORD dwConnectionIndex,char* pMsg,DWORD dwLength);
void __stdcall ReceivedMsgUserUDP(DWORD dwUserID,char* pMsg,DWORD length);
void __stdcall OnAcceptUser(DWORD dwConnectionIndex);
void __stdcall OnAcceptServer(DWORD dwConnectionIndex);
#endif