#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/XInput2.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

typedef struct {
  int number;
  Atom label;
  int increment;
  Bool absolute;
  double value;
} Valuator;

Valuator valuators[32][32];
Bool motion_detected;
int last_x, last_y;

void processEvents(Display *display, Window window, int xi_opcode, Atom *wm_delete_window) {
  while (True) {
    XEvent event;
    XGenericEventCookie *cookie = &event.xcookie;
    XNextEvent(display, &event);

    if (event.type == ClientMessage && (Atom)event.xclient.data.l[0] == *wm_delete_window) {
      XFreeEventData(display, cookie);
      return;
    }

    if (event.type == MotionNotify) {
      XMotionEvent *motion_event = (XMotionEvent *) &event;
      int dx = motion_event->x - last_x;
      int dy = motion_event->y - last_y;
      if (motion_detected) {
        if (dx != 0 || dy != 0) {
          printf("OS\t%+d, %+d\n", dx, dy);
        }
      } else {
        motion_detected = True;
      }
      last_x = motion_event->x;
      last_y = motion_event->y;
    }

    if (event.type == LeaveNotify) {
      motion_detected = False;
    }

    if (XGetEventData(display, cookie) && cookie->type == GenericEvent && cookie->extension == xi_opcode) {
      XIDeviceEvent *device_event = cookie->data;

      switch (device_event->evtype) {
        case XI_RawMotion: {
          if (!motion_detected) break;

          XIRawEvent *raw_event = (XIRawEvent *) device_event;
          double *raw_value = raw_event->raw_values;

          XIValuatorState *states = &raw_event->valuators;

          for (int i = 0; i < states->mask_len * 8; i++) {
            if (XIMaskIsSet(states->mask, i)) {
              Valuator *valuator = &valuators[device_event->sourceid][i];
              char *name = XGetAtomName(display, valuator->label);
              if (strcmp("Rel X", name) == 0 || strcmp("Rel Y", name) == 0 ||
                  strcmp("Abs X", name) == 0 || strcmp("Abs Y", name) == 0) {
                float delta = valuator->absolute ? (*raw_value) - valuator->value: *raw_value;
                printf("%s\t%+.2f\n", name, delta);
              }
              XFree(name);
              valuator->value = *raw_value;
              raw_value++;
            }
          }
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

void queryValuators(Display *display) {
  int ndevices;
  XIDeviceInfo *device_info = XIQueryDevice(display, XIAllDevices, &ndevices);

  for (int i = 0; i < ndevices; i++) {
    for (int j = 0; j < device_info[i].num_classes; j++) {
      XIAnyClassInfo *class_info = device_info[i].classes[j];
      switch (class_info->type) {
        case XIValuatorClass: {
          XIValuatorClassInfo *valuator_info = (XIValuatorClassInfo *) class_info;
          valuators[device_info[i].deviceid][valuator_info->number] =
            (Valuator) { .label = valuator_info->label,
                         .increment = 0,
                         .absolute = valuator_info->mode == XIModeAbsolute,
                         .value = 0.0 };
          break;
        }
        case XIScrollClass: {
          XIScrollClassInfo *scroll_info = (XIScrollClassInfo *) class_info;
          valuators[device_info[i].deviceid][scroll_info->number].increment = scroll_info->increment;
          break;
        }
      }
    }
  }
  XIFreeDeviceInfo(device_info);
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

  queryValuators(display);

  Window window = XCreateSimpleWindow(display, DefaultRootWindow(display),
    0, 0, 640, 480, 0, 0, WhitePixel(display, 0));

  if (!window) {
    fprintf(stderr, "Error creating windows.\n");
    return -1;
  }

  selectRawMotionEvents(display);

  XSelectInput(display, window, PointerMotionMask | LeaveWindowMask);

  XStoreName(display, window, "Mouse Logger");

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
