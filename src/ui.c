#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "ui.h"
#include "ui_inner.h"

int last_annunc_state = -1;

unsigned char lcd_nibbles_buffer[ DISP_ROWS ][ NIBS_PER_BUFFER_ROW ];

letter_t small_font[ 128 ] = {
    {0,                 0,                  0                 },
    {nl_gx_width,       nl_gx_height,       nl_gx_bitmap      }, /* \001 == \n gx */
    {comma_gx_width,    comma_gx_height,    comma_gx_bitmap   }, /* \002 == comma gx */
    {arrow_gx_width,    arrow_gx_height,    arrow_gx_bitmap   }, /* \003 == \-> gx */
    {equal_gx_width,    equal_gx_height,    equal_gx_bitmap   }, /* \004 == equal gx */
    {pi_gx_width,       pi_gx_height,       pi_gx_bitmap      }, /* \005 == pi gx */
    {0,                 0,                  0                 },
    {0,                 0,                  0                 },
    {0,                 0,                  0                 },
    {0,                 0,                  0                 },
    {0,                 0,                  0                 },
    {0,                 0,                  0                 },
    {0,                 0,                  0                 },
    {0,                 0,                  0                 },
    {0,                 0,                  0                 },
    {0,                 0,                  0                 },
    {0,                 0,                  0                 }, /* # 16 */
    {0,                 0,                  0                 },
    {0,                 0,                  0                 },
    {0,                 0,                  0                 },
    {0,                 0,                  0                 },
    {0,                 0,                  0                 },
    {0,                 0,                  0                 },
    {0,                 0,                  0                 },
    {0,                 0,                  0                 },
    {0,                 0,                  0                 },
    {0,                 0,                  0                 },
    {0,                 0,                  0                 },
    {0,                 0,                  0                 },
    {0,                 0,                  0                 },
    {0,                 0,                  0                 },
    {0,                 0,                  0                 },
    {blank_width,       blank_height,       blank_bitmap      }, /* # 32 */
    {0,                 0,                  0                 },
    {0,                 0,                  0                 },
    {hash_width,        hash_height,        hash_bitmap       },
    {0,                 0,                  0                 },
    {0,                 0,                  0                 },
    {0,                 0,                  0                 },
    {0,                 0,                  0                 },
    {lbrace_width,      lbrace_height,      lbrace_bitmap     },
    {rbrace_width,      rbrace_height,      rbrace_bitmap     },
    {0,                 0,                  0                 },
    {0,                 0,                  0                 },
    {comma_width,       comma_height,       comma_bitmap      },
    {0,                 0,                  0                 },
    {0,                 0,                  0                 },
    {slash_width,       slash_height,       slash_bitmap      },
    {0,                 0,                  0                 }, /* # 48 */
    {0,                 0,                  0                 },
    {two_width,         two_height,         two_bitmap        },
    {three_width,       three_height,       three_bitmap      },
    {0,                 0,                  0                 },
    {0,                 0,                  0                 },
    {0,                 0,                  0                 },
    {0,                 0,                  0                 },
    {0,                 0,                  0                 },
    {0,                 0,                  0                 },
    {small_colon_width, small_colon_height, small_colon_bitmap},
    {0,                 0,                  0                 },
    {0,                 0,                  0                 },
    {equal_width,       equal_height,       equal_bitmap      },
    {0,                 0,                  0                 },
    {0,                 0,                  0                 },
    {0,                 0,                  0                 }, /* # 64 */
    {A_width,           A_height,           A_bitmap          },
    {B_width,           B_height,           B_bitmap          },
    {C_width,           C_height,           C_bitmap          },
    {D_width,           D_height,           D_bitmap          },
    {E_width,           E_height,           E_bitmap          },
    {F_width,           F_height,           F_bitmap          },
    {G_width,           G_height,           G_bitmap          },
    {H_width,           H_height,           H_bitmap          },
    {I_width,           I_height,           I_bitmap          },
    {J_width,           J_height,           J_bitmap          },
    {K_width,           K_height,           K_bitmap          },
    {L_width,           L_height,           L_bitmap          },
    {M_width,           M_height,           M_bitmap          },
    {N_width,           N_height,           N_bitmap          },
    {O_width,           O_height,           O_bitmap          },
    {P_width,           P_height,           P_bitmap          }, /* # 80 */
    {Q_width,           Q_height,           Q_bitmap          },
    {R_width,           R_height,           R_bitmap          },
    {S_width,           S_height,           S_bitmap          },
    {T_width,           T_height,           T_bitmap          },
    {U_width,           U_height,           U_bitmap          },
    {V_width,           V_height,           V_bitmap          },
    {W_width,           W_height,           W_bitmap          },
    {X_width,           X_height,           X_bitmap          },
    {Y_width,           Y_height,           Y_bitmap          },
    {Z_width,           Z_height,           Z_bitmap          },
    {lbracket_width,    lbracket_height,    lbracket_bitmap   },
    {0,                 0,                  0                 },
    {rbracket_width,    rbracket_height,    rbracket_bitmap   },
    {0,                 0,                  0                 },
    {under_width,       under_height,       under_bitmap      },
    {0,                 0,                  0                 }, /* # 96 */
    {arrow_width,       arrow_height,       arrow_bitmap      }, /* a == left arrow   */
    {diff_width,        diff_height,        diff_bitmap       }, /* b == differential */
    {integral_width,    integral_height,    integral_bitmap   }, /* c == integral */
    {sigma_width,       sigma_height,       sigma_bitmap      }, /* d == sigma */
    {sqr_width,         sqr_height,         sqr_bitmap        }, /* e == sqr */
    {root_width,        root_height,        root_bitmap       }, /* f == root */
    {pow10_width,       pow10_height,       pow10_bitmap      }, /* g == pow10 */
    {exp_width,         exp_height,         exp_bitmap        }, /* h == exp */
    {prog_width,        prog_height,        prog_bitmap       }, /* i == << >> */
    {string_width,      string_height,      string_bitmap     }, /* j == " " */
    {nl_width,          nl_height,          nl_bitmap         }, /* k == New Line */
    {pi_width,          pi_height,          pi_bitmap         }, /* l == pi */
    {angle_width,       angle_height,       angle_bitmap      }, /* m == angle */
    {sqr_gx_width,      sqr_gx_height,      sqr_gx_bitmap     }, /* n == sqr gx */
    {root_gx_width,     root_gx_height,     root_gx_bitmap    }, /* o == root gx */
    {pow10_gx_width,    pow10_gx_height,    pow10_gx_bitmap   }, /* p == pow10 gx */
    {exp_gx_width,      exp_gx_height,      exp_gx_bitmap     }, /* q == exp gx */
    {parens_gx_width,   parens_gx_height,   parens_gx_bitmap  }, /* r == ( ) gx */
    {hash_gx_width,     hash_gx_height,     hash_gx_bitmap    }, /* s == # gx */
    {bracket_gx_width,  bracket_gx_height,  bracket_gx_bitmap }, /* t == [] gx */
    {under_gx_width,    under_gx_height,    under_gx_bitmap   }, /* u == _ gx */
    {prog_gx_width,     prog_gx_height,     prog_gx_bitmap    }, /* v == << >> gx */
    {quote_gx_width,    quote_gx_height,    quote_gx_bitmap   }, /* w == " " gx */
    {curly_gx_width,    curly_gx_height,    curly_gx_bitmap   }, /* x == {} gx */
    {colon_gx_width,    colon_gx_height,    colon_gx_bitmap   }, /* y == :: gx */
    {angle_gx_width,    angle_gx_height,    angle_gx_bitmap   }, /* z == angle gx */
    {lcurly_width,      lcurly_height,      lcurly_bitmap     },
    {0,                 0,                  0                 },
    {rcurly_width,      rcurly_height,      rcurly_bitmap     },
    {0,                 0,                  0                 },
    {0,                 0,                  0                 }
};

