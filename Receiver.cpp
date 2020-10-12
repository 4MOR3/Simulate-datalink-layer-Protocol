
//理想信道的数据链路层协议，接收端程序 

#include "stdafx.h"
//VC 6.0   add  WSOCK32.LIB in Project -> Settings... ->  Link
#include <winsock.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>


using namespace std;

void wlc(BYTE Fram);
//按钮
#define BTN_Quit 501
#define BTN_Reset 502
//单行编辑框
#define EDIT_Receiver 601
//多行文本框
#define INFOWin1 801
#define INFOWin2 802
//静态文本
#define STATIC_W1 701
#define STATIC_W2 702
//--------------------------------------------------------------
WSADATA wsData;
SOCKET Cs1,Cs2;
struct sockaddr_in Cs1A,Cs2A;
char DataB[200];
int d,i;
#define Cs2Port 7100		//远程端口(对应于模拟通道程序中的本地端口)
#define Cs1Port 8100		//本地端口(对应于模拟通道程序中的B站端口)
#define Cs1IP   "127.0.0.1"	//本地IP网址(127.0.0.1为本地环路)
#define Cs2IP   "127.0.0.1"	//远程IP网址(127.0.0.1为本地环路)


char DisplayStringBuffer[5000];//显示缓冲区
//--------------------------------------------------------------
// Global Variables:
HINSTANCE hInst;						// current instance
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);//回调函数
//--------------------------------------------------------------
//主函数
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
	TempString = "接收端[端口:";
	TempString += TempChar;
	TempString += "]－理想信道\0";
	
	HWND hWnd;
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
		400,10,320,300,
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
//静态文本
HWND CreateStatic(char *Titel,int x0,int y0,int w,int h,int ID,HWND hW,HINSTANCE hInst)
{
	return CreateWindowEx(WS_EX_PALETTEWINDOW,"STATIC",Titel,
		WS_VISIBLE | WS_CHILD ,
		x0,y0,w,h, hW,(HMENU)ID,hInst,NULL);
}

//--------------------------------------------------------------
//按钮
HWND CreateButton(char *Titel,int x0,int y0,int w,int h,int ID,HWND hW,HINSTANCE hInst)
{
	return CreateWindowEx(WS_EX_PALETTEWINDOW,"BUTTON",Titel,
		WS_VISIBLE | WS_CHILD ,
		x0,y0,w,h, hW,(HMENU)ID,hInst,NULL);
}
//--------------------------------------------------------------
//单行编辑框
HWND CreateEdit(char *Titel,int x0,int y0,int w,int h,int ID,HWND hW,HINSTANCE hInst)
{
	return CreateWindowEx(WS_EX_CLIENTEDGE,"EDIT",Titel,
		WS_VISIBLE | WS_CHILD |
		    ES_LEFT | ES_MULTILINE | WS_HSCROLL ,
		x0,y0,w,h, hW,
		(HMENU)ID,hInst,NULL);
}
//--------------------------------------------------------------
//多行文本框
HWND CreateMemo(char *Titel,int x0,int y0,int w,int h,int ID,HWND hW,HINSTANCE hInst)
{
	return CreateWindowEx(WS_EX_CLIENTEDGE,"EDIT",Titel,
		WS_VISIBLE | WS_CHILD |
		    ES_LEFT | ES_MULTILINE |ES_READONLY|
		    WS_HSCROLL | WS_VSCROLL,
		x0,y0,w,h,
		hW,(HMENU)ID,hInst,NULL);
}
//---------------------------------------------------------------------------
//添加控件文本
void Puts(HWND hW,int ID_EDIT,char *str)
{	
	GetDlgItemText(hW,ID_EDIT,DisplayStringBuffer,sizeof(DisplayStringBuffer));
	if (strlen(DisplayStringBuffer) > 4900)
	{
		//MessageBox(0,"显示缓冲区溢出！","显示缓冲区",0);
		strcpy(DisplayStringBuffer,"  \r\n");
	}
	strcat(DisplayStringBuffer,"\r\n");
	strcat(DisplayStringBuffer,str);
	SetDlgItemText(hW,ID_EDIT,(LPSTR) DisplayStringBuffer);
	SendMessage(GetDlgItem(hW,ID_EDIT),WM_VSCROLL,SB_THUMBPOSITION+1000*0x10000,0);
}
//---------------------------------------------------------------------------

