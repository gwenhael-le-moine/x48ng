#include <locale.h>
#include <wchar.h>

#include <curses.h>

#include "api.h"
#include "inner.h"

#define LCD_OFFSET_X ( ui4x_config.chromeless ? 0 : 1 )
#define LCD_OFFSET_Y 1
#define LCD_BOTTOM LCD_OFFSET_Y + ( LCD_HEIGHT / ( ui4x_config.tiny ? 4 : ( ui4x_config.small ? 2 : 1 ) ) )
#define LCD_RIGHT LCD_OFFSET_X + ( LCD_WIDTH / ( ui4x_config.small || ui4x_config.tiny ? 2 : 1 ) ) + 1

/* typedef enum { */
/*     LCD_COLOR_BG = 30, */
/*     LCD_COLOR_FG_0x1, */
/*     LCD_COLOR_FG_0x2, */
/*     LCD_COLOR_FG_0x3, */
/*     LCD_COLOR_FG_0x4, */
/*     LCD_COLOR_FG_0x5, */
/*     LCD_COLOR_FG_0x6, */
/*     LCD_COLOR_FG_0x7, */
/*     LCD_COLOR_FG_0x8, */
/*     LCD_COLOR_FG_0x9, */
/*     LCD_COLOR_FG_0xA, */
/*     LCD_COLOR_FG_0xB, */
/*     LCD_COLOR_FG_0xC, */
/*     LCD_COLOR_FG_0xD, */
/*     LCD_COLOR_FG_0xE, */
/*     LCD_COLOR_FG_0xF */
/* } nc_color_t; */

/* typedef enum { */
/*     LCD_PIXEL_OFF = 60, */
/*     LCD_PIXEL_ON_0x1, */
/*     LCD_PIXEL_ON_0x2, */
/*     LCD_PIXEL_ON_0x3, */
/*     LCD_PIXEL_ON_0x4, */
/*     LCD_PIXEL_ON_0x5, */
/*     LCD_PIXEL_ON_0x6, */
/*     LCD_PIXEL_ON_0x7, */
/*     LCD_PIXEL_ON_0x8, */
/*     LCD_PIXEL_ON_0x9, */
/*     LCD_PIXEL_ON_0xA, */
/*     LCD_PIXEL_ON_0xB, */
/*     LCD_PIXEL_ON_0xC, */
/*     LCD_PIXEL_ON_0xD, */
/*     LCD_PIXEL_ON_0xE, */
/*     LCD_PIXEL_ON_0xF */
/* } nc_color_pair_t; */

/*************/
/* variables */
/*************/
static int display_buffer_grayscale[ LCD_WIDTH * 80 ];
static char last_annunciators = -1;

static bool keyboard_state[ NB_HP4950_KEYS ];

static WINDOW* lcd_window;
static WINDOW* help_window;

/****************************/
/* functions implementation */
/****************************/
static inline wchar_t eight_bits_to_braille_char( bool b1, bool b2, bool b3, bool b4, bool b5, bool b6, bool b7, bool b8 )
{
    /*********/
    /* b1 b4 */
    /* b2 b5 */
    /* b3 b6 */
    /* b7 b8 */
    /*********/
    wchar_t chr = 0x2800;

    if ( b1 )
        chr |= 0b0000000000000001;
    if ( b2 )
        chr |= 0b0000000000000010;
    if ( b3 )
        chr |= 0b0000000000000100;
    if ( b4 )
        chr |= 0b0000000000001000;
    if ( b5 )
        chr |= 0b0000000000010000;
    if ( b6 )
        chr |= 0b0000000000100000;
    if ( b7 )
        chr |= 0b0000000001000000;
    if ( b8 )
        chr |= 0b0000000010000000;

    return chr;
}