color_t colors_sx[ NB_COLORS ] = {
    {
     .name = "white",
     .r = 255,
     .g = 255,
     .b = 255,
     .mono_rgb = 255,
     .gray_rgb = 255,
     },
    {
     .name = "left",
     .r = 255,
     .g = 166,
     .b = 0,
     .mono_rgb = 255,
     .gray_rgb = 230,
     },
    {
     .name = "right",
     .r = 0,
     .g = 210,
     .b = 255,
     .mono_rgb = 255,
     .gray_rgb = 169,
     },
    {
     .name = "but_top",
     .r = 109,
     .g = 93,
     .b = 93,
     .mono_rgb = 0,
     .gray_rgb = 91,
     },
    {
     .name = "button",
     .r = 90,
     .g = 77,
     .b = 77,
     .mono_rgb = 0,
     .gray_rgb = 81,
     },
    {
     .name = "but_bot",
     .r = 76,
     .g = 65,
     .b = 65,
     .mono_rgb = 0,
     .gray_rgb = 69,
     },
    {
     .name = "lcd_col",
     .r = 202,
     .g = 221,
     .b = 92,
     .mono_rgb = 255,
     .gray_rgb = 205,
     },
    {
     .name = "pix_col",
     .r = 0,
     .g = 0,
     .b = 128,
     .mono_rgb = 0,
     .gray_rgb = 20,
     },
    {
     .name = "pad_top",
     .r = 109,
     .g = 78,
     .b = 78,
     .mono_rgb = 0,
     .gray_rgb = 88,
     },
    {
     .name = "pad",
     .r = 90,
     .g = 64,
     .b = 64,
     .mono_rgb = 0,
     .gray_rgb = 73,
     },
    {
     .name = "pad_bot",
     .r = 76,
     .g = 54,
     .b = 54,
     .mono_rgb = 0,
     .gray_rgb = 60,
     },
    {
     .name = "disp_pad_top",
     .r = 155,
     .g = 118,
     .b = 84,
     .mono_rgb = 0,
     .gray_rgb = 124,
     },
    {
     .name = "disp_pad",
     .r = 124,
     .g = 94,
     .b = 67,
     .mono_rgb = 0,
     .gray_rgb = 99,
     },
    {
     .name = "disp_pad_bot",
     .r = 100,
     .g = 75,
     .b = 53,
     .mono_rgb = 0,
     .gray_rgb = 79,
     },
    {
     .name = "logo",
     .r = 204,
     .g = 169,
     .b = 107,
     .mono_rgb = 255,
     .gray_rgb = 172,
     },
    {
     .name = "logo_back",
     .r = 64,
     .g = 64,
     .b = 64,
     .mono_rgb = 0,
     .gray_rgb = 65,
     },
    {
     .name = "label",
     .r = 202,
     .g = 184,
     .b = 144,
     .mono_rgb = 255,
     .gray_rgb = 185,
     },
    {
     .name = "frame",
     .r = 0,
     .g = 0,
     .b = 0,
     .mono_rgb = 255,
     .gray_rgb = 0,
     },
    {
     .name = "underlay",
     .r = 60,
     .g = 42,
     .b = 42,
     .mono_rgb = 0,
     .gray_rgb = 48,
     },
    {
     .name = "black",
     .r = 0,
     .g = 0,
     .b = 0,
     .mono_rgb = 0,
     .gray_rgb = 0,
     },
};