//以上不需改动!
//--------------------------------------------------------------
//你可以在这里定义变量
//void PhysicalLayerSend(BYTE c);	//物理层：发送一个字节
//BYTE PhysicalLayerReceive();	//物理层：接收一个字节
void PhysicalLayerSend(char* FrameData, int Length);	//物理层：发送一帧报文
int PhysicalLayerReceive(char* FrameData);	//物理层：接收一帧报文，返回数据长度
char Buffer[6];
int cnt1=0,cnt2=0;
bool Parity_Check(char *data){
	int cnt=0;
	char temp=*data;
	while(temp!=0){
		if((temp%2)!=0) cnt++;
		temp=temp/2;
	}
	if(cnt%2==1) return 1;
	else return 0;
}
//--------------------------------------------------------------
//消息处理
LRESULT CALLBACK WndProc(HWND hW, UINT msg, WPARAM wP, LPARAM lP)
{
	switch (msg)
	{
	case WM_DESTROY://窗口关闭时的消息处理
		WSAAsyncSelect(Cs1, hW, 0, 0);
		closesocket(Cs1);
		WSACleanup( );
		PostQuitMessage(0);
		break;
	case WM_CREATE://窗口创建时的消息处理
		WSAStartup(0x0101,&wsData);//网络初始化
		Cs1=socket(AF_INET, SOCK_DGRAM,0);
		Cs1A.sin_family=AF_INET;
		Cs1A.sin_port = htons(Cs1Port);
//		Cs1A.sin_addr.s_addr =INADDR_ANY;//在本机设置的网址(可能多个)中，都有效。
		Cs1A.sin_addr.s_addr =inet_addr(Cs1IP);
		d=bind(Cs1,(struct sockaddr *) &Cs1A,sizeof(Cs1A));
		if(d==-1)//bind函数调用失败
		{
			MessageBox(0,"端口发生冲突","信道模拟",0);
			PostQuitMessage(0);
		};
		Cs2A.sin_family=AF_INET;
		Cs2A.sin_port = htons(Cs2Port);
		Cs2A.sin_addr.s_addr =inet_addr(Cs2IP);
		WSAAsyncSelect(Cs1,hW,WM_USER+212,FD_READ);
		Beep(2000,100);//响铃提示处理

		CreateEdit("接收端",2,2,130,20,EDIT_Receiver,hW,hInst);
		CreateButton("退出", 140,2,50,24,BTN_Quit,hW,hInst);
		CreateButton("复位",200,2,50,24,BTN_Reset,hW,hInst);
		CreateStatic("物理层收到的信息:",0,35,150,24,STATIC_W1,hW,hInst);
		CreateStatic("上交网络层的信息:",160,35,150,24,STATIC_W2,hW,hInst);
		CreateMemo(" ",0,55,150,220,INFOWin1,hW,hInst);
		CreateMemo(" ",160,55,150,220,INFOWin2,hW,hInst);

		break;
//============================================================================
//接收消息事件：
//你可以在这里添加和编写代码：
	case WM_USER+212:
		switch(LOWORD(lP))
		{
		case FD_READ://网络数据接收消息处理
			//*****************************************************************************
			//进行数据帧接收处理，请在此根据协议修改或添加代码
			//以下13行代码只是用于测试，你需要根据协议进行修改
			//*****************************************************************************
			char ReceData[100];
			int Length;
			char check;
			char ctrl[3];
			Length=PhysicalLayerReceive(ReceData);							//物理层：接收一个字节
			if(ReceData[0]==0x7E){											//如果检测到帧头，再接收后续字节
				Sleep(50);													//拥塞控制
				for(int i=0;i<3;i++){										//将收到的数据存入Buffer[]中
					PhysicalLayerReceive(ReceData);
					Buffer[i]=ReceData[0];
				}
				check=Parity_Check(&Buffer[2]);								//奇校验，存入check
				char DisplayBufffer[250];
				if(check==Buffer[0]){										//奇校验通过
					cnt1++;
					DataB[0]=Buffer[2];
					DataB[1]=0;
					wsprintf(DisplayBufffer,"%02XH",(unsigned char)DataB[0]);
					Puts(hW,INFOWin1,DisplayBufffer);						//显示，物理层收到的信息
					DataB[0]=(DataB[0]&0x0F)+0x30;							//去掉高4位内容,低4位转为ASCII码
					Puts(hW,INFOWin2,DataB);								//显示，上交网络层的信息
					ctrl[0]=0x88;											//ACK=0x88
					PhysicalLayerSend(&ctrl[0],1);
				}
				else if(check!=Buffer[0]){ 
					wsprintf(DisplayBufffer,"%02XH",(unsigned char)DataB[0]);
					Puts(hW,INFOWin1,DisplayBufffer);						//显示，物理层收到的信息
					char error[6]="error";									//显示，error信息
					Puts(hW,INFOWin2,error);	
					char SendData[100];
					ctrl[0]=0x55;											//NAK=0x55
					PhysicalLayerSend(&ctrl[0],1);
				}
			
			}
			break;
		}
		break;
//============================================================================
	case WM_COMMAND://按钮命令消息处理
		switch(wP)
		{
		case BTN_Quit:												//退出程序
			PostQuitMessage(0);
			break;
		case BTN_Reset:												//清除信息框内容
			SetDlgItemText(hW,INFOWin1,(LPSTR) " ");
			SetDlgItemText(hW,INFOWin2,(LPSTR) " ");
			break;
		}
	}
	return DefWindowProc(hW,msg,wP,lP);
}

//--------------------------------------------------------------
void PhysicalLayerSend(char* FrameData, int Length)					//物理层：发送一帧报文
{
	int CS2A_Length;
	CS2A_Length = sizeof(Cs2A);
	sendto(Cs1,FrameData,Length,0,(struct sockaddr *) &Cs2A,CS2A_Length);
}

//--------------------------------------------------------------
int PhysicalLayerReceive(char* FrameData)							//物理层：接收一帧报文，返回数据长度
{	
	int Length;
	int CS2A_Length;
	CS2A_Length = sizeof(Cs2A);
	Length = recvfrom(Cs1,FrameData,sizeof(FrameData),0,(struct sockaddr *) &Cs2A,&CS2A_Length);
	return Length;
}
//--------------------------------------------------------------
