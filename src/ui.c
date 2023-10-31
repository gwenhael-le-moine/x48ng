#include <stdio.h>
#include <stdlib.h>

#include "runtime_options.h"
#include "ui.h"
#include "ui_inner.h"

int last_annunc_state = -1;

unsigned char lcd_nibbles_buffer[ DISP_ROWS ][ NIBS_PER_BUFFER_ROW ];

letter_t small_font[ 128 ] = {
    {0,                  0,                  0                 },
    { nl_gx_width,       nl_gx_height,       nl_gx_bitmap      }, /* \001 == \n gx */
    { comma_gx_width,    comma_gx_height,    comma_gx_bitmap   }, /* \002 == comma gx */
    { arrow_gx_width,    arrow_gx_height,    arrow_gx_bitmap   }, /* \003 == \-> gx */
    { equal_gx_width,    equal_gx_height,    equal_gx_bitmap   }, /* \004 == equal gx */
    { pi_gx_width,       pi_gx_height,       pi_gx_bitmap      }, /* \005 == pi gx */
    { 0,                 0,                  0                 },
    { 0,                 0,                  0                 },
    { 0,                 0,                  0                 },
    { 0,                 0,                  0                 },
    { 0,                 0,                  0                 },
    { 0,                 0,                  0                 },
    { 0,                 0,                  0                 },
    { 0,                 0,                  0                 },
    { 0,                 0,                  0                 },
    { 0,                 0,                  0                 },
    { 0,                 0,                  0                 }, /* # 16 */
    { 0,                 0,                  0                 },
    { 0,                 0,                  0                 },
    { 0,                 0,                  0                 },
    { 0,                 0,                  0                 },
    { 0,                 0,                  0                 },
    { 0,                 0,                  0                 },
    { 0,                 0,                  0                 },
    { 0,                 0,                  0                 },
    { 0,                 0,                  0                 },
    { 0,                 0,                  0                 },
    { 0,                 0,                  0                 },
    { 0,                 0,                  0                 },
    { 0,                 0,                  0                 },
    { 0,                 0,                  0                 },
    { 0,                 0,                  0                 },
    { blank_width,       blank_height,       blank_bitmap      }, /* # 32 */
    { 0,                 0,                  0                 },
    { 0,                 0,                  0                 },
    { hash_width,        hash_height,        hash_bitmap       },
    { 0,                 0,                  0                 },
    { 0,                 0,                  0                 },
    { 0,                 0,                  0                 },
    { 0,                 0,                  0                 },
    { lbrace_width,      lbrace_height,      lbrace_bitmap     },
    { rbrace_width,      rbrace_height,      rbrace_bitmap     },
    { 0,                 0,                  0                 },
    { 0,                 0,                  0                 },
    { comma_width,       comma_height,       comma_bitmap      },
    { 0,                 0,                  0                 },
    { 0,                 0,                  0                 },
    { slash_width,       slash_height,       slash_bitmap      },
    { 0,                 0,                  0                 }, /* # 48 */
    { 0,                 0,                  0                 },
    { two_width,         two_height,         two_bitmap        },
    { three_width,       three_height,       three_bitmap      },
    { 0,                 0,                  0                 },
    { 0,                 0,                  0                 },
    { 0,                 0,                  0                 },
    { 0,                 0,                  0                 },
    { 0,                 0,                  0                 },
    { 0,                 0,                  0                 },
    { small_colon_width, small_colon_height, small_colon_bitmap},
    { 0,                 0,                  0                 },
    { 0,                 0,                  0                 },
    { equal_width,       equal_height,       equal_bitmap      },
    { 0,                 0,                  0                 },
    { 0,                 0,                  0                 },
    { 0,                 0,                  0                 }, /* # 64 */
    { A_width,           A_height,           A_bitmap          },
    { B_width,           B_height,           B_bitmap          },
    { C_width,           C_height,           C_bitmap          },
    { D_width,           D_height,           D_bitmap          },
    { E_width,           E_height,           E_bitmap          },
    { F_width,           F_height,           F_bitmap          },
    { G_width,           G_height,           G_bitmap          },
    { H_width,           H_height,           H_bitmap          },
    { I_width,           I_height,           I_bitmap          },
    { J_width,           J_height,           J_bitmap          },
    { K_width,           K_height,           K_bitmap          },
    { L_width,           L_height,           L_bitmap          },
    { M_width,           M_height,           M_bitmap          },
    { N_width,           N_height,           N_bitmap          },
    { O_width,           O_height,           O_bitmap          },
    { P_width,           P_height,           P_bitmap          }, /* # 80 */
    { Q_width,           Q_height,           Q_bitmap          },
    { R_width,           R_height,           R_bitmap          },
    { S_width,           S_height,           S_bitmap          },
    { T_width,           T_height,           T_bitmap          },
    { U_width,           U_height,           U_bitmap          },
    { V_width,           V_height,           V_bitmap          },
    { W_width,           W_height,           W_bitmap          },
    { X_width,           X_height,           X_bitmap          },
    { Y_width,           Y_height,           Y_bitmap          },
    { Z_width,           Z_height,           Z_bitmap          },
    { lbracket_width,    lbracket_height,    lbracket_bitmap   },
    { 0,                 0,                  0                 },
    { rbracket_width,    rbracket_height,    rbracket_bitmap   },
    { 0,                 0,                  0                 },
    { under_width,       under_height,       under_bitmap      },
    { 0,                 0,                  0                 }, /* # 96 */
    { arrow_width,       arrow_height,       arrow_bitmap      }, /* a == left arrow   */
    { diff_width,        diff_height,        diff_bitmap       }, /* b == differential */
    { integral_width,    integral_height,    integral_bitmap   }, /* c == integral */
    { sigma_width,       sigma_height,       sigma_bitmap      }, /* d == sigma */
    { sqr_width,         sqr_height,         sqr_bitmap        }, /* e == sqr */
    { root_width,        root_height,        root_bitmap       }, /* f == root */
    { pow10_width,       pow10_height,       pow10_bitmap      }, /* g == pow10 */
    { exp_width,         exp_height,         exp_bitmap        }, /* h == exp */
    { prog_width,        prog_height,        prog_bitmap       }, /* i == << >> */
    { string_width,      string_height,      string_bitmap     }, /* j == " " */
    { nl_width,          nl_height,          nl_bitmap         }, /* k == New Line */
    { pi_width,          pi_height,          pi_bitmap         }, /* l == pi */
    { angle_width,       angle_height,       angle_bitmap      }, /* m == angle */
    { sqr_gx_width,      sqr_gx_height,      sqr_gx_bitmap     }, /* n == sqr gx */
    { root_gx_width,     root_gx_height,     root_gx_bitmap    }, /* o == root gx */
    { pow10_gx_width,    pow10_gx_height,    pow10_gx_bitmap   }, /* p == pow10 gx */
    { exp_gx_width,      exp_gx_height,      exp_gx_bitmap     }, /* q == exp gx */
    { parens_gx_width,   parens_gx_height,   parens_gx_bitmap  }, /* r == ( ) gx */
    { hash_gx_width,     hash_gx_height,     hash_gx_bitmap    }, /* s == # gx */
    { bracket_gx_width,  bracket_gx_height,  bracket_gx_bitmap }, /* t == [] gx */
    { under_gx_width,    under_gx_height,    under_gx_bitmap   }, /* u == _ gx */
    { prog_gx_width,     prog_gx_height,     prog_gx_bitmap    }, /* v == << >> gx */
    { quote_gx_width,    quote_gx_height,    quote_gx_bitmap   }, /* w == " " gx */
    { curly_gx_width,    curly_gx_height,    curly_gx_bitmap   }, /* x == {} gx */
    { colon_gx_width,    colon_gx_height,    colon_gx_bitmap   }, /* y == :: gx */
    { angle_gx_width,    angle_gx_height,    angle_gx_bitmap   }, /* z == angle gx */
    { lcurly_width,      lcurly_height,      lcurly_bitmap     },
    { 0,                 0,                  0                 },
    { rcurly_width,      rcurly_height,      rcurly_bitmap     },
    { 0,                 0,                  0                 },
    { 0,                 0,                  0                 }
};