static inline void ncurses_draw_lcd_tiny( void )
{
    bool b1, b2, b3, b4, b5, b6, b7, b8;
    int step_x = 2;
    int step_y = 4;
    bool last_column = false;

    wchar_t line[ 66 ]; /* ( LCD_WIDTH / step_x ) + 1 */
    wchar_t pixels;

    /* if ( !ui4x_config.mono && has_colors() ) */
    /*     attron( COLOR_PAIR( COLOR_RED ) ); */

    for ( int y = 0; y < LCD_HEIGHT; y += step_y ) {
        wcscpy( line, L"" );

        for ( int x = 0; x < LCD_WIDTH; x += step_x ) {
            last_column = x == ( LCD_WIDTH - 1 );

            b1 = display_buffer_grayscale[ ( y * LCD_WIDTH ) + x ] > 0;
            b2 = display_buffer_grayscale[ ( ( y + 1 ) * LCD_WIDTH ) + x ] > 0;
            b3 = display_buffer_grayscale[ ( ( y + 2 ) * LCD_WIDTH ) + x ] > 0;
            b7 = display_buffer_grayscale[ ( ( y + 3 ) * LCD_WIDTH ) + x ] > 0;

            if ( last_column )
                b4 = b5 = b6 = b8 = 0;
            else {
                b4 = display_buffer_grayscale[ ( y * LCD_WIDTH ) + x + 1 ] > 0 ? 1 : 0;
                b5 = display_buffer_grayscale[ ( ( y + 1 ) * LCD_WIDTH ) + x + 1 ] > 0 ? 1 : 0;
                b6 = display_buffer_grayscale[ ( ( y + 2 ) * LCD_WIDTH ) + x + 1 ] > 0 ? 1 : 0;
                b8 = display_buffer_grayscale[ ( ( y + 3 ) * LCD_WIDTH ) + x + 1 ] > 0 ? 1 : 0;
            }

            pixels = eight_bits_to_braille_char( b1, b2, b3, b4, b5, b6, b7, b8 );
            wcsncat( line, &pixels, 1 );
        }
        mvwaddwstr( lcd_window, LCD_OFFSET_Y + ( y / step_y ), LCD_OFFSET_X, line );
    }

    /* if ( !ui4x_config.mono && has_colors() ) */
    /*     attroff( COLOR_PAIR( COLOR_RED ) ); */
}

static inline wchar_t four_bits_to_quadrant_char( bool top_left, bool top_right, bool bottom_left, bool bottom_right )
{
    if ( top_left ) {
        if ( top_right ) {
            if ( bottom_left )
                return bottom_right ? L'█' : L'▛'; /* 0x2588 0x2598 */
            else
                return bottom_right ? L'▜' : L'▀'; /* 0x259C 0x2580 */
        } else {
            if ( bottom_left )
                return bottom_right ? L'▙' : L'▌';
            else
                return bottom_right ? L'▚' : L'▘';
        }
    } else {
        if ( top_right ) {
            if ( bottom_left )
                return bottom_right ? L'▟' : L'▞';
            else
                return bottom_right ? L'▐' : L'▝';
        } else {
            if ( bottom_left )
                return bottom_right ? L'▄' : L'▖';
            else
                return bottom_right ? L'▗' : L' ';
        }
    }
}

static inline void ncurses_draw_lcd_small( void )
{
    bool top_left, top_right, bottom_left, bottom_right;
    int step_x = 2;
    int step_y = 2;
    bool last_column = false;

    wchar_t line[ 66 ]; /* ( LCD_WIDTH / step_x ) + 1 */
    wchar_t pixels;

    /* if ( !ui4x_config.mono && has_colors() ) */
    /*     attron( COLOR_PAIR( COLOR_RED ) ); */

    for ( int y = 0; y < LCD_HEIGHT; y += step_y ) {
        wcscpy( line, L"" );

        for ( int x = 0; x < LCD_WIDTH; x += step_x ) {
            last_column = x == ( LCD_WIDTH - 1 );

            top_left = display_buffer_grayscale[ ( y * LCD_WIDTH ) + x ] > 0 ? 1 : 0;
            bottom_left = display_buffer_grayscale[ ( ( y + 1 ) * LCD_WIDTH ) + x ] > 0 ? 1 : 0;

            if ( last_column )
                top_right = bottom_right = 0;
            else {
                top_right = display_buffer_grayscale[ ( y * LCD_WIDTH ) + x + 1 ] > 0 ? 1 : 0;
                bottom_right = display_buffer_grayscale[ ( ( y + 1 ) * LCD_WIDTH ) + x + 1 ] > 0 ? 1 : 0;
            }

            pixels = four_bits_to_quadrant_char( top_left, top_right, bottom_left, bottom_right );
            wcsncat( line, &pixels, 1 );
        }
        mvwaddwstr( lcd_window, LCD_OFFSET_Y + ( y / step_y ), LCD_OFFSET_X, line );
    }

    /* if ( !ui4x_config.mono && has_colors() ) */
    /*     attroff( COLOR_PAIR( COLOR_RED ) ); */
}

