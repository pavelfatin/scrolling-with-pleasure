// Right click to capture a screenshot after launching the program.

#include <stdio.h>
#include <windows.h>
#include <windowsx.h>
#include <assert.h>
#include <commctrl.h>

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  HDC hdc;
  PAINTSTRUCT ps;
  SCROLLINFO si;

  static HDC hdcWin;
  static HDC hdcScreen;
  static HDC hdcScreenCompat;
  static HBITMAP hbmpCompat;
  static BITMAP bmp;
  static BOOL fBlt;
  static BOOL fScroll;
  static BOOL fSize;

  static int xMinScroll;
  static int xCurrentScroll;
  static int xMaxScroll;

  static int yMinScroll;
  static int yCurrentScroll;
  static int yMaxScroll;

  switch (uMsg) {
    case WM_CREATE: {
      hdcScreen = CreateDC("DISPLAY", (LPCTSTR) NULL, (LPCTSTR) NULL, (CONST DEVMODE *) NULL);
      hdcScreenCompat = CreateCompatibleDC(hdcScreen);

      bmp.bmBitsPixel = (BYTE) GetDeviceCaps(hdcScreen, BITSPIXEL);
      bmp.bmPlanes = (BYTE) GetDeviceCaps(hdcScreen, PLANES);
      bmp.bmWidth = GetDeviceCaps(hdcScreen, HORZRES);
      bmp.bmHeight = GetDeviceCaps(hdcScreen, VERTRES);

      bmp.bmWidthBytes = ((bmp.bmWidth + 15) & ~15) / 8;

      hbmpCompat = CreateBitmap(bmp.bmWidth, bmp.bmHeight, bmp.bmPlanes, bmp.bmBitsPixel, (CONST VOID *) NULL);

      SelectObject(hdcScreenCompat, hbmpCompat);

      fBlt = FALSE;
      fScroll = FALSE;
      fSize = FALSE;

      xMinScroll = 0;
      xCurrentScroll = 0;
      xMaxScroll = 0;

      yMinScroll = 0;
      yCurrentScroll = 0;
      yMaxScroll = 0;

      break;
    }

    case WM_SIZE: {
      int xNewSize;
      int yNewSize;

      xNewSize = LOWORD(lParam);
      yNewSize = HIWORD(lParam);

      if (fBlt) {
        fSize = TRUE;
      }

      xMaxScroll = max(bmp.bmWidth - xNewSize, 0);
      xCurrentScroll = min(xCurrentScroll, xMaxScroll);
      si.cbSize = sizeof(si);
      si.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS;
      si.nMin   = xMinScroll;
      si.nMax   = bmp.bmWidth;
      si.nPage  = xNewSize;
      si.nPos   = xCurrentScroll;
      SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);

      yMaxScroll = max(bmp.bmHeight - yNewSize, 0);
      yCurrentScroll = min(yCurrentScroll, yMaxScroll);
      si.cbSize = sizeof(si);
      si.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS;
      si.nMin   = yMinScroll;
      si.nMax   = bmp.bmHeight;
      si.nPage  = yNewSize;
      si.nPos   = yCurrentScroll;
      SetScrollInfo(hwnd, SB_VERT, &si, TRUE);

      break;
    }

    case WM_PAINT: {
      PRECT prect;

      hdc = BeginPaint(hwnd, &ps);

      if (fSize) {
        BitBlt(ps.hdc, 0, 0, bmp.bmWidth, bmp.bmHeight, hdcScreenCompat, xCurrentScroll, yCurrentScroll, SRCCOPY);

        fSize = FALSE;
      }

      if (fScroll) {
        Sleep(350);
        prect = &ps.rcPaint;

        BitBlt(ps.hdc, prect->left, prect->top, (prect->right - prect->left), (prect->bottom - prect->top),
          hdcScreenCompat, prect->left + xCurrentScroll, prect->top + yCurrentScroll, SRCCOPY);

        fScroll = FALSE;
      }

      EndPaint(hwnd, &ps);

      break;
    }

    case WM_HSCROLL: {
      int xDelta;
      int xNewPos;
      int yDelta = 0;

      switch (LOWORD(wParam)) {
        case SB_PAGEUP:
          xNewPos = xCurrentScroll - 50;
          break;

        case SB_PAGEDOWN:
          xNewPos = xCurrentScroll + 50;
          break;

        case SB_LINEUP:
          xNewPos = xCurrentScroll - 5;
          break;

        case SB_LINEDOWN:
          xNewPos = xCurrentScroll + 5;
          break;

        case SB_THUMBPOSITION:
          xNewPos = HIWORD(wParam);
          break;

        case SB_THUMBTRACK:
          xNewPos = HIWORD(wParam);
          break;

        default:
          xNewPos = xCurrentScroll;
      }

      xNewPos = max(0, xNewPos);
      xNewPos = min(xMaxScroll, xNewPos);

      if (xNewPos == xCurrentScroll) {
        break;
      }

      fScroll = TRUE;

      xDelta = xNewPos - xCurrentScroll;

      xCurrentScroll = xNewPos;

      ScrollWindowEx(hwnd, -xDelta, -yDelta, (CONST RECT *) NULL,
        (CONST RECT *) NULL, (HRGN) NULL, (PRECT) NULL, SW_INVALIDATE);
      UpdateWindow(hwnd);

      si.cbSize = sizeof(si);
      si.fMask  = SIF_POS;
      si.nPos   = xCurrentScroll;
      SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);

      break;
    }

    case WM_VSCROLL: {
      int xDelta = 0;
      int yDelta;
      int yNewPos;

      switch (LOWORD(wParam)) {
        case SB_PAGEUP:
          yNewPos = yCurrentScroll - 50;
          break;

        case SB_PAGEDOWN:
          yNewPos = yCurrentScroll + 50;
          break;

        case SB_LINEUP:
          yNewPos = yCurrentScroll - 5;
          break;

        case SB_LINEDOWN:
          yNewPos = yCurrentScroll + 5;
          break;

        case SB_THUMBPOSITION:
          yNewPos = HIWORD(wParam);
          break;

        case SB_THUMBTRACK:
          yNewPos = HIWORD(wParam);
          break;

        default:
          yNewPos = yCurrentScroll;
      }

      yNewPos = max(0, yNewPos);
      yNewPos = min(yMaxScroll, yNewPos);

      if (yNewPos == yCurrentScroll) {
        break;
      }

      fScroll = TRUE;

      yDelta = yNewPos - yCurrentScroll;

      yCurrentScroll = yNewPos;

      ScrollWindowEx(hwnd, -xDelta, -yDelta, (CONST RECT *) NULL,
        (CONST RECT *) NULL, (HRGN) NULL, (PRECT) NULL, SW_INVALIDATE);
      UpdateWindow(hwnd);

      si.cbSize = sizeof(si);
      si.fMask  = SIF_POS;
      si.nPos   = yCurrentScroll;
      SetScrollInfo(hwnd, SB_VERT, &si, TRUE);

      break;
    }

    case WM_RBUTTONDOWN: {
      hdcWin = GetDC(hwnd);

      RECT rect;
      GetClientRect(hwnd, &rect);
      FillRect(hdcWin, &rect, (HBRUSH)(COLOR_WINDOW + 1));

      BitBlt(hdcScreenCompat, 0, 0, bmp.bmWidth, bmp.bmHeight, hdcScreen, 0, 0, SRCCOPY);

      BitBlt(hdcWin, 0, 0, bmp.bmWidth, bmp.bmHeight, hdcScreenCompat, 0, 0, SRCCOPY);

      ReleaseDC(hwnd, hdcWin);
      fBlt = TRUE;
      break;
    }

    case WM_CLOSE: {
      DestroyWindow(hwnd);
    }

    case WM_DESTROY: {
      PostQuitMessage (0);
      return 0;
    }
  }

  return DefWindowProc (hwnd, uMsg, wParam, lParam);
}

const char g_szClassName[] = "myWindowClass";

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  WNDCLASSEX wc;
  HWND hwnd;
  MSG Msg;

  wc.cbSize = sizeof(WNDCLASSEX);
  wc.style = 0;
  wc.lpfnWndProc = WndProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = hInstance;
  wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
  wc.lpszMenuName = NULL;
  wc.lpszClassName = g_szClassName;
  wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

  if(!RegisterClassEx(&wc)) {
    MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
    return -1;
  }

  hwnd = CreateWindowEx(
    WS_EX_CLIENTEDGE,
    g_szClassName,
    "Tearing Example",
    WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, CW_USEDEFAULT, 640, 480,
    NULL, NULL, hInstance, NULL);

  if(hwnd == NULL) {
    MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
    return -1;
  }

  ShowWindow(hwnd, nCmdShow);
  UpdateWindow(hwnd);

  while(GetMessage(&Msg, NULL, 0, 0) > 0) {
    TranslateMessage(&Msg);
    DispatchMessage(&Msg);
  }

  return Msg.wParam;
}
