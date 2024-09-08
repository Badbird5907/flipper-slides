#pragma once

#include <gui/view.h>

typedef struct Remote Remote;
typedef struct RemoteKeynote RemoteKeynote;

RemoteKeynote* remote_keynote_alloc(Remote* bt_hid);

void remote_keynote_free(RemoteKeynote* remote_keynote);

View* remote_keynote_get_view(RemoteKeynote* remote_keynote);

void remote_keynote_set_connected_status(RemoteKeynote* remote_keynote, bool connected);

void remote_keynote_set_orientation(RemoteKeynote* remote_keynote, bool vertical);
