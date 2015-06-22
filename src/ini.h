#ifndef _INI_H_
#define _INI_H_

#define INI_MAX_LINE_LENGTH    80

extern int get_private_profile_int(const char *, const char *, int, const char *);
extern int get_private_profile_string(const char *, const char *, char *, char *, int, const char *);

#endif
