#include "fonts.h"
#include "bitmaps_misc.h"

/**************/
/* SMALL FONT */
/**************/

unsigned char greater_than_bits[] = { 1, 2, 4, 8, 4, 2, 1 };

unsigned char greater_than_or_eq_bits[] = { 3, 12, 48, 12, 51, 12, 3 };

unsigned char lesser_than_bits[] = { 32, 16, 8, 4, 8, 16, 32 };

unsigned char lesser_than_or_eq_bits[] = { 48, 12, 3, 12, 51, 12, 48 };

unsigned char infinity_bits[] = { 0, 54, 73, 73, 73, 54, 0 };

unsigned char not_equal_bits[] = { 64, 32, 127, 8, 127, 2, 1 };

unsigned char ampersand_bits[] = { 28, 34, 34, 156, 82, 34, 220 };

unsigned char dot_bits[] = {
    0x0, 0x0, 0x0, 0x0, 0x0, 0x02, 0x02,
};

unsigned char single_quote_bits[] = { 0x02, 0x02, 0x02, 0x0, 0x0, 0x0, 0x0 };

unsigned char blank_bits[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

unsigned char hash_bits[] = { 0x00, 0x0a, 0x1f, 0x0a, 0x0a, 0x1f, 0x0a };

unsigned char lbrace_bits[] = { 0x04, 0x02, 0x01, 0x01, 0x01, 0x02, 0x04 };

unsigned char rbrace_bits[] = { 0x01, 0x02, 0x04, 0x04, 0x04, 0x02, 0x01 };

unsigned char comma_bits[] = { 0x00, 0x00, 0x00, 0x00, 0x06, 0x06, 0x03 };

unsigned char slash_bits[] = { 0x04, 0x04, 0x02, 0x02, 0x02, 0x01, 0x01 };

unsigned char zero_bits[] = {
    14, 17, 25, 21, 19, 17, 14,
};

unsigned char one_bits[] = {
    4, 6, 5, 4, 4, 4, 31,
};

unsigned char two_bits[] = { 0x0e, 0x11, 0x10, 0x08, 0x04, 0x02, 0x1f };

unsigned char three_bits[] = { 0x0e, 0x11, 0x10, 0x0c, 0x10, 0x11, 0x0e };

unsigned char four_bits[] = {
    8, 12, 10, 9, 31, 8, 8,
};

unsigned char five_bits[] = {
    31, 1, 1, 15, 16, 16, 15,
};

unsigned char six_bits[] = {
    14, 17, 1, 15, 17, 17, 14,
};

unsigned char seven_bits[] = {
    31, 16, 8, 4, 2, 1, 1,
};

unsigned char eight_bits[] = {
    14, 17, 17, 14, 17, 17, 14,
};

unsigned char nine_bits[] = {
    14, 17, 17, 30, 16, 17, 14,
};

unsigned char small_colon_bits[] = { 0x00, 0x03, 0x03, 0x00, 0x03, 0x03, 0x00 };

unsigned char d_bits[] = {
    16, 16, 30, 17, 17, 30, 0,
};

unsigned char e_bits[] = {
    0, 14, 17, 15, 1, 14, 0,
};

unsigned char i_bits[] = {
    4, 0, 6, 4, 4, 14, 0,
};

unsigned char p_bits[] = {
    0, 15, 17, 17, 15, 1, 1,
};

unsigned char r_bits[] = {
    0, 29, 3, 1, 1, 1, 0,
};

unsigned char s_bits[] = {
    0, 30, 1, 14, 16, 15, 0,
};

unsigned char t_bits[] = {
    2, 15, 2, 2, 2, 12, 0,
};

unsigned char v_bits[] = {
    0, 17, 17, 10, 10, 4, 0,
};

unsigned char w_bits[] = {
    0, 17, 17, 21, 21, 10, 0,
};

unsigned char y_bits[] = {
    0, 0, 17, 17, 30, 16, 15,
};

unsigned char A_bits[] = { 0x0e, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x11 };

unsigned char B_bits[] = { 0x0f, 0x11, 0x11, 0x0f, 0x11, 0x11, 0x0f };

unsigned char C_bits[] = { 0x0e, 0x11, 0x01, 0x01, 0x01, 0x11, 0x0e };

unsigned char D_bits[] = { 0x0f, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0f };

unsigned char E_bits[] = { 0x1f, 0x01, 0x01, 0x0f, 0x01, 0x01, 0x1f };

unsigned char F_bits[] = { 0x1f, 0x01, 0x01, 0x0f, 0x01, 0x01, 0x01 };

unsigned char G_bits[] = { 0x0e, 0x11, 0x01, 0x01, 0x19, 0x11, 0x0e };

unsigned char H_bits[] = { 0x11, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x11 };

unsigned char I_bits[] = { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 };

unsigned char J_bits[] = { 0x08, 0x08, 0x08, 0x08, 0x08, 0x09, 0x06 };

unsigned char K_bits[] = { 0x11, 0x09, 0x05, 0x03, 0x05, 0x09, 0x11 };

unsigned char L_bits[] = { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x0f };

unsigned char M_bits[] = { 0x11, 0x1b, 0x15, 0x11, 0x11, 0x11, 0x11 };

unsigned char N_bits[] = { 0x11, 0x11, 0x13, 0x15, 0x19, 0x11, 0x11 };

unsigned char O_bits[] = { 0x0e, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e };

unsigned char P_bits[] = { 0x0f, 0x11, 0x11, 0x0f, 0x01, 0x01, 0x01 };

unsigned char Q_bits[] = { 0x0e, 0x11, 0x11, 0x11, 0x15, 0x09, 0x16 };

unsigned char R_bits[] = { 0x0f, 0x11, 0x11, 0x0f, 0x05, 0x09, 0x11 };

unsigned char S_bits[] = { 0x0e, 0x11, 0x01, 0x0e, 0x10, 0x11, 0x0e };

unsigned char T_bits[] = { 0x1f, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04 };

unsigned char U_bits[] = { 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e };

unsigned char V_bits[] = { 0x11, 0x11, 0x11, 0x11, 0x0a, 0x0a, 0x04 };

unsigned char W_bits[] = { 0x11, 0x11, 0x11, 0x11, 0x15, 0x1b, 0x11 };

unsigned char X_bits[] = { 0x11, 0x11, 0x0a, 0x04, 0x0a, 0x11, 0x11 };

unsigned char Y_bits[] = { 0x11, 0x11, 0x0a, 0x04, 0x04, 0x04, 0x04 };

unsigned char Z_bits[] = { 0x1f, 0x10, 0x08, 0x04, 0x02, 0x01, 0x1f };

unsigned char lbracket_bits[] = { 0x07, 0x01, 0x01, 0x01, 0x01, 0x01, 0x07 };

unsigned char rbracket_bits[] = { 0x07, 0x04, 0x04, 0x04, 0x04, 0x04, 0x07 };

unsigned char arrow_bits[] = { 0x00, 0x08, 0x18, 0x3f, 0x18, 0x08, 0x00 };

unsigned char diff_bits[] = { 0x0e, 0x10, 0x10, 0x1e, 0x11, 0x11, 0x0e };

unsigned char integral_bits[] = { 0x0c, 0x12, 0x02, 0x04, 0x04, 0x08, 0x09, 0x06 };

unsigned char sigma_bits[] = { 0x3f, 0x21, 0x02, 0x04, 0x08, 0x04, 0x02, 0x21, 0x3f };

unsigned char sqr_bits[] = { 0x00, 0x03, 0x80, 0x04, 0x00, 0x04, 0x00, 0x02, 0x26, 0x01,
                             0x94, 0x07, 0x08, 0x00, 0x14, 0x00, 0x53, 0x00, 0x21, 0x00 };

unsigned char root_bits[] = { 0x26, 0x00, 0x00, 0x14, 0x00, 0x00, 0x08, 0xfe, 0x03, 0x14, 0x02, 0x02, 0x53,
                              0x02, 0x00, 0x21, 0x99, 0x00, 0x00, 0x91, 0x00, 0x10, 0x91, 0x00, 0xa0, 0x50,
                              0x00, 0xc0, 0x60, 0x00, 0x80, 0x20, 0x00, 0x00, 0x14, 0x00, 0x00, 0x0c, 0x00 };

unsigned char pow10_bits[] = { 0x00, 0x12, 0x00, 0x0c, 0x32, 0x04, 0x4b, 0x0a, 0x4a, 0x09, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x32, 0x00 };

unsigned char exp_bits[] = { 0x80, 0x04, 0x00, 0x03, 0x00, 0x01, 0x8c, 0x02, 0x52, 0x02, 0x09, 0x00, 0x07, 0x00, 0x21, 0x00, 0x1e, 0x00 };

unsigned char under_bits[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f };

unsigned char prog_bits[] = { 0x48, 0x12, 0x24, 0x24, 0x12, 0x48, 0x09, 0x90, 0x12, 0x48, 0x24, 0x24, 0x48, 0x12 };

unsigned char string_bits[] = { 0x85, 0x02, 0x85, 0x02, 0x85, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

unsigned char equal_bits[] = { 0x00, 0x1f, 0x00, 0x00, 0x1f, 0x00, 0x00 };

unsigned char nl_bits[] = { 0x00, 0x84, 0x86, 0xff, 0x06, 0x04, 0x00 };

unsigned char pi_bits[] = { 0x20, 0x1f, 0x12, 0x12, 0x12, 0x12, 0x12 };

unsigned char angle_bits[] = { 0x40, 0x20, 0x10, 0x28, 0x44, 0x42, 0xff };

unsigned char lcurly_bits[] = { 0x18, 0x04, 0x04, 0x02, 0x04, 0x04, 0x18 };

unsigned char rcurly_bits[] = { 0x03, 0x04, 0x04, 0x08, 0x04, 0x04, 0x03 };

unsigned char sqr_48gx_bits[] = { 0x00, 0x03, 0x80, 0x04, 0x00, 0x04, 0x00, 0x02, 0x00, 0x01, 0x80, 0x07, 0x00,
                                  0x00, 0x66, 0x00, 0x14, 0x00, 0x08, 0x00, 0x14, 0x00, 0x53, 0x00, 0x21, 0x00 };

unsigned char root_48gx_bits[] = { 0x66, 0x00, 0x00, 0x14, 0x00, 0x00, 0x08, 0x00, 0x00, 0x14, 0x00, 0x00, 0x53, 0xfe, 0x03,
                                   0x21, 0x02, 0x02, 0x00, 0x02, 0x00, 0x00, 0x99, 0x00, 0x00, 0x91, 0x00, 0x10, 0x91, 0x00,
                                   0xa0, 0x50, 0x00, 0xc0, 0x60, 0x00, 0x80, 0x20, 0x00, 0x00, 0x14, 0x00, 0x00, 0x0c, 0x00 };

unsigned char pow10_48gx_bits[] = { 0x00, 0x12, 0x00, 0x0c, 0x00, 0x04, 0x00, 0x0a, 0x00, 0x09, 0x32, 0x00,
                                    0x4b, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x32, 0x00 };

unsigned char exp_48gx_bits[] = { 0x00, 0xfb, 0x00, 0xf6, 0x00, 0xe6, 0x00, 0xf6, 0x80, 0xed, 0x18, 0xe0,
                                  0x36, 0xe0, 0x36, 0xe0, 0x1f, 0xe0, 0x03, 0xe0, 0x13, 0xe0, 0x0e, 0xe0 };
unsigned char parens_48gx_bits[] = { 0x0c, 0x00, 0x03, 0x06, 0x00, 0x06, 0x06, 0x00, 0x06, 0x03, 0x00, 0x0c,
                                     0x03, 0x00, 0x0c, 0x03, 0x00, 0x0c, 0x03, 0x00, 0x0c, 0x03, 0x00, 0x0c,
                                     0x03, 0x00, 0x0c, 0x06, 0x00, 0x06, 0x06, 0x00, 0x06, 0x0c, 0x00, 0x03 };

unsigned char hash_48gx_bits[] = { 0x00, 0x00, 0x48, 0x48, 0xfe, 0x24, 0x24, 0x7f, 0x12, 0x12, 0x00, 0x00 };

unsigned char bracket_48gx_bits[] = { 0x0f, 0x0f, 0x03, 0x0c, 0x03, 0x0c, 0x03, 0x0c, 0x03, 0x0c, 0x03, 0x0c,
                                      0x03, 0x0c, 0x03, 0x0c, 0x03, 0x0c, 0x03, 0x0c, 0x03, 0x0c, 0x0f, 0x0f };

unsigned char under_48gx_bits[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x03, 0xff, 0x03 };

unsigned char prog_48gx_bits[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0xc3, 0x18,
                                   0x8c, 0x81, 0x31, 0xc6, 0x00, 0x63, 0x63, 0x00, 0xc6, 0xc6, 0x00, 0x63,
                                   0x8c, 0x81, 0x31, 0x18, 0xc3, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

unsigned char quote_48gx_bits[] = { 0x05, 0x0a, 0x05, 0x0a, 0x05, 0x0a, 0x05, 0x0a, 0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

unsigned char curly_48gx_bits[] = { 0x0c, 0x0c, 0x06, 0x18, 0x06, 0x18, 0x06, 0x18, 0x06, 0x18, 0x03, 0x30,
                                    0x06, 0x18, 0x06, 0x18, 0x06, 0x18, 0x06, 0x18, 0x06, 0x18, 0x0c, 0x0c };

unsigned char colon_48gx_bits[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0xc3, 0xc3, 0x00, 0x00, 0xc3, 0xc3, 0x00 };

unsigned char angle_48gx_bits[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x01, 0xc0, 0x00, 0xe0, 0x01,
                                    0xb0, 0x03, 0x18, 0x03, 0x0c, 0x03, 0x06, 0x03, 0xff, 0x0f, 0xff, 0x0f };

unsigned char pi_48gx_bits[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xfe, 0x03, 0xff, 0x01,
                                 0xcc, 0x00, 0xcc, 0x00, 0xcc, 0x00, 0xcc, 0x00, 0xcc, 0x00, 0xcc, 0x00 };

unsigned char nl_48gx_bits[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x03, 0xf0, 0x00, 0x03,
                                 0xfc, 0x00, 0x03, 0xff, 0xff, 0x03, 0xff, 0xff, 0x03, 0xfc, 0x00, 0x00,
                                 0xf0, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

unsigned char comma_48gx_bits[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x07, 0x07, 0x04, 0x04, 0x02 };

unsigned char arrow_48gx_bits[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x3c, 0x00,
                                    0x00, 0xfc, 0x00, 0xff, 0xff, 0x03, 0xff, 0xff, 0x03, 0x00, 0xfc, 0x00,
                                    0x00, 0x3c, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

unsigned char equal_48gx_bits[] = { 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00 };

letter_t small_font[ 128 ] = {
    {0,                        0,                         0                      },
    {nl_48gx_width,            nl_48gx_height,            nl_48gx_bits           }, /* \x01 == \n gx */
    {comma_48gx_width,         comma_48gx_height,         comma_48gx_bits        }, /* \x02 == comma gx */
    {arrow_48gx_width,         arrow_48gx_height,         arrow_48gx_bits        }, /* \x03 == \-> gx */
    {equal_48gx_width,         equal_48gx_height,         equal_48gx_bits        }, /* \x04 == equal gx */
    {pi_48gx_width,            pi_48gx_height,            pi_48gx_bits           }, /* \x05 == pi gx */
    {arrow_width,              arrow_height,              arrow_bits             }, /* \x06 == left arrow   */
    {diff_width,               diff_height,               diff_bits              }, /* \x07 == differential */
    {integral_width,           integral_height,           integral_bits          }, /* \x08 == integral */
    {sigma_width,              sigma_height,              sigma_bits             }, /* \x09 == sigma */
    {sqr_width,                sqr_height,                sqr_bits               }, /* \x0a == sqr */
    {root_width,               root_height,               root_bits              }, /* \x0b == root */
    {pow10_width,              pow10_height,              pow10_bits             }, /* \x0c == pow10 */
    {exp_width,                exp_height,                exp_bits               }, /* \x0d == exp */
    {prog_width,               prog_height,               prog_bits              }, /* \x0e == << >> */
    {string_width,             string_height,             string_bits            }, /* \x0f == " " */
    {nl_width,                 nl_height,                 nl_bits                }, /* \x10 == New Line # 16 */
    {pi_width,                 pi_height,                 pi_bits                }, /* \x11 == pi */
    {angle_width,              angle_height,              angle_bits             }, /* \x12 == angle */
    {sqr_48gx_width,           sqr_48gx_height,           sqr_48gx_bits          }, /* \x13 == sqr gx */
    {root_48gx_width,          root_48gx_height,          root_48gx_bits         }, /* \x14 == root gx */
    {pow10_48gx_width,         pow10_48gx_height,         pow10_48gx_bits        }, /* \x15 == pow10 gx */
    {exp_48gx_width,           exp_48gx_height,           exp_48gx_bits          }, /* \x16 == exp gx */
    {parens_48gx_width,        parens_48gx_height,        parens_48gx_bits       }, /* \x17 == ( ) gx */
    {hash_48gx_width,          hash_48gx_height,          hash_48gx_bits         }, /* \x18 == # gx */
    {bracket_48gx_width,       bracket_48gx_height,       bracket_48gx_bits      }, /* \x19 == [] gx */
    {under_48gx_width,         under_48gx_height,         under_48gx_bits        }, /* \x1a == _ gx */
    {prog_48gx_width,          prog_48gx_height,          prog_48gx_bits         }, /* \x1b == << >> gx */
    {quote_48gx_width,         quote_48gx_height,         quote_48gx_bits        }, /* \x1c == " " gx */
    {curly_48gx_width,         curly_48gx_height,         curly_48gx_bits        }, /* \x1d == {} gx */
    {colon_48gx_width,         colon_48gx_height,         colon_48gx_bits        }, /* \x1e == :: gx */
    {angle_48gx_width,         angle_48gx_height,         angle_48gx_bits        }, /* \x1f == angle gx */
    {blank_width,              blank_height,              blank_bits             }, /* # 0x20 */
    {0,                        0,                         0                      },
    {0,                        0,                         0                      },
    {hash_width,               hash_height,               hash_bits              }, /* \x23 == # */
    {0,                        0,                         0                      },
    {0,                        0,                         0                      },
    {ampersand_width,          ampersand_height,          ampersand_bits         }, /* \x26 == & */
    {single_quote_width,       single_quote_height,       single_quote_bits      }, /* \x27 == ' */
    {lbrace_width,             lbrace_height,             lbrace_bits            }, /* \x28 == ( */
    {rbrace_width,             rbrace_height,             rbrace_bits            }, /* \x29 == ) */
    {0,                        0,                         0                      },
    {0,                        0,                         0                      },
    {comma_width,              comma_height,              comma_bits             }, /* \x2c == , */
    {0,                        0,                         0                      },
    {dot_width,                dot_height,                dot_bits               },
    {slash_width,              slash_height,              slash_bits             },
    {zero_width,               zero_height,               zero_bits              }, /* # 0x30 */
    {one_width,                one_height,                one_bits               },
    {two_width,                two_height,                two_bits               },
    {three_width,              three_height,              three_bits             },
    {four_width,               four_height,               four_bits              },
    {five_width,               five_height,               five_bits              },
    {six_width,                six_height,                six_bits               },
    {seven_width,              seven_height,              seven_bits             },
    {eight_width,              eight_height,              eight_bits             },
    {nine_width,               nine_height,               nine_bits              },
    {small_colon_width,        small_colon_height,        small_colon_bits       }, /* : */
    {lesser_than_or_eq_width,  lesser_than_or_eq_height,  lesser_than_or_eq_bits }, /* \x3b == ≤ */
    {lesser_than_width,        lesser_than_height,        lesser_than_bits       }, /* \x3c == < */
    {equal_width,              equal_height,              equal_bits             },
    {greater_than_width,       greater_than_height,       greater_than_bits      }, /* \x3e == > */
    {greater_than_or_eq_width, greater_than_or_eq_height, greater_than_or_eq_bits}, /* \x3f == ≥ */
    {0,                        0,                         0                      }, /* # 0x40 */
    {A_width,                  A_height,                  A_bits                 },
    {B_width,                  B_height,                  B_bits                 },
    {C_width,                  C_height,                  C_bits                 },
    {D_width,                  D_height,                  D_bits                 },
    {E_width,                  E_height,                  E_bits                 },
    {F_width,                  F_height,                  F_bits                 },
    {G_width,                  G_height,                  G_bits                 },
    {H_width,                  H_height,                  H_bits                 },
    {I_width,                  I_height,                  I_bits                 },
    {J_width,                  J_height,                  J_bits                 },
    {K_width,                  K_height,                  K_bits                 },
    {L_width,                  L_height,                  L_bits                 },
    {M_width,                  M_height,                  M_bits                 },
    {N_width,                  N_height,                  N_bits                 },
    {O_width,                  O_height,                  O_bits                 },
    {P_width,                  P_height,                  P_bits                 }, /* # 0x50 */
    {Q_width,                  Q_height,                  Q_bits                 },
    {R_width,                  R_height,                  R_bits                 },
    {S_width,                  S_height,                  S_bits                 },
    {T_width,                  T_height,                  T_bits                 },
    {U_width,                  U_height,                  U_bits                 },
    {V_width,                  V_height,                  V_bits                 },
    {W_width,                  W_height,                  W_bits                 },
    {X_width,                  X_height,                  X_bits                 },
    {Y_width,                  Y_height,                  Y_bits                 },
    {Z_width,                  Z_height,                  Z_bits                 },
    {lbracket_width,           lbracket_height,           lbracket_bits          },
    {0,                        0,                         0                      },
    {rbracket_width,           rbracket_height,           rbracket_bits          },
    {0,                        0,                         0                      },
    {under_width,              under_height,              under_bits             },
    {not_equal_width,          not_equal_height,          not_equal_bits         }, /* \x60 == ≠ */
    {0,                        0,                         0                      }, /* a */
    {0,                        0,                         0                      }, /* b */
    {0,                        0,                         0                      }, /* c */
    {d_width,                  d_height,                  d_bits                 },
    {e_width,                  e_height,                  e_bits                 },
    {0,                        0,                         0                      }, /* f */
    {0,                        0,                         0                      }, /* g */
    {0,                        0,                         0                      }, /* h */
    {i_width,                  i_height,                  i_bits                 },
    {0,                        0,                         0                      }, /* j */
    {0,                        0,                         0                      }, /* k */
    {0,                        0,                         0                      }, /* l */
    {0,                        0,                         0                      }, /* m */
    {0,                        0,                         0                      }, /* n */
    {0,                        0,                         0                      }, /* o */
    {p_width,                  p_height,                  p_bits                 },
    {0,                        0,                         0                      }, /* q */
    {r_width,                  r_height,                  r_bits                 },
    {s_width,                  s_height,                  s_bits                 },
    {t_width,                  t_height,                  t_bits                 },
    {0,                        0,                         0                      }, /* u */
    {v_width,                  v_height,                  v_bits                 },
    {w_width,                  w_height,                  w_bits                 },
    {0,                        0,                         0                      }, /* x */
    {y_width,                  y_height,                  y_bits                 },
    {0,                        0,                         0                      }, /* z */
    {lcurly_width,             lcurly_height,             lcurly_bits            },
    {0,                        0,                         0                      }, /* | */
    {rcurly_width,             rcurly_height,             rcurly_bits            },
    {infinity_width,           infinity_height,           infinity_bits          }, /* \x7e == ∞ */
    {0,                        0,                         0                      }, /* # 0x7f */
};

/************/
/* BIG FONT */
/************/

unsigned char big_font_dot_bits[] = {
    0, 0, 0, 0, 0, 0, 0, 24, 24, 0, 0, 0, 0,
};

unsigned char big_font_0_bits[] = {
    24, 60, 102, 102, 195, 195, 195, 195, 195, 102, 102, 60, 24,
};

unsigned char big_font_1_bits[] = {
    24, 28, 30, 27, 24, 24, 24, 24, 24, 24, 24, 24, 255,
};

unsigned char big_font_2_bits[] = {
    60, 102, 195, 195, 192, 192, 96, 56, 12, 6, 3, 3, 255,
};

unsigned char big_font_3_bits[] = {
    60, 102, 195, 195, 192, 96, 56, 96, 192, 195, 195, 102, 60,
};

unsigned char big_font_4_bits[] = {
    96, 112, 120, 108, 102, 99, 99, 255, 96, 96, 96, 96, 96,
};

unsigned char big_font_5_bits[] = {
    255, 3, 3, 3, 3, 59, 103, 192, 192, 192, 195, 102, 60,
};

unsigned char big_font_6_bits[] = {
    60, 102, 67, 3, 3, 59, 103, 195, 195, 195, 195, 102, 60,
};

unsigned char big_font_7_bits[] = {
    255, 192, 192, 96, 96, 48, 48, 24, 24, 12, 12, 6, 6,
};

unsigned char big_font_8_bits[] = {
    60, 102, 195, 195, 195, 102, 60, 102, 195, 195, 195, 102, 60,
};

unsigned char big_font_9_bits[] = {
    60, 102, 195, 195, 195, 195, 230, 220, 192, 192, 194, 102, 60,
};

unsigned char big_font_A_bits[] = {
    30, 51, 51, 51, 51, 51, 63, 51, 51, 51,
};

unsigned char big_font_B_bits[] = {
    0b1111, 0b11011, 0b110011, 0b110011, 0b1111, 0b110011, 0b110011, 0b110011, 0b11011, 0b1111,
};

unsigned char big_font_C_bits[] = {
    30, 51, 51, 3, 3, 3, 3, 51, 51, 30,
};

unsigned char big_font_D_bits[] = {
    15, 27, 51, 51, 51, 51, 51, 51, 27, 15,
};

unsigned char big_font_E_bits[] = {
    63, 3, 3, 3, 31, 3, 3, 3, 3, 63,
};

unsigned char big_font_F_bits[] = {
    63, 3, 3, 3, 31, 3, 3, 3, 3, 3,
};

unsigned char big_font_G_bits[] = {
    30, 51, 51, 3, 3, 59, 51, 51, 51, 30,
};

unsigned char big_font_H_bits[] = {
    51, 51, 51, 51, 63, 51, 51, 51, 51, 51,
};

unsigned char big_font_I_bits[] = {
    63, 12, 12, 12, 12, 12, 12, 12, 12, 63,
};

unsigned char big_font_J_bits[] = {
    96, 96, 96, 96, 96, 96, 96, 51, 51, 30,
};

unsigned char big_font_K_bits[] = {
    0b110011, 0b110011, 0b110011, 0b11011, 0b1111, 0b11011, 0b110011, 0b110011, 0b11011, 0b110011,
};

unsigned char big_font_L_bits[] = {
    3, 3, 3, 3, 3, 3, 3, 3, 3, 63,
};

unsigned char big_font_M_bits[] = {
    33, 51, 51, 63, 63, 51, 51, 51, 51, 51,
};

unsigned char big_font_N_bits[] = {
    51, 51, 55, 55, 55, 59, 59, 59, 51, 51,
};

unsigned char big_font_O_bits[] = {
    30, 51, 51, 51, 51, 51, 51, 51, 51, 30,
};

unsigned char big_font_P_bits[] = {
    31, 51, 51, 51, 51, 31, 3, 3, 3, 3,
};

unsigned char big_font_Q_bits[] = {
    30, 51, 51, 51, 51, 51, 51, 55, 22, 60,
};

unsigned char big_font_R_bits[] = {
    31, 51, 51, 51, 31, 27, 51, 51, 51, 51,
};

unsigned char big_font_S_bits[] = {
    30, 51, 51, 6, 12, 12, 24, 51, 51, 30,
};

unsigned char big_font_T_bits[] = {
    63, 12, 12, 12, 12, 12, 12, 12, 12, 12,
};

unsigned char big_font_U_bits[] = {
    51, 51, 51, 51, 51, 51, 51, 51, 51, 30,
};

unsigned char big_font_V_bits[] = {
    51, 51, 51, 51, 51, 51, 51, 30, 30, 12,
};

unsigned char big_font_W_bits[] = {
    51, 51, 51, 51, 51, 63, 63, 51, 51, 33,
};

unsigned char big_font_X_bits[] = {
    51, 51, 30, 30, 12, 12, 30, 30, 51, 51,
};

unsigned char big_font_Y_bits[] = {
    51, 51, 30, 30, 12, 12, 12, 12, 12, 12,
};

letter_t big_font[ 128 ] = {
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                }, /* # 16 */
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                }, /* # 32 */
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {big_font_dot_width, big_font_dot_height, big_font_dot_bits}, /* # 46 */
    {0,                  0,                   0                },
    {big_font_0_width,   big_font_0_height,   big_font_0_bits  }, /* # 48 */
    {big_font_1_width,   big_font_1_height,   big_font_1_bits  },
    {big_font_2_width,   big_font_2_height,   big_font_2_bits  },
    {big_font_3_width,   big_font_3_height,   big_font_3_bits  },
    {big_font_4_width,   big_font_4_height,   big_font_4_bits  },
    {big_font_5_width,   big_font_5_height,   big_font_5_bits  },
    {big_font_6_width,   big_font_6_height,   big_font_6_bits  },
    {big_font_7_width,   big_font_7_height,   big_font_7_bits  },
    {big_font_8_width,   big_font_8_height,   big_font_8_bits  },
    {big_font_9_width,   big_font_9_height,   big_font_9_bits  },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                }, /* # 64 */
    {big_font_A_width,   big_font_A_height,   big_font_A_bits  },
    {big_font_B_width,   big_font_B_height,   big_font_B_bits  },
    {big_font_C_width,   big_font_C_height,   big_font_C_bits  },
    {big_font_D_width,   big_font_D_height,   big_font_D_bits  },
    {big_font_E_width,   big_font_E_height,   big_font_E_bits  },
    {big_font_F_width,   big_font_F_height,   big_font_F_bits  },
    {big_font_G_width,   big_font_G_height,   big_font_G_bits  },
    {big_font_H_width,   big_font_H_height,   big_font_H_bits  },
    {big_font_I_width,   big_font_I_height,   big_font_I_bits  },
    {big_font_J_width,   big_font_J_height,   big_font_J_bits  },
    {big_font_K_width,   big_font_K_height,   big_font_K_bits  },
    {big_font_L_width,   big_font_L_height,   big_font_L_bits  },
    {big_font_M_width,   big_font_M_height,   big_font_M_bits  },
    {big_font_N_width,   big_font_N_height,   big_font_N_bits  },
    {big_font_O_width,   big_font_O_height,   big_font_O_bits  },
    {big_font_P_width,   big_font_P_height,   big_font_P_bits  }, /* # 80 */
    {big_font_Q_width,   big_font_Q_height,   big_font_Q_bits  },
    {big_font_R_width,   big_font_R_height,   big_font_R_bits  },
    {big_font_S_width,   big_font_S_height,   big_font_S_bits  },
    {big_font_T_width,   big_font_T_height,   big_font_T_bits  },
    {big_font_U_width,   big_font_U_height,   big_font_U_bits  },
    {big_font_V_width,   big_font_V_height,   big_font_V_bits  },
    {big_font_W_width,   big_font_W_height,   big_font_W_bits  },
    {big_font_X_width,   big_font_X_height,   big_font_X_bits  },
    {big_font_Y_width,   big_font_Y_height,   big_font_Y_bits  },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                }, /* # 96 */
    {0,                  0,                   0                }, /* a */
    {0,                  0,                   0                }, /* b */
    {0,                  0,                   0                }, /* c */
    {0,                  0,                   0                }, /* d */
    {0,                  0,                   0                }, /* e */
    {0,                  0,                   0                }, /* f */
    {0,                  0,                   0                }, /* g */
    {0,                  0,                   0                }, /* h */
    {0,                  0,                   0                }, /* i */
    {0,                  0,                   0                }, /* j */
    {0,                  0,                   0                }, /* k */
    {0,                  0,                   0                }, /* l */
    {0,                  0,                   0                }, /* m */
    {0,                  0,                   0                }, /* n */
    {0,                  0,                   0                }, /* o */
    {0,                  0,                   0                }, /* p */
    {0,                  0,                   0                }, /* q */
    {0,                  0,                   0                }, /* r */
    {0,                  0,                   0                }, /* s */
    {0,                  0,                   0                }, /* t */
    {0,                  0,                   0                }, /* u */
    {0,                  0,                   0                }, /* v */
    {0,                  0,                   0                }, /* w */
    {0,                  0,                   0                }, /* x */
    {0,                  0,                   0                }, /* y */
    {0,                  0,                   0                }, /* z */
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                },
    {0,                  0,                   0                }
};
