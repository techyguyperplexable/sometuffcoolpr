#ifndef STATUS_H
#define STATUS_H

// void status_ok(const char* message);
// void status_error(const char* message);
// void status_info(const char* message);
void status_end_ok(void);
void status_end_error(void);
void status_begin(const char* message);

#endif
