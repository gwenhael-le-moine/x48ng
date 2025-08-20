#ifndef _UI4x_COMMON_H
#define _UI4x_COMMON_H 1

#include "../options.h"

/*************************************************/
/* public API: if it's there it's used elsewhere */
/*************************************************/
extern void ( *ui_get_event )( void );
extern void ( *ui_update_display )( void );

extern void ( *ui_start )( config_t* conf );
extern void ( *ui_stop )( void );

extern void setup_ui( config_t* conf );
extern void close_and_exit( void );

#endif /* !_UI4x_COMMON_H */
