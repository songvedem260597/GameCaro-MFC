
// vebanco.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "vebanco.h"
#include "MainFrm.h"
#include <DbgHelp.h>
#include <Shlwapi.h>
#include <stdio.h>

#pragma comment(lib, "Dbghelp.lib")
#pragma comment(lib, "Shlwapi.lib")

#include "vebancoDoc.h"
#include "vebancoView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace
{
	CString GetDiagnosticDirectory()
	{
		TCHAR buffer[MAX_PATH * 4] = { 0 };
		DWORD len = GetEnvironmentVariable(_T("CARO_DIAG_DIR"), buffer, _countof(buffer));
		if (len > 0 && len < _countof(buffer))
			return CString(buffer);

		GetModuleFileName(NULL, buffer, _countof(buffer));
		PathRemoveFileSpec(buffer);
		return CString(buffer);
	}

	void EnsureDirectory(const CString& dir)
	{
		CreateDirectory(dir, NULL);
	}

	LONG WINAPI CaroUnhandledExceptionFilter(EXCEPTION_POINTERS* exceptionInfo)
	{
		CString dir = GetDiagnosticDirectory();
		EnsureDirectory(dir);

		CString dumpPath = dir + _T("\\caro-crash.dmp");
		HANDLE dumpFile = CreateFile(dumpPath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (dumpFile != INVALID_HANDLE_VALUE)
		{
			MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
			dumpInfo.ThreadId = GetCurrentThreadId();
			dumpInfo.ExceptionPointers = exceptionInfo;
			dumpInfo.ClientPointers = FALSE;

			MiniDumpWriteDump(
				GetCurrentProcess(),
				GetCurrentProcessId(),
				dumpFile,
				(MINIDUMP_TYPE)(MiniDumpWithDataSegs | MiniDumpWithHandleData | MiniDumpWithThreadInfo),
				&dumpInfo,
				NULL,
				NULL);
			CloseHandle(dumpFile);
		}

		CString crashPath = dir + _T("\\caro-crash.txt");
		FILE* fp = NULL;
		_tfopen_s(&fp, crashPath, _T("a+, ccs=UTF-8"));
		if (fp)
		{
			SYSTEMTIME st;
			GetLocalTime(&st);
			DWORD code = exceptionInfo && exceptionInfo->ExceptionRecord
				? exceptionInfo->ExceptionRecord->ExceptionCode
				: 0;
			PVOID address = exceptionInfo && exceptionInfo->ExceptionRecord
				? exceptionInfo->ExceptionRecord->ExceptionAddress
				: NULL;
			_ftprintf(fp,
				_T("%04d-%02d-%02d %02d:%02d:%02d crash code=0x%08X address=%p pid=%lu tid=%lu\n"),
				st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond,
				code, address, GetCurrentProcessId(), GetCurrentThreadId());
			fclose(fp);
		}

		return EXCEPTION_EXECUTE_HANDLER;
	}
}

// CvebancoApp

BEGIN_MESSAGE_MAP(CvebancoApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, &CvebancoApp::OnAppAbout)
	ON_COMMAND(ID_FILE_NEW, &CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinApp::OnFileOpen)
	ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

CvebancoApp::CvebancoApp()
{
	SetAppID(_T("vebanco.AppID.NoVersion"));
}

CvebancoApp theApp;

BOOL CvebancoApp::InitInstance()
{
	SetUnhandledExceptionFilter(CaroUnhandledExceptionFilter);

	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();
	EnableTaskbarInteraction(FALSE);
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	LoadStdProfileSettings(4);

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CvebancoDoc),
		RUNTIME_CLASS(CMainFrame),
		RUNTIME_CLASS(CvebancoView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();
	return TRUE;
}

int CvebancoApp::ExitInstance()
{
	AfxOleTerm(FALSE);
	return CWinApp::ExitInstance();
}

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

void CvebancoApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}