color_t colors_gx[ NB_COLORS ] = {
    {
     .name = "white",
     .r = 255,
     .g = 255,
     .b = 255,
     .mono_rgb = 255,
     .gray_rgb = 255,
     },
    {
     .name = "left",
     .r = 255,
     .g = 186,
     .b = 255,
     .mono_rgb = 255,
     .gray_rgb = 220,
     },
    {
     .name = "right",
     .r = 0,
     .g = 255,
     .b = 204,
     .mono_rgb = 255,
     .gray_rgb = 169,
     },
    {
     .name = "but_top",
     .r = 104,
     .g = 104,
     .b = 104,
     .mono_rgb = 0,
     .gray_rgb = 104,
     },
    {
     .name = "button",
     .r = 88,
     .g = 88,
     .b = 88,
     .mono_rgb = 0,
     .gray_rgb = 88,
     },
    {
     .name = "but_bot",
     .r = 74,
     .g = 74,
     .b = 74,
     .mono_rgb = 0,
     .gray_rgb = 74,
     },
    {
     .name = "lcd_col",
     .r = 202,
     .g = 221,
     .b = 92,
     .mono_rgb = 255,
     .gray_rgb = 205,
     },
    {
     .name = "pix_col",
     .r = 0,
     .g = 0,
     .b = 128,
     .mono_rgb = 0,
     .gray_rgb = 20,
     },
    {
     .name = "pad_top",
     .r = 88,
     .g = 88,
     .b = 88,
     .mono_rgb = 0,
     .gray_rgb = 88,
     },
    {
     .name = "pad",
     .r = 74,
     .g = 74,
     .b = 74,
     .mono_rgb = 0,
     .gray_rgb = 74,
     },
    {
     .name = "pad_bot",
     .r = 64,
     .g = 64,
     .b = 64,
     .mono_rgb = 0,
     .gray_rgb = 64,
     },
    {
     .name = "disp_pad_top",
     .r = 128,
     .g = 128,
     .b = 138,
     .mono_rgb = 0,
     .gray_rgb = 128,
     },
    {
     .name = "disp_pad",
     .r = 104,
     .g = 104,
     .b = 110,
     .mono_rgb = 0,
     .gray_rgb = 104,
     },
    {
     .name = "disp_pad_bot",
     .r = 84,
     .g = 84,
     .b = 90,
     .mono_rgb = 0,
     .gray_rgb = 84,
     },
    {
     .name = "logo",
     .r = 176,
     .g = 176,
     .b = 184,
     .mono_rgb = 255,
     .gray_rgb = 176,
     },
    {
     .name = "logo_back",
     .r = 104,
     .g = 104,
     .b = 110,
     .mono_rgb = 0,
     .gray_rgb = 104,
     },
    {
     .name = "label",
     .r = 240,
     .g = 240,
     .b = 240,
     .mono_rgb = 255,
     .gray_rgb = 240,
     },
    {
     .name = "frame",
     .r = 0,
     .g = 0,
     .b = 0,
     .mono_rgb = 255,
     .gray_rgb = 0,
     },
    {
     .name = "underlay",
     .r = 104,
     .g = 104,
     .b = 110,
     .mono_rgb = 0,
     .gray_rgb = 104,
     },
    {
     .name = "black",
     .r = 0,
     .g = 0,
     .b = 0,
     .mono_rgb = 0,
     .gray_rgb = 0,
     },
};

