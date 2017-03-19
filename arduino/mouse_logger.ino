// Use https://github.com/pavelfatin/USB_Host_Shield_2.0 to override the polling frequency.

#include <hidboot.h>
#include <usbhub.h>
#include <Mouse.h>

typedef struct {
  int count;
  int total;
  int minimum;
  int maximum;
} Stats;

Stats hStats, vStats;

char buffer1[128];
char buffer2[128];
char buffer3[256];

void reset(Stats *stats) {
  stats->count = 0;
  stats->total = 0;
  stats->minimum = 32767;
  stats->maximum = 0;
}

void resetStats() {
  reset(&hStats);
  reset(&vStats);
}

void doUpdate(Stats *stats, int value) {
  stats->count++;
  stats->total += value;
  if (value < stats->minimum) {
    stats->minimum = value;
  }
  if (value > stats->maximum) {
    stats->maximum = value;
  }
}

void update(Stats *stats, int value) {
  if (value != 0) {
    doUpdate(stats, abs(value));
  }
}

void format(char *buffer, Stats *stats) {
  char average[7];
  dtostrf(stats->count == 0 ? 0.0 : ((double)stats->total / stats->count), 5, 2, average);

  sprintf(buffer, "% 2d <%s <% 3d",
    stats->count == 0 ? 0 : stats->minimum,
    average,
    stats->maximum);
}

class MouseParser : public MouseReportParser {
protected:
  void OnMouseMove(MOUSEINFO *info);
  void OnLeftButtonUp(MOUSEINFO *info);
  void OnLeftButtonDown(MOUSEINFO *info);
  void OnRightButtonUp(MOUSEINFO *info);
  void OnRightButtonDown(MOUSEINFO *info);
  void OnMiddleButtonUp(MOUSEINFO *info);
  void OnMiddleButtonDown(MOUSEINFO *info);
};

void MouseParser::OnMouseMove(MOUSEINFO *info) {
  int dx = info->dX;
  int dy = info->dY;

  if (dx != 0 || dy != 0) {
    update(&hStats, dx);
    update(&vStats, dy);
    format(buffer1, &hStats);
    format(buffer2, &vStats);
    sprintf(buffer3, "%+d, %+d\t%s\t%s", dx, dy, buffer1, buffer2);
    Serial.println(buffer3);
  }

  Mouse.move(dx, dy);
}

void MouseParser::OnLeftButtonDown(MOUSEINFO *info) {
  resetStats();
  Serial.println("Reset.");

  Mouse.press(MOUSE_LEFT);
}

void MouseParser::OnLeftButtonUp(MOUSEINFO *info) {
  Mouse.release(MOUSE_LEFT);
}

void MouseParser::OnRightButtonDown(MOUSEINFO *info) {
  resetStats();
  Serial.println("Reset.");

  Mouse.press(MOUSE_RIGHT);
}

void MouseParser::OnRightButtonUp(MOUSEINFO *info) {
  Mouse.release(MOUSE_RIGHT);
}

void MouseParser::OnMiddleButtonDown(MOUSEINFO *info) {
  Mouse.press(MOUSE_MIDDLE);
}

void MouseParser::OnMiddleButtonUp(MOUSEINFO *info) {
  Mouse.release(MOUSE_MIDDLE);
}

USB Usb;
USBHub Hub(&Usb);
HIDBoot<USB_HID_PROTOCOL_MOUSE> HidMouse(&Usb, false, 1);
MouseParser Parser;

void setup() {
  resetStats();

  Mouse.begin();

  Usb.Init();
  delay(200);
  HidMouse.SetReportParser(0, &Parser);
}

void loop() {
  Usb.Task();
}
