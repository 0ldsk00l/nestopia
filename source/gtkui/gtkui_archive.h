#ifndef _GTKUI_ARCHIVE_H_
#define _GTKUI_ARCHIVE_H_

bool gtkui_archive_handle(const char *filename, char *reqfile, size_t reqsize);
void gtkui_archive_ok();
void gtkui_archive_cancel();

#endif