static inline void ncurses_draw_lcd_fullsize( void )
{
    int val;
    /* int color = COLOR_RED; */
    wchar_t pixel;

    wchar_t line[ LCD_WIDTH ];

    /* if ( !ui4x_config.mono && has_colors() ) */
    /*     attron( COLOR_PAIR( color ) ); */

    for ( int y = 0; y < LCD_HEIGHT; y++ ) {
        wcscpy( line, L"" );
        for ( int x = 0; x < LCD_WIDTH; x++ ) {
            val = display_buffer_grayscale[ ( y * LCD_WIDTH ) + x ];
            if ( ui4x_config.model == MODEL_50G )
                val /= 3;
            else if ( val == 3 )
                val = 4;

            switch ( val ) {
                case 0:
                    pixel = L' ';
                    break;
                case 1:
                    pixel = L'░';
                    break;
                case 2:
                    pixel = L'▒';
                    break;
                case 3:
                    pixel = L'▓';
                    break;
                case 4:
                default:
                    pixel = L'█';
                    break;
            }

            wcsncat( line, &pixel, 1 );
        }
        mvwaddwstr( lcd_window, LCD_OFFSET_Y + y, LCD_OFFSET_X, line );
    }

    /* if ( !ui4x_config.mono && has_colors() ) */
    /*     attroff( COLOR_PAIR( color ) ); */
}

static void toggle_help_window( void )
{
    int border_width = ui4x_config.chromeless ? 0 : 1;

    if ( help_window == NULL ) {
        help_window = newwin( 7, LCD_RIGHT + border_width, LCD_BOTTOM + 1, 0 );
        refresh();

        if ( !ui4x_config.chromeless )
            wborder( help_window, 0, 0, 0, 0, 0, 0, 0, 0 );

        mvwprintw( help_window, 0, 1 + border_width, "[ Help ]" );
        mvwprintw( help_window, 1, border_width, "Special keys:" );
        mvwprintw( help_window, 2, 1 + border_width, "F1: Help, F7: Quit" );

        mvwprintw( help_window, 3, border_width, "Calculator keys:" );
        mvwprintw( help_window, 4, 1 + border_width, "all alpha-numerical keys " );
        mvwprintw( help_window, 5, 1 + border_width, "F2: Left-Shift, F3: Right-Shift, F4: Alpha, F5: On, F6: Enter" );

        wrefresh( help_window );
    } else {
        wclear( help_window );
        wrefresh( help_window );
        refresh();

        help_window = NULL;
    }
}

static void ncurses_refresh_annunciators( void )
{
    int annunciators = ui4x_emulator_api.get_annunciators();

    if ( last_annunciators == annunciators )
        return;

    last_annunciators = annunciators;

    for ( int i = 0; i < NB_ANNUNCIATORS; i++ )
        mvwaddstr( lcd_window, 0, 4 + ( i * 4 ), ( ( annunciators >> i ) & 0x01 ) ? ui_annunciators[ i ] : " " );
}

/**********/
/* Public */
/**********/
void ncurses_refresh_lcd( void )
{
    if ( !ui4x_emulator_api.is_display_on() )
        return;

    ncurses_refresh_annunciators();

    ui4x_emulator_api.get_lcd_buffer( display_buffer_grayscale );

    if ( ui4x_config.small )
        ncurses_draw_lcd_small();
    else if ( ui4x_config.tiny )
        ncurses_draw_lcd_tiny();
    else
        ncurses_draw_lcd_fullsize();

    wrefresh( lcd_window );
}