void ( *ui_disp_draw_nibble )( word_20 addr, word_4 val );
void ( *ui_menu_draw_nibble )( word_20 addr, word_4 val );
void ( *ui_get_event )( void );
void ( *ui_update_LCD )( void );
void ( *ui_refresh_LCD )( void );
void ( *ui_adjust_contrast )( void );
void ( *ui_draw_annunc )( void );

void ui_init_LCD( void ) { memset( lcd_nibbles_buffer, 0xf0, sizeof( lcd_nibbles_buffer ) ); }

int SmallTextWidth( const char* string, unsigned int length )
{
    int w = 0;
    for ( unsigned int i = 0; i < length; i++ ) {
        if ( small_font[ ( int )string[ i ] ].h != 0 )
            w += small_font[ ( int )string[ i ] ].w + 1;
        else {
            if ( config.verbose )
                fprintf( stderr, "Unknown small letter 0x00%x\n", ( int )string[ i ] );
            w += 5;
        }
    }

    return w;
}

void start_UI( int argc, char** argv )
{
    ui_init_LCD();

    switch ( config.frontend_type ) {
#if defined( HAS_X11 )
        case FRONTEND_X11:
        default:
            init_x11_ui( argc, argv );
            break;
#endif

#if defined( HAS_SDL )
        case FRONTEND_SDL:
#  if !defined( HAS_X11 )
        default:
#  endif
            init_sdl_ui( argc, argv );
            break;
#endif

        case FRONTEND_TEXT:
#if ( !defined( HAS_X11 ) && !defined( HAS_SDL ) )
        default:
#endif
            init_text_ui( argc, argv );
            break;
    }
}

void ui_stop( void )
{
    ui_init_LCD();

    switch ( config.frontend_type ) {
#if defined( HAS_X11 )
        case FRONTEND_X11:
        default:
            x11_ui_stop();
            break;
#endif

#if defined( HAS_SDL )
        case FRONTEND_SDL:
#  if !defined( HAS_X11 )
        default:
#  endif
            sdl_ui_stop();
            break;
#endif

        case FRONTEND_TEXT:
#if ( !defined( HAS_X11 ) && !defined( HAS_SDL ) )
        default:
#endif
            text_ui_stop();
            break;
    }
}
