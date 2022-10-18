#include "stdafx.h" //Standard headers
#include "resource.h" //Standard headers
#include "Commctrl.h"
#include "Uxtheme.h"

//Global Scope
HWND DialogWindowHwnd;
HWND WindowListCtrl;
HWND CurrentSelectedWindow;

//Window List Population
BOOL IsAltTabWindow(HWND hwnd)
{
    TITLEBARINFO ti;
    HWND hwndTry, hwndWalk = NULL;

    if(!IsWindowVisible(hwnd))
        return FALSE;

    hwndTry = GetAncestor(hwnd, GA_ROOTOWNER);
    while(hwndTry != hwndWalk) 
    {
        hwndWalk = hwndTry;
        hwndTry = GetLastActivePopup(hwndWalk);
        if(IsWindowVisible(hwndTry)) 
            break;
    }
    if(hwndWalk != hwnd)
        return FALSE;

    // the following removes some task tray programs and "Program Manager"
    ti.cbSize = sizeof(ti);
    GetTitleBarInfo(hwnd, &ti);

    if(ti.rgstate[0] & STATE_SYSTEM_INVISIBLE)
        return FALSE;

    return TRUE;
}

BOOL CALLBACK MyEnumProc(HWND hWnd, LPARAM lParam)
{
	//if(!IsAltTabWindow(hWnd) || IsIconic(hWnd))
	//return TRUE;			// skip this window and continue

	char title[500] = {0};
	GetWindowText(hWnd, (LPSTR)title, 500);

	LVITEM item;
	item.mask = LVIF_TEXT | LVIF_STATE | LVIF_PARAM;
	item.pszText = title;
	item.iSubItem = 0;
	item.state = 0;
	item.lParam = (LPARAM)hWnd;

	SendMessage(WindowListCtrl, LVM_INSERTITEM, 0, (LPARAM)&item);
	return TRUE;
}

void populateWindowList()
{
	ListView_DeleteAllItems(WindowListCtrl);
	EnumWindows(MyEnumProc, 0);

	//Disable Buttons
	EnableWindow( GetDlgItem( DialogWindowHwnd, IDSETFULLSCREEN ), FALSE);
	EnableWindow( GetDlgItem( DialogWindowHwnd, IDSETWINDOWED ), FALSE);
}

void SetWindowFullscreen(HWND appHandle)
{
	LONG lStyle = GetWindowLong(appHandle, GWL_STYLE);
	lStyle &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);
	SetWindowLong(appHandle, GWL_STYLE, lStyle);

	LONG lExStyle = GetWindowLong(appHandle, GWL_EXSTYLE);
	lExStyle &= ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
	SetWindowLong(appHandle, GWL_EXSTYLE, lExStyle);

	SetWindowPos(appHandle, NULL, 0,0,0,0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER);
	ShowWindow(appHandle, SW_MAXIMIZE);
}

void RestoreFullscreen(HWND appHandle)
{
	SetWindowLong(appHandle, GWL_STYLE, WS_POPUPWINDOW | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE | WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZEBOX | WS_SYSMENU);
	SetWindowPos(appHandle, NULL, 0,0,0,0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER);
	ShowWindow(appHandle, SW_RESTORE);
	UpdateWindow(GetDesktopWindow());
}

// Message handler for our Main Window. And yes, we're using a dialog box for the UI :P
INT_PTR CALLBACK MainWindow(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	int iIndex;

	switch (message)
	{
	case WM_INITDIALOG:
		DialogWindowHwnd = hDlg;
		WindowListCtrl = GetDlgItem(hDlg, IDC_WNDLIST);
		SetWindowTheme(WindowListCtrl, L"Explorer", NULL);
		ListView_SetExtendedListViewStyle(WindowListCtrl, LVS_EX_ONECLICKACTIVATE | LVS_EX_UNDERLINEHOT | LVS_EX_LABELTIP);
		populateWindowList();

		//NEW IN 1.2
		/*
		Forces always on top
		*/

		SetWindowPos(hDlg, HWND_TOPMOST, NULL, NULL, NULL, NULL, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDCLOSE)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		if(LOWORD(wParam) == IDREFRESH)
		{
			populateWindowList();
		}
		if(LOWORD(wParam) == IDSETFULLSCREEN)
		{
			SetWindowFullscreen(CurrentSelectedWindow);
		}
		if(LOWORD(wParam) == IDSETWINDOWED)
		{
			RestoreFullscreen(CurrentSelectedWindow);
		}
		break;

	case WM_NOTIFY:
			 LPNMHDR lpnmHdr = (LPNMHDR)lParam;

			 if(WindowListCtrl==lpnmHdr->hwndFrom)
			 {
				 switch(lpnmHdr->code)
				 {
					case LVN_ITEMCHANGED:
						//	lParam  contains our user-defined info!
						NMITEMACTIVATE nmItem = *(NMITEMACTIVATE*)lParam;
						LVITEM lvi = {LVIF_PARAM};
						lvi.iItem = nmItem.iItem;

						ListView_GetItem(lpnmHdr->hwndFrom, &lvi);
						
						CurrentSelectedWindow = (HWND)lvi.lParam;

						iIndex = (int)SendMessage(WindowListCtrl, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
						if(iIndex == -1)
						{
							//Disable Buttons
							EnableWindow( GetDlgItem( hDlg, IDSETFULLSCREEN ), FALSE);
							EnableWindow( GetDlgItem( hDlg, IDSETWINDOWED ), FALSE);
						}
						else
						{
							//Enable Buttons
							EnableWindow( GetDlgItem( hDlg, IDSETFULLSCREEN ), TRUE);
							EnableWindow( GetDlgItem( hDlg, IDSETWINDOWED ), TRUE);

						}
					break;
				 }
			 }
		break;
	}
	return (INT_PTR)FALSE;
}

int WINAPI wWinMain( HINSTANCE Ari_hInst, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
	DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(MAINWINDOW), NULL, MainWindow);
}