void ( *ui_disp_draw_nibble )( word_20 addr, word_4 val );
void ( *ui_menu_draw_nibble )( word_20 addr, word_4 val );
int ( *ui_get_event )( void );
void ( *ui_update_LCD )( void );
void ( *ui_refresh_LCD )( void );
void ( *ui_adjust_contrast )( void );
void ( *ui_draw_annunc )( void );
void ( *ui_init_LCD )( void );
void ( *init_ui )( int argc, char** argv );

void setup_frontend( void )
{
    switch ( frontend_type ) {
        case FRONTEND_X11:
        default:
#ifdef HAS_X11
            init_ui = init_x11_ui;
#else
            fprintf( stderr, "ERROR: x11 frontend disabled at compilation\n" );
            exit( -1 );
#endif
            break;

        case FRONTEND_SDL:
#ifdef HAS_SDL
            init_ui = init_sdl_ui;
#else
            fprintf( stderr, "ERROR: sdl frontend disabled at compilation\n" );
            exit( -1 );
#endif
            break;

        case FRONTEND_TEXT:
            init_ui = init_text_ui;
            break;
    }
}

int SmallTextWidth( const char* string, unsigned int length )
{
    unsigned int i;
    int w;

    w = 0;
    for ( i = 0; i < length; i++ ) {
        if ( small_font[ ( int )string[ i ] ].h != 0 ) {
            w += small_font[ ( int )string[ i ] ].w + 1;
        } else {
            if ( verbose )
                fprintf( stderr, "Unknown small letter 0x00%x\n", ( int )string[ i ] );
            w += 5;
        }
    }

    return w;
}