void ncurses_handle_pending_inputs( void )
{
    bool new_keyboard_state[ NB_HP4950_KEYS ];
    uint32_t k;

    // each run records the state of the keyboard (pressed keys)
    // This allow to diff with previous state and issue PRESS and RELEASE calls
    for ( int key = 0; key < NB_KEYS; key++ )
        new_keyboard_state[ key ] = false;

    // READ KB STATE
    /* Iterate over all currently pressed keys and mark them as pressed */
    while ( ( k = getch() ) ) {
        if ( k == ( uint32_t )ERR )
            break;

        switch ( k ) {
            case '0':
                new_keyboard_state[ UI4X_KEY_0 ] = true;
                break;
            case '1':
                new_keyboard_state[ UI4X_KEY_1 ] = true;
                break;
            case '2':
                new_keyboard_state[ UI4X_KEY_2 ] = true;
                break;
            case '3':
                new_keyboard_state[ UI4X_KEY_3 ] = true;
                break;
            case '4':
                new_keyboard_state[ UI4X_KEY_4 ] = true;
                break;
            case '5':
                new_keyboard_state[ UI4X_KEY_5 ] = true;
                break;
            case '6':
                new_keyboard_state[ UI4X_KEY_6 ] = true;
                break;
            case '7':
                new_keyboard_state[ UI4X_KEY_7 ] = true;
                break;
            case '8':
                new_keyboard_state[ UI4X_KEY_8 ] = true;
                break;
            case '9':
                new_keyboard_state[ UI4X_KEY_9 ] = true;
                break;
            case 'a':
                new_keyboard_state[ UI4X_KEY_A ] = true;
                break;
            case 'b':
                new_keyboard_state[ UI4X_KEY_B ] = true;
                break;
            case 'c':
                new_keyboard_state[ UI4X_KEY_C ] = true;
                break;
            case 'd':
                new_keyboard_state[ UI4X_KEY_D ] = true;
                break;
            case 'e':
                new_keyboard_state[ UI4X_KEY_E ] = true;
                break;
            case 'f':
                new_keyboard_state[ UI4X_KEY_F ] = true;
                break;
            case 'g':
                new_keyboard_state[ UI4X_KEY_G ] = true;
                break;
            case 'h':
                new_keyboard_state[ UI4X_KEY_H ] = true;
                break;
            case 'i':
                new_keyboard_state[ UI4X_KEY_I ] = true;
                break;
            case 'j':
                new_keyboard_state[ UI4X_KEY_J ] = true;
                break;
            case 'k':
                new_keyboard_state[ UI4X_KEY_K ] = true;
                break;
            case KEY_UP:
                new_keyboard_state[ UI4X_KEY_UP ] = true;
                break;
            case 'l':
                new_keyboard_state[ UI4X_KEY_L ] = true;
                break;
            case 'm':
                new_keyboard_state[ UI4X_KEY_M ] = true;
                break;
            case 'n':
                new_keyboard_state[ UI4X_KEY_N ] = true;
                break;
            case 'o':
                new_keyboard_state[ UI4X_KEY_O ] = true;
                break;
            case 'p':
                new_keyboard_state[ UI4X_KEY_P ] = true;
                break;
            case KEY_LEFT:
                new_keyboard_state[ UI4X_KEY_LEFT ] = true;
                break;
            case 'q':
                new_keyboard_state[ UI4X_KEY_Q ] = true;
                break;
            case KEY_DOWN:
                new_keyboard_state[ UI4X_KEY_DOWN ] = true;
                break;
            case 'r':
                new_keyboard_state[ UI4X_KEY_R ] = true;
                break;
            case KEY_RIGHT:
                new_keyboard_state[ UI4X_KEY_RIGHT ] = true;
                break;
            case 's':
                new_keyboard_state[ UI4X_KEY_S ] = true;
                break;
            case 't':
                new_keyboard_state[ UI4X_KEY_T ] = true;
                break;
            case 'u':
                new_keyboard_state[ UI4X_KEY_U ] = true;
                break;
            case 'v':
                new_keyboard_state[ UI4X_KEY_V ] = true;
                break;
            case 'w':
                new_keyboard_state[ UI4X_KEY_W ] = true;
                break;
            case 'x':
                new_keyboard_state[ UI4X_KEY_X ] = true;
                break;
            case 'y':
                new_keyboard_state[ UI4X_KEY_Y ] = true;
                break;
            case 'z':
            case '/':
                new_keyboard_state[ UI4X_KEY_Z ] = true;
                break;
            case ' ':
                new_keyboard_state[ UI4X_KEY_SPACE ] = true;
                break;
            case KEY_DC:
            case KEY_BACKSPACE:
            case 127:
            case '\b':
                new_keyboard_state[ UI4X_KEY_BACKSPACE ] = true;
                break;
            case '.':
                new_keyboard_state[ UI4X_KEY_PERIOD ] = true;
                break;
            case '+':
                new_keyboard_state[ UI4X_KEY_PLUS ] = true;
                break;
            case '-':
                new_keyboard_state[ UI4X_KEY_MINUS ] = true;
                break;
            case '*':
                new_keyboard_state[ UI4X_KEY_MULTIPLY ] = true;
                break;

            case KEY_F( 2 ):
            case '[':
            case 339: /* PgUp */
                new_keyboard_state[ UI4X_KEY_LSHIFT ] = true;
                break;
            case KEY_F( 3 ):
            case ']':
            case 338: /* PgDn */
                new_keyboard_state[ UI4X_KEY_RSHIFT ] = true;
                break;
            case KEY_F( 4 ):
            case ';':
            case KEY_IC: /* Ins */
                new_keyboard_state[ UI4X_KEY_ALPHA ] = true;
                break;
            case KEY_F( 5 ):
            case '\\':
            case 27:  /* Esc */
            case 262: /* Home */
                new_keyboard_state[ UI4X_KEY_ON ] = true;
                break;
            case KEY_F( 6 ):
            case KEY_ENTER:
            case '\n':
            case ',':
            case 13:
                new_keyboard_state[ UI4X_KEY_ENTER ] = true;
                break;

            case KEY_F( 1 ):
                toggle_help_window();
                break;

            case KEY_F( 7 ):
            case '|':      /* Shift+\ */
            case KEY_SEND: /* Shift+End */
            case KEY_F( 10 ):
                ui4x_emulator_api.do_stop();
                break;

            case KEY_F( 12 ):
                ui4x_emulator_api.do_reset();
                break;
        }
    }

    for ( int key = 0; key < NB_KEYS; key++ ) {
        if ( keyboard_state[ key ] == new_keyboard_state[ key ] )
            continue; /* key hasn't changed state */

        if ( !keyboard_state[ key ] && new_keyboard_state[ key ] && !ui4x_emulator_api.is_key_pressed( key ) )
            ui4x_emulator_api.press_key( key );
        else if ( keyboard_state[ key ] && !new_keyboard_state[ key ] && ui4x_emulator_api.is_key_pressed( key ) )
            ui4x_emulator_api.release_key( key );

        keyboard_state[ key ] = new_keyboard_state[ key ];
    }
}

