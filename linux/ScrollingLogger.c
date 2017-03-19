#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/XInput2.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void processEvents(Display *display, Window window, int xi_opcode, Atom *wm_delete_window) {
  while (True) {
    XEvent event;
    XGenericEventCookie *cookie = &event.xcookie;
    XNextEvent(display, &event);

    if (event.type == ClientMessage && (Atom)event.xclient.data.l[0] == *wm_delete_window) {
      XFreeEventData(display, cookie);
      return;
    }

    if (XGetEventData(display, cookie) && cookie->type == GenericEvent && cookie->extension == xi_opcode) {
      XIDeviceEvent *device_event = cookie->data;

      switch (device_event->evtype) {
        case XI_RawMotion: {
          XIRawEvent *raw_event = (XIRawEvent *) device_event;
          double *raw_value = raw_event->raw_values;

          XIValuatorState *states = &raw_event->valuators;

          double *value = states->values;

          for (int i = 0; i < states->mask_len * 8; i++) {
            if (XIMaskIsSet(states->mask, i)) {
              printf("%d\t%.2f\t%.2f\n", i, *raw_value, *value);
              raw_value++;
              value++;
            }
          }
          break;
        }
      }
    }

    XFreeEventData(display, cookie);
  }
}

void selectRawMotionEvents(Display *display) {
  unsigned char mask[XIMaskLen(XI_RawMotion)] = { 0 };
  XISetMask(mask, XI_RawMotion);

  XIEventMask eventmask;
  eventmask.deviceid = XIAllMasterDevices;
  eventmask.mask_len = sizeof(mask);
  eventmask.mask = mask;

  Window window = DefaultRootWindow(display);

  XISelectEvents(display, window, &eventmask, 1);
}

int main() {
  Display *display = XOpenDisplay(0);

  if (!display) {
    fprintf(stderr, "Error opening display\n");
    return -1;
  }

  int xi_opcode, xi_firstevent, xi_firsterror;

  if (!XQueryExtension(display, "XInputExtension", &xi_opcode, &xi_firstevent, &xi_firsterror)) {
    fprintf(stderr, "XInput extension is not supported by server.\n");
    return -1;
  }

  int major = 2, minor = 2;

  int result = XIQueryVersion(display, &major, &minor);

  if (result == BadRequest) {
    fprintf(stderr, "XInput 2.0+ required, supported by server: %d.%d\n", major, minor);
    return -1;
  } else if (result != Success) {
    fprintf(stderr, "Error reading XInput version.\n");
    return -1;
  }

  Window window = XCreateSimpleWindow(display, DefaultRootWindow(display),
    0, 0, 640, 480, 0, 0, WhitePixel(display, 0));

  if (!window) {
    fprintf(stderr, "Error creating windows.\n");
    return -1;
  }

  selectRawMotionEvents(display);

  XStoreName(display, window, "Scrolling Logger");

  Atom net_wm_window_type = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
  Atom atoms[1] = { None };
  atoms[0] = XInternAtom (display, "_NET_WM_WINDOW_TYPE_DIALOG", False);
  XChangeProperty(display, window, net_wm_window_type, XA_ATOM, 32, PropModeReplace, (unsigned char *)atoms, 1);

  Atom wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(display, window, &wm_delete_window, 1);

  XMapWindow(display, window);
  XFlush(display);

  processEvents(display, window, xi_opcode, &wm_delete_window);
  XCloseDisplay(display);

  return 0;
}
