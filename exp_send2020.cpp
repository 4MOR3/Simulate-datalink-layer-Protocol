// Exp_Send.cpp : Defines the entry point for the application.
//��ϢͨѶ��������γ̣������ŵ���������·��Э�飬���Ͷ˳���, 2020.3

#include "stdafx.h"
//VC 6.0   add  WSOCK32.LIB in Project -> Settings... ->  Link
#include <winsock.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
using namespace std;


//--------------------------------------------------------------
// Global Variables:
//��ť
#define BTN_Sender 501
#define BTN_Reset  502
//���б༭��
#define EDIT_Sender 1601
//�����ı���
#define INFOWin1 801
#define INFOWin2 802
//��̬�ı�
#define STATIC_W1 701
#define STATIC_W2 702
//--------------------------------------------------------------

SOCKET Cs1,Cs2;
struct sockaddr_in Cs1A,Cs2A;
WSADATA wsData;
char DataAA[200];
char DataB[200];
int d,i;

#define Cs1Port 6100		//���ض˿�(��Ӧ��ģ��ͨ�������е�Aվ�˿�)
#define Cs2Port 7100		//Զ�̶˿�(��Ӧ��ģ��ͨ�������еı��ض˿�)
#define Cs1IP   "127.0.0.1"	//����IP��ַ(127.0.0.1Ϊ���ػ�·)
#define Cs2IP   "127.0.0.1"	//Զ��IP��ַ(127.0.0.1Ϊ���ػ�·)

#define ID_TIMER  201
int time_Num=0;
HWND hWnd;

char DisplayStringBuffer[5000];//��ʾ������

HINSTANCE hInst;						// current instance
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);//�ص�����
//--------------------------------------------------------------
//������
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: Place code here.
	MSG msg;
	string TempString;
	
	char TempChar[80];
	char TempChar1[80];
	itoa(Cs1Port,TempChar,10);
	TempString = "���Ͷ�[�˿�:";
	TempString += TempChar;
	TempString += "]�������ŵ�";
	
	hInst = hInstance; // Store instance handle in our global variable
	WNDCLASS wc;

	memset(&wc,0,sizeof(WNDCLASS));

	wc.lpfnWndProc	= (WNDPROC)WndProc;
	wc.hInstance		= hInstance;
	wc.hIcon			= LoadIcon(NULL, IDI_APPLICATION);
	wc.hbrBackground	= (HBRUSH)COLOR_WINDOW;
	wc.lpszClassName	= "W1";
	RegisterClass(&wc);

	TempString.copy(TempChar1,TempString.length());
	TempChar1[TempString.length()] = 0x00;
	hWnd=CreateWindow("W1",TempChar1,
		WS_DLGFRAME|WS_SYSMENU,
		50,10,320,300,
		NULL,NULL,hInstance,NULL);

   	if (!hWnd)   return FALSE;
	

   	ShowWindow(hWnd, nCmdShow);
	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}

//--------------------------------------------------------------
//��̬�ı�
HWND CreateStatic(char *Titel,int x0,int y0,int w,int h,int ID,HWND hW,HINSTANCE hInst)
{
	return CreateWindowEx(WS_EX_PALETTEWINDOW,"STATIC",Titel,
		WS_VISIBLE | WS_CHILD ,
		x0,y0,w,h, hW,(HMENU)ID,hInst,NULL);
}

