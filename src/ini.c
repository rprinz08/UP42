#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "ini.h"



/**************************************************************************
* Function:     read_line()
* Arguments:    <FILE *> fp - a pointer to the file to be read from
*               <char *> bp - a pointer to the copy buffer
* Returns:      TRUE if successful FALSE otherwise
***************************************************************************/
int read_line(FILE *fp, char *bp)
{
    char c = '\0';
    int i = 0;

    /* Read one line from the source file */
    while( (c = getc(fp)) != '\n' ) {
         if( c == EOF )         /* return FALSE on unexpected EOF */
              return(0);
         bp[i++] = c;
    }
    bp[i] = '\0';
    return(1);
}



/**************************************************************************
* Function:     get_private_profile_int()
* Arguments:    <char *> section - the name of the section to search for
*               <char *> entry - the name of the entry to find the value of
*               <int> def - the default value in the event of a failed read
*               <char *> file_name - the name of the .ini file to read from
* Returns:      the value located at entry
***************************************************************************/
int get_private_profile_int(char *section, char *entry,
                            int def, char *file_name)
{
    FILE *fp = fopen(file_name,"r");
    char buff[INI_MAX_LINE_LENGTH];
    char *ep;
    char t_section[INI_MAX_LINE_LENGTH];
    char value[6];
    int len = strlen(entry);
    int i;

    if( !fp )
         return(0);
    sprintf(t_section,"[%s]",section); /* Format the section name */
    /*  Move through file 1 line at a time until a section is matched or EOF */
    do {
         if( !read_line(fp,buff) ) {
              fclose(fp);
              return(def);
         }
    } while( stricmp(buff,t_section) );
    /* Now that the section has been found, find the entry.
	* Stop searching upon leaving the section's area. */
    do {
	    if( !read_line(fp,buff) || buff[0] == '[' ) {
		    fclose(fp);
		    return(def);
	    }
    } while( strnicmp(buff,entry,len) );
    ep = strchr(buff,'=');    /* Parse out the equal sign */
    ep++;
    if( !strlen(ep) )          /* No setting? */
	    return(def);
    /* Copy only numbers fail on characters */

    for(i = 0; isdigit(ep[i]); i++ )
	    value[i] = ep[i];
    value[i] = '\0';
    fclose(fp);                /* Clean up and return the value */
    return(atoi(value));
}



/**************************************************************************
* Function:     get_private_profile_string()
* Arguments:    <char *> section - the name of the section to search for
*               <char *> entry - the name of the entry to find the value of
*               <char *> def - default string in the event of a failed read
*               <char *> buffer - a pointer to the buffer to copy into
*               <int> buffer_len - the max number of characters to copy
*               <char *> file_name - the name of the .ini file to read from
* Returns:      the number of characters copied into the supplied buffer
***************************************************************************/
int get_private_profile_string(char *section, char *entry, char *def,
                               char *buffer, int buffer_len, char *file_name)
{
    FILE *fp = fopen(file_name,"r");
    char buff[INI_MAX_LINE_LENGTH];
    char *p,*ep;
    char t_section[INI_MAX_LINE_LENGTH];
    int len = strlen(entry);
    int i;

    if( !fp )
         return(0);
    sprintf(t_section,"[%s]",section);    /* Format the section name */
    /*  Move through file 1 line at a time until a section is matched or EOF */
    do {
         if( !read_line(fp,buff) ) {
              fclose(fp);
              if(def == NULL)
                return 0;
              strncpy(buffer,def,buffer_len);
              return(strlen(buffer));
         }
    }
    while( stricmp(buff,t_section) );
    /* Now that the section has been found, find the entry.
     * Stop searching upon leaving the section's area. */
    do {
	    if( !read_line(fp,buff) || buff[0] == '[' ) {
		    fclose(fp);
            if(def == NULL)
                return 0;
		    strncpy(buffer,def,buffer_len);
		    return(strlen(buffer));
	    }
    } while( strnicmp(buff,entry,len) );
    ep = strchr(buff,'=');    /* Parse out the equal sign */
    ep++;
    p=buffer;
    /* Copy up to buffer_len chars to buffer */
    i=1;
    while(*ep != '\0' && i <= buffer_len-1) {
            *buffer++ = *ep++;
            i++;
    }
    *buffer='\0';
    fclose(fp);               /* Clean up and return the amount copied */
    return(strlen(p));
}

