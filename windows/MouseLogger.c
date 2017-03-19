#ifndef UNICODE
#define UNICODE
#endif

#include <Windows.h>
#include <Windowsx.h>
#include <stdio.h>
#include <limits.h>

#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC ((USHORT) 0x01)
#endif

#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE ((USHORT) 0x02)
#endif

typedef struct {
  int count;
  int total;
  int minimum;
  int maximum;
} Stats;

BOOL motionDetected;
int lastX;
int lastY;
Stats hStats, vStats;
Stats hRawStats, vRawStats;
char buffer1[128];
char buffer2[128];

void reset(Stats *stats)
{
  stats->count = 0;
  stats->total = 0;
  stats->minimum = INT_MAX;
  stats->maximum = 0;
}

void resetStats()
{
  reset(&hStats);
  reset(&vStats);
  reset(&hRawStats);
  reset(&vRawStats);
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  const wchar_t CLASS_NAME[] = L"Window Class";

  WNDCLASS wc = {};

  wc.lpfnWndProc = WindowProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = CLASS_NAME;
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);

  RegisterClass(&wc);

  HWND hWnd = CreateWindowEx(
    0, CLASS_NAME, L"Mouse Logger",
    WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 640, 480,
    NULL, NULL, hInstance, NULL);

  RAWINPUTDEVICE devices[1];
  devices[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
  devices[0].usUsage = HID_USAGE_GENERIC_MOUSE;
  devices[0].dwFlags = RIDEV_INPUTSINK;
  devices[0].hwndTarget = hWnd;
  RegisterRawInputDevices(devices, 1, sizeof(devices[0]));

  RECT rc;
  GetWindowRect(hWnd, &rc) ;
  SetWindowPos(hWnd, 0,
    (GetSystemMetrics(SM_CXSCREEN) - rc.right) / 2,
    (GetSystemMetrics(SM_CYSCREEN) - rc.bottom) / 2,
    0, 0, SWP_NOZORDER | SWP_NOSIZE);

  resetStats();

  ShowWindow(hWnd, nCmdShow);

  MSG msg = {};
  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return 0;
}

void doUpdate(Stats *stats, int value)
{
  stats->count++;
  stats->total += value;
  if (value < stats->minimum) {
    stats->minimum = value;
  }
  if (value > stats->maximum) {
    stats->maximum = value;
  }
}

void update(Stats *stats, int value)
{
  if (value != 0) {
    doUpdate(stats, abs(value));
  }
}

void format(char *buffer, Stats *stats) {
  sprintf(buffer, "% 2d <% 5.2f <% 3d",
    stats->count == 0 ? 0 : stats->minimum,
    stats->count == 0 ? 0.0 : ((double)stats->total / stats->count),
    stats->maximum);
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg) {
  case WM_LBUTTONDOWN:
    resetStats();
    printf("Reset.\n");
    break;
  case WM_MOUSEMOVE: {
    int x = GET_X_LPARAM(lParam);
    int y = GET_Y_LPARAM(lParam);

    if (!motionDetected) {
      motionDetected = TRUE;
      TRACKMOUSEEVENT tme = { sizeof(tme) };
      tme.dwFlags = TME_LEAVE;
      tme.hwndTrack = hWnd;
      TrackMouseEvent(&tme);
    } else {
      int dx = x - lastX;
      int dy = y - lastY;
      update(&hStats, dx);
      update(&vStats, dy);
      format(buffer1, &hStats);
      format(buffer2, &vStats);
      printf("OS\t%+d, %+d\t%s\t%s\n", dx, dy, buffer1, buffer2);
    }

    lastX = x;
    lastY = y;

    break;
  }
  case WM_MOUSELEAVE:
    motionDetected = FALSE;
    break;
  case WM_MOUSEWHEEL: {
    int delta = GET_WHEEL_DELTA_WPARAM(wParam);
    printf(" OS WHEEL %+d (%+.2f)\n", delta, (double)delta / WHEEL_DELTA);
    break;
  }
  case WM_INPUT: {
    if (!motionDetected) break;

    UINT dwSize = 40;
    static BYTE lpb[40];

    GetRawInputData((HRAWINPUT)lParam, RID_INPUT,
      lpb, &dwSize, sizeof(RAWINPUTHEADER));

    RAWINPUT* input = (RAWINPUT*)lpb;

    if (input->header.dwType == RIM_TYPEMOUSE) {
      int dx = input->data.mouse.lLastX;
      int dy = input->data.mouse.lLastY;

      if (dx != 0 || dy != 0) {
        update(&hRawStats, dx);
        update(&vRawStats, dy);
        format(buffer1, &hRawStats);
        format(buffer2, &vRawStats);
        printf("RAW\t%+d, %+d\t%s\t%s\n", dx, dy, buffer1, buffer2);
      }

      if (input->data.mouse.usButtonFlags & RI_MOUSE_WHEEL) {
        short delta = input->data.mouse.usButtonData;
        printf("RAW WHEEL %d (%.2f)\n", delta, (double)delta / WHEEL_DELTA);
      }
    }
    break;
  }
  case WM_PAINT: {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hWnd, &ps);
    FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW + 1));
    EndPaint(hWnd, &ps);
    break;
  }
  case WM_DESTROY:
    PostQuitMessage(0);
    break;
  default:
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
  }

  return 0;
}