//--------------------------------------------------------------
//��ť
HWND CreateButton(char *Titel,int x0,int y0,int w,int h,int ID,HWND hW,HINSTANCE hInst)
{
	return ::CreateWindowEx(WS_EX_PALETTEWINDOW,"BUTTON",Titel,
		WS_VISIBLE | WS_CHILD ,
		x0,y0,w,h, hW,(HMENU)ID,hInst,NULL);
}
//--------------------------------------------------------------
//���б༭��
HWND CreateEdit(char *Titel,int x0,int y0,int w,int h,int ID,HWND hW,HINSTANCE hInst)
{
	return ::CreateWindowEx(WS_EX_CLIENTEDGE,"EDIT",Titel,
		WS_VISIBLE | WS_CHILD | ES_LEFT /*| ES_MULTILINE | WS_HSCROLL */,
		x0,y0,w,h, hW,
		(HMENU)ID,hInst,NULL);
}
//--------------------------------------------------------------
//�����ı���
HWND CreateMemo(char *Titel,int x0,int y0,int w,int h,int ID,HWND hW,HINSTANCE hInst)
{
	return ::CreateWindowEx(WS_EX_CLIENTEDGE,"EDIT",Titel,
		WS_VISIBLE | WS_CHILD |
		    ES_LEFT | ES_MULTILINE |ES_READONLY|
		    WS_HSCROLL | WS_VSCROLL,
		x0,y0,w,h,
		hW,(HMENU)ID,hInst,NULL);
}
//---------------------------------------------------------------------------
//��ӿؼ��ı�
void Puts(HWND hW,int ID_EDIT,char *str)
{
	GetDlgItemText(hW,ID_EDIT,DisplayStringBuffer,sizeof(DisplayStringBuffer));
	if (strlen(DisplayStringBuffer) > 4900)
	{
		//MessageBox(0,"��ʾ�����������","��ʾ������",0);
		strcpy(DisplayStringBuffer,"  \r\n");
	}
	strcat(DisplayStringBuffer,"\r\n");
	strcat(DisplayStringBuffer,str);
	SetDlgItemText(hW,ID_EDIT,(LPSTR) DisplayStringBuffer);
	SendMessage(GetDlgItem(hW,ID_EDIT),WM_VSCROLL,SB_THUMBPOSITION+1000*0x10000,0);
}
//---------------------------------------------------------------------------
//���ϴ��벻��Ķ�!
//********************************************************************************
//--------------------------------------------------------------------------------
//����������ﶨ�����
int SendBufLeng;										//���ĳ���
int Sendi=0;											//����֡���ͼ���
//void PhysicalLayerSend(BYTE c);						//����㣺����һ���ֽ�
//BYTE PhysicalLayerReceive();							//����㣺����һ���ֽ�
void PhysicalLayerSend(char* FrameData, int Length);	//����㣺����һ֡����
int PhysicalLayerReceive(char* FrameData);				//����㣺����һ֡���ģ��������ݳ���
char Buffer[4];											//���ͻ���
int flag=0;												//Sending Status.

bool Parity_Check(char *data){							//��У��
	int cnt=0;
	char temp=*data;
	while(temp!=0){
		if((temp%2)!=0) cnt++;
		temp=temp/2;
	}
	if(cnt%2==1) return 1;
	else return 0;
}

void Send(char seq){
	
	int k=seq;
	Buffer[0]=0x7E;									//Header
	Buffer[1]=Parity_Check(&DataAA[k]);					//CRC
	Buffer[2]=k;									//Seq
	Buffer[3]=DataAA[k];							//Data
	PhysicalLayerSend(Buffer,sizeof(Buffer));		//����㣺����һ���ֽ�
	flag=1;
	Sleep(300);
	SetTimer(hWnd,ID_TIMER,300,NULL);				//������ʱ������ʱʱ��300����
	
};