void ncurses_exit( void )
{
    delwin( lcd_window );
    delwin( help_window );

    nodelay( stdscr, FALSE );
    echo();
    endwin();
}

void ncurses_init( void )
{
    for ( int i = 0; i < NB_KEYS; ++i )
        keyboard_state[ i ] = false;

    setlocale( LC_ALL, "" );
    initscr();              /* initialize the curses library */
    keypad( stdscr, TRUE ); /* enable keyboard mapping */
    nodelay( stdscr, TRUE );
    curs_set( 0 );
    cbreak(); /* take input chars one at a time, no wait for \n */
    noecho();
    nonl(); /* tell curses not to do NL->CR/NL on output */

    /* if ( !ui4x_config.mono && has_colors() ) { */
    /*     start_color(); */

    /*     int step = 1000 / 15; */
    /*     int rgb = 0; */
    /*     for ( int i = 0; i < 16; i++ ) { */
    /*         rgb = ( i * step ); */
    /*         init_color( LCD_COLOR_BG + i, 0, rgb, 0 ); */

    /*         init_pair( LCD_PIXEL_OFF + i, LCD_COLOR_BG + i, COLOR_BLACK ); */
    /*     } */
    /* } */

    lcd_window = newwin( LCD_BOTTOM + 1, LCD_RIGHT + 1, 0, 0 );
    refresh();

    if ( !ui4x_config.chromeless ) {
        wborder( lcd_window, 0, 0, 0, 0, 0, 0, 0, 0 );

        toggle_help_window();
    }

    mvwprintw( lcd_window, 0, 2, "[   |   |   |   |   |   ]" ); /* annunciators */
    mvwprintw( lcd_window, 0, 32, "< %s v%i.%i.%i >", ui4x_config.name, VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL );

    wrefresh( lcd_window );
}
