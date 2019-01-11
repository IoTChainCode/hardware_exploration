; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CCommandView
LastTemplate=CFormView
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "BlockchainMfc.h"
LastPage=0

ClassCount=8
Class1=CBlockchainMfcApp
Class2=CBlockchainMfcDoc
Class3=CBlockchainMfcView
Class4=CMainFrame

ResourceCount=3
Resource1=IDR_MAINFRAME
Class5=CAboutDlg
Class6=CConnectView
Class7=CTransactionView
Resource2=IDD_ABOUTBOX
Class8=CCommandView
Resource3=IDD_FORMVIEW (English (U.S.))

[CLS:CBlockchainMfcApp]
Type=0
HeaderFile=BlockchainMfc.h
ImplementationFile=BlockchainMfc.cpp
Filter=N
LastObject=CBlockchainMfcApp

[CLS:CBlockchainMfcDoc]
Type=0
HeaderFile=BlockchainMfcDoc.h
ImplementationFile=BlockchainMfcDoc.cpp
Filter=N
LastObject=CBlockchainMfcDoc

[CLS:CBlockchainMfcView]
Type=0
HeaderFile=BlockchainMfcView.h
ImplementationFile=BlockchainMfcView.cpp
Filter=C
LastObject=CBlockchainMfcView
BaseClass=CView
VirtualFilter=VWC


[CLS:CMainFrame]
Type=0
HeaderFile=MainFrm.h
ImplementationFile=MainFrm.cpp
Filter=T
BaseClass=CFrameWnd
VirtualFilter=fWC
LastObject=CMainFrame




[CLS:CAboutDlg]
Type=0
HeaderFile=BlockchainMfc.cpp
ImplementationFile=BlockchainMfc.cpp
Filter=D

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[MNU:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_APP_EXIT
Command2=ID_APP_ABOUT
CommandCount=2

[ACL:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_FILE_PRINT
Command5=ID_EDIT_UNDO
Command6=ID_EDIT_CUT
Command7=ID_EDIT_COPY
Command8=ID_EDIT_PASTE
Command9=ID_EDIT_UNDO
Command10=ID_EDIT_CUT
Command11=ID_EDIT_COPY
Command12=ID_EDIT_PASTE
Command13=ID_NEXT_PANE
Command14=ID_PREV_PANE
CommandCount=14

[CLS:CConnectView]
Type=0
HeaderFile=ConnectView.h
ImplementationFile=ConnectView.cpp
BaseClass=CScrollView
Filter=C
VirtualFilter=VWC
LastObject=CConnectView

[CLS:CTransactionView]
Type=0
HeaderFile=TransactionView.h
ImplementationFile=TransactionView.cpp
BaseClass=CScrollView
Filter=C
VirtualFilter=VWC
LastObject=CTransactionView

[DLG:IDD_FORMVIEW (English (U.S.))]
Type=1
Class=CCommandView
ControlCount=4
Control1=IDC_LIST_TRANSACTION,SysListView32,1350631425
Control2=IDC_BUTTON_LIMIT,button,1342242816
Control3=IDC_BUTTON_RANDOM,button,1342242816
Control4=IDC_LIST_DEVICE,SysListView32,1350631425

[CLS:CCommandView]
Type=0
HeaderFile=CommandView.h
ImplementationFile=CommandView.cpp
BaseClass=CFormView
Filter=D
VirtualFilter=VWC
LastObject=IDC_BUTTON_LIMIT

