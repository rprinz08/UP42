#ifndef _INI_H_
#define _INI_H_

#define INI_MAX_LINE_LENGTH    80

extern int get_private_profile_int(char *, char *, int, char *);
extern int get_private_profile_string(char *, char *, char *, char *, int, char *);
extern int write_private_profile_string(char *, char *, char *, char *);

#endif