//--------------------------------------------------------------
//��Ϣ����
LRESULT CALLBACK WndProc(HWND hW, UINT msg, WPARAM wP, LPARAM lP)
{
	switch (msg)
	{
	case WM_DESTROY://���ڹر�ʱ����Ϣ����
		WSAAsyncSelect(Cs1, hW, 0, 0);
		closesocket(Cs1);
		WSACleanup( );
		KillTimer(hWnd,ID_TIMER);
		PostQuitMessage(0);
		break;
	case WM_CREATE://���ڴ���ʱ����Ϣ����
		WSAStartup(0x0101,&wsData);//�����ʼ��
		Cs1=socket(AF_INET, SOCK_DGRAM,0);
		Cs1A.sin_family=AF_INET;
		Cs1A.sin_port = htons(Cs1Port);
//		Cs1A.sin_addr.s_addr =INADDR_ANY;//�ڱ������õ���ַ(���ܶ��)�У�����Ч��
		Cs1A.sin_addr.s_addr =inet_addr(Cs1IP);
		d = bind(Cs1,(struct sockaddr *) &Cs1A,sizeof(Cs1A));
		if(d == -1)//bind��������ʧ��
		{
			MessageBox(0,"�˿ڷ�����ͻ","�ŵ�ģ��",0);
			PostQuitMessage(0);
		};
		Cs2A.sin_family=AF_INET;
		Cs2A.sin_port = htons(Cs2Port);
		Cs2A.sin_addr.s_addr =inet_addr(Cs2IP);
		WSAAsyncSelect(Cs1,hW,WM_USER+211,FD_READ);
		Beep(2000,100);//������ʾ����

		CreateEdit("0123456789",2,2,130,25,EDIT_Sender,hW,hInst);
		CreateButton("����",140,2,50,24,BTN_Sender,hW,hInst);
		CreateButton("��λ",200,2,50,24,BTN_Reset,hW,hInst);
		CreateStatic("������յ�����Ϣ:",0,35,150,24,STATIC_W1,hW,hInst);
		CreateStatic("���͵���Ϣ:",160,35,150,24,STATIC_W2,hW,hInst);
		CreateMemo(" ",0,55,150,220,INFOWin1,hW,hInst);
		CreateMemo(" ",160,55,150,220,INFOWin2,hW,hInst);

		//SetTimer(hWnd,ID_TIMER,1000,NULL);

		break;
//============================================================================
//�����¼�������
//�������������Ӻͱ�д���룺
	case WM_USER+211:
		switch(LOWORD(lP))
		{
		case FD_READ://�������ݽ�����Ϣ����
			//*******************************************************************************
			//��������֡���մ���
			//*******************************************************************************
			
			char ReceData[100];
			char ctrl[2];
			int Length;
			Length=PhysicalLayerReceive(ReceData);					//����㣺����һ���ֽ�	
			flag=0;													//���յ��ֽڣ����ʾδ��ʱ�����flag��0
			DataB[0] = ReceData[0];									//����ACK/NAK
			DataB[1]=0;
			if(DataB[0]==-120){										//����ȷ����Sendi++��������һ����
				if(Sendi==SendBufLeng-1) break;
				Sendi++;
				Send(Sendi);
				char buf[22];
				wsprintf(buf,"%02XH,  %c",(unsigned char)DataAA[Sendi],DataAA[Sendi]);
				Puts(hW,INFOWin2,buf);								//��ʾ������㷢�͵���Ϣ
			}
			else  if(DataB[0]==0x55){								//�����������Send()�ط�
				Sleep(100);
				Send(Sendi);
				char buf[22];
				wsprintf(buf,"%02XH,  %c",(unsigned char)DataAA[Sendi],DataAA[Sendi]);
				Puts(hW,INFOWin2,buf);								//��ʾ������㷢�͵���Ϣ
			};
			char DisplayBufffer[250];
			wsprintf(DisplayBufffer,"%02XH,  %c",(unsigned char)DataB[0],DataB[0]);
			Puts(hW,INFOWin1,DisplayBufffer);						//��ʾ��������յ�����Ϣ
			//if(Sendi==SendBufLeng-1) Sendi=-1;
			//MessageBox(0,DataB,"(FD_READ)",0);
			//*******************************************************************************
			break;
		}
		break;
	case WM_TIMER://��ʱ����Ϣ����
		switch(wP)
		{
		case ID_TIMER:
			if(Sendi==SendBufLeng) break;
			time_Num++;
			if(flag==1){
				Send(Sendi);
				char buf[22];
				wsprintf(buf,"%02XH,  %c",(unsigned char)DataAA[Sendi],DataAA[Sendi]);
				Puts(hW,INFOWin2,buf);								//��ʾ������㷢�͵���Ϣ
			}
			//char buf[22];
			//wsprintf(buf,"TIMER:%d",time_Num);
			//Puts(hW,INFOWin1,buf);								//��ʾ��������յ�����Ϣ
			break;
		};
		break;
	case WM_COMMAND://��ť������Ϣ����
		switch(wP)
		{
		case BTN_Sender:											// "����" ��ť�¼�����	
			GetDlgItemText(hW,EDIT_Sender,DataAA,sizeof(DataAA));	//�ӵ��б༭��õ�����
			SendBufLeng=strlen(DataAA);								//������ĳ���
			//*******************************************************************************
			//for(Sendi=0;Sendi<SendBufLeng;Sendi++)					//���ͱ���
			//Frame encapsulation
			/*Buffer[0]=0x7E;										//Header
			Buffer[1]=crc16(&DataAA[Sendi],1);					//CRC
			Buffer[2]=Sendi;									//Seq
			Buffer[3]=DataAA[Sendi];							//Data
			PhysicalLayerSend(Buffer,sizeof(Buffer));			//����㣺����һ���ֽ�
			*/ 
			Sendi=0;											//���°�ť����ͷ����
			Send(Sendi);
			char buf[22];
			wsprintf(buf,"%02XH,  %c",(unsigned char)DataAA[Sendi],DataAA[Sendi]);
			Puts(hW,INFOWin2,buf);								//��ʾ������㷢�͵���Ϣ
			//*******************************************************************************
			break;
		case BTN_Reset:												//�����Ϣ������
			SetDlgItemText(hW,INFOWin1,(LPSTR) " ");
			SetDlgItemText(hW,INFOWin2,(LPSTR) " ");
			break;
		}
	}
	return DefWindowProc(hW,msg,wP,lP);
}

//---------------------------------------------------------------------------------------------
void PhysicalLayerSend(char* FrameData, int Length)					//����㣺����һ֡����
{
	int CS2A_Length;
	CS2A_Length = sizeof(Cs2A);
	sendto(Cs1,FrameData,Length,0,(struct sockaddr *) &Cs2A,CS2A_Length);
}

//---------------------------------------------------------------------------------------------
int PhysicalLayerReceive(char* FrameData)							//����㣺����һ֡���ģ��������ݳ���
{
	int Length;
	int CS2A_Length;
	CS2A_Length = sizeof(Cs2A);
	Length = recvfrom(Cs1,FrameData,sizeof(FrameData),0,(struct sockaddr *) &Cs2A,&CS2A_Length);
	return Length;
}
//---------------------------------------------------------------------------------------------

