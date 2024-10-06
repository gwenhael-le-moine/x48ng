#ifndef _UI_SMALL_FONT_H
#define _UI_SMALL_FONT_H 1

#define blank_width 4
#define blank_height 7
static unsigned char blank_bits[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

#define hash_width 5
#define hash_height 7
static unsigned char hash_bits[] = { 0x00, 0x0a, 0x1f, 0x0a, 0x0a, 0x1f, 0x0a };

#define lbrace_width 3
#define lbrace_height 7
static unsigned char lbrace_bits[] = { 0x04, 0x02, 0x01, 0x01, 0x01, 0x02, 0x04 };

#define rbrace_width 3
#define rbrace_height 7
static unsigned char rbrace_bits[] = { 0x01, 0x02, 0x04, 0x04, 0x04, 0x02, 0x01 };

#define comma_width 3
#define comma_height 7
static unsigned char comma_bits[] = { 0x00, 0x00, 0x00, 0x00, 0x06, 0x06, 0x03 };

#define slash_width 3
#define slash_height 7
static unsigned char slash_bits[] = { 0x04, 0x04, 0x02, 0x02, 0x02, 0x01, 0x01 };

#define zero_width 5
#define zero_height 7
static unsigned char zero_bits[] = {
    14, 17, 25, 21, 19, 17, 14,
};

#define one_width 5
#define one_height 7
static unsigned char one_bits[] = {
    4, 6, 5, 4, 4, 4, 31,
};

#define two_width 5
#define two_height 7
static unsigned char two_bits[] = { 0x0e, 0x11, 0x10, 0x08, 0x04, 0x02, 0x1f };

#define three_width 5
#define three_height 7
static unsigned char three_bits[] = { 0x0e, 0x11, 0x10, 0x0c, 0x10, 0x11, 0x0e };

#define four_width 5
#define four_height 7
static unsigned char four_bits[] = {
    8, 12, 10, 9, 31, 8, 8,
};

#define five_width 5
#define five_height 7
static unsigned char five_bits[] = {
    31, 1, 1, 15, 16, 16, 15,
};

#define six_width 5
#define six_height 7
static unsigned char six_bits[] = {
    14, 17, 1, 15, 17, 17, 14,
};

#define seven_width 5
#define seven_height 7
static unsigned char seven_bits[] = {
    31, 16, 8, 4, 2, 1, 1,
};

#define eight_width 5
#define eight_height 7
static unsigned char eight_bits[] = {
    14, 17, 17, 14, 17, 17, 14,
};

#define nine_width 5
#define nine_height 7
static unsigned char nine_bits[] = {
    14, 17, 17, 30, 16, 17, 14,
};

#define small_colon_width 2
#define small_colon_height 7
static unsigned char small_colon_bits[] = { 0x00, 0x03, 0x03, 0x00, 0x03, 0x03, 0x00 };

#define d_width 5
#define d_height 7
static unsigned char d_bits[] = {
    16, 16, 30, 17, 17, 30, 0,
};

#define e_width 5
#define e_height 7
static unsigned char e_bits[] = {
    0, 14, 17, 15, 1, 14, 0,
};

#define i_width 5
#define i_height 7
static unsigned char i_bits[] = {
    4, 0, 6, 4, 4, 14, 0,
};

#define p_width 5
#define p_height 7
static unsigned char p_bits[] = {
    0, 15, 17, 17, 15, 1, 1,
};

#define r_width 5
#define r_height 7
static unsigned char r_bits[] = {
    0, 29, 3, 1, 1, 1, 0,
};

#define s_width 5
#define s_height 7
static unsigned char s_bits[] = {
    0, 30, 1, 14, 16, 15, 0,
};

#define t_width 5
#define t_height 7
static unsigned char t_bits[] = {
    2, 15, 2, 2, 2, 12, 0,
};

#define v_width 5
#define v_height 7
static unsigned char v_bits[] = {
    0, 17, 17, 10, 10, 4, 0,
};

#define w_width 5
#define w_height 7
static unsigned char w_bits[] = {
    0, 17, 17, 21, 21, 10, 0,
};

#define y_width 5
#define y_height 7
static unsigned char y_bits[] = {
    0, 0, 17, 17, 30, 16, 15,
};

#define A_width 5
#define A_height 7
static unsigned char A_bits[] = { 0x0e, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x11 };

#define B_width 5
#define B_height 7
static unsigned char B_bits[] = { 0x0f, 0x11, 0x11, 0x0f, 0x11, 0x11, 0x0f };

#define C_width 5
#define C_height 7
static unsigned char C_bits[] = { 0x0e, 0x11, 0x01, 0x01, 0x01, 0x11, 0x0e };

#define D_width 5
#define D_height 7
static unsigned char D_bits[] = { 0x0f, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0f };

#define E_width 5
#define E_height 7
static unsigned char E_bits[] = { 0x1f, 0x01, 0x01, 0x0f, 0x01, 0x01, 0x1f };

#define F_width 5
#define F_height 7
static unsigned char F_bits[] = { 0x1f, 0x01, 0x01, 0x0f, 0x01, 0x01, 0x01 };

#define G_width 5
#define G_height 7
static unsigned char G_bits[] = { 0x0e, 0x11, 0x01, 0x01, 0x19, 0x11, 0x0e };

#define H_width 5
#define H_height 7
static unsigned char H_bits[] = { 0x11, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x11 };

#define I_width 1
#define I_height 7
static unsigned char I_bits[] = { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 };

#define J_width 4
#define J_height 7
static unsigned char J_bits[] = { 0x08, 0x08, 0x08, 0x08, 0x08, 0x09, 0x06 };

#define K_width 5
#define K_height 7
static unsigned char K_bits[] = { 0x11, 0x09, 0x05, 0x03, 0x05, 0x09, 0x11 };

#define L_width 4
#define L_height 7
static unsigned char L_bits[] = { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x0f };

#define M_width 5
#define M_height 7
static unsigned char M_bits[] = { 0x11, 0x1b, 0x15, 0x11, 0x11, 0x11, 0x11 };

#define N_width 5
#define N_height 7
static unsigned char N_bits[] = { 0x11, 0x11, 0x13, 0x15, 0x19, 0x11, 0x11 };

#define O_width 5
#define O_height 7
static unsigned char O_bits[] = { 0x0e, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e };

#define P_width 5
#define P_height 7
static unsigned char P_bits[] = { 0x0f, 0x11, 0x11, 0x0f, 0x01, 0x01, 0x01 };

#define Q_width 5
#define Q_height 7
static unsigned char Q_bits[] = { 0x0e, 0x11, 0x11, 0x11, 0x15, 0x09, 0x16 };

#define R_width 5
#define R_height 7
static unsigned char R_bits[] = { 0x0f, 0x11, 0x11, 0x0f, 0x05, 0x09, 0x11 };

#define S_width 5
#define S_height 7
static unsigned char S_bits[] = { 0x0e, 0x11, 0x01, 0x0e, 0x10, 0x11, 0x0e };

#define T_width 5
#define T_height 7
static unsigned char T_bits[] = { 0x1f, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04 };

#define U_width 5
#define U_height 7
static unsigned char U_bits[] = { 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e };

#define V_width 5
#define V_height 7
static unsigned char V_bits[] = { 0x11, 0x11, 0x11, 0x11, 0x0a, 0x0a, 0x04 };

#define W_width 5
#define W_height 7
static unsigned char W_bits[] = { 0x11, 0x11, 0x11, 0x11, 0x15, 0x1b, 0x11 };

#define X_width 5
#define X_height 7
static unsigned char X_bits[] = { 0x11, 0x11, 0x0a, 0x04, 0x0a, 0x11, 0x11 };

#define Y_width 5
#define Y_height 7
static unsigned char Y_bits[] = { 0x11, 0x11, 0x0a, 0x04, 0x04, 0x04, 0x04 };

#define Z_width 5
#define Z_height 7
static unsigned char Z_bits[] = { 0x1f, 0x10, 0x08, 0x04, 0x02, 0x01, 0x1f };

#define lbracket_width 3
#define lbracket_height 7
static unsigned char lbracket_bits[] = { 0x07, 0x01, 0x01, 0x01, 0x01, 0x01, 0x07 };

#define rbracket_width 3
#define rbracket_height 7
static unsigned char rbracket_bits[] = { 0x07, 0x04, 0x04, 0x04, 0x04, 0x04, 0x07 };

#define arrow_width 7
#define arrow_height 7
static unsigned char arrow_bits[] = { 0x00, 0x08, 0x18, 0x3f, 0x18, 0x08, 0x00 };

#define diff_width 5
#define diff_height 7
static unsigned char diff_bits[] = { 0x0e, 0x10, 0x10, 0x1e, 0x11, 0x11, 0x0e };

#define integral_width 5
#define integral_height 8
static unsigned char integral_bits[] = { 0x0c, 0x12, 0x02, 0x04, 0x04, 0x08, 0x09, 0x06 };

#define sigma_width 6
#define sigma_height 9
static unsigned char sigma_bits[] = { 0x3f, 0x21, 0x02, 0x04, 0x08, 0x04, 0x02, 0x21, 0x3f };

#define sqr_width 11
#define sqr_height 10
static unsigned char sqr_bits[] = { 0x00, 0x03, 0x80, 0x04, 0x00, 0x04, 0x00, 0x02, 0x26, 0x01,
                                    0x94, 0x07, 0x08, 0x00, 0x14, 0x00, 0x53, 0x00, 0x21, 0x00 };

#define root_width 18
#define root_height 13
static unsigned char root_bits[] = { 0x26, 0x00, 0x00, 0x14, 0x00, 0x00, 0x08, 0xfe, 0x03, 0x14, 0x02, 0x02, 0x53,
                                     0x02, 0x00, 0x21, 0x99, 0x00, 0x00, 0x91, 0x00, 0x10, 0x91, 0x00, 0xa0, 0x50,
                                     0x00, 0xc0, 0x60, 0x00, 0x80, 0x20, 0x00, 0x00, 0x14, 0x00, 0x00, 0x0c, 0x00 };

#define pow10_width 13
#define pow10_height 9
static unsigned char pow10_bits[] = { 0x00, 0x12, 0x00, 0x0c, 0x32, 0x04, 0x4b, 0x0a, 0x4a,
                                      0x09, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x32, 0x00 };

#define exp_width 11
#define exp_height 9
static unsigned char exp_bits[] = { 0x80, 0x04, 0x00, 0x03, 0x00, 0x01, 0x8c, 0x02, 0x52,
                                    0x02, 0x09, 0x00, 0x07, 0x00, 0x21, 0x00, 0x1e, 0x00 };

#define under_width 6
#define under_height 7
static unsigned char under_bits[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f };

#define prog_width 16
#define prog_height 7
static unsigned char prog_bits[] = { 0x48, 0x12, 0x24, 0x24, 0x12, 0x48, 0x09, 0x90, 0x12, 0x48, 0x24, 0x24, 0x48, 0x12 };

#define string_width 10
#define string_height 7
static unsigned char string_bits[] = { 0x85, 0x02, 0x85, 0x02, 0x85, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

#define equal_width 5
#define equal_height 7
static unsigned char equal_bits[] = { 0x00, 0x1f, 0x00, 0x00, 0x1f, 0x00, 0x00 };

#define nl_width 8
#define nl_height 7
static unsigned char nl_bits[] = { 0x00, 0x84, 0x86, 0xff, 0x06, 0x04, 0x00 };

#define pi_width 6
#define pi_height 7
static unsigned char pi_bits[] = { 0x20, 0x1f, 0x12, 0x12, 0x12, 0x12, 0x12 };

#define angle_width 8
#define angle_height 7
static unsigned char angle_bits[] = { 0x40, 0x20, 0x10, 0x28, 0x44, 0x42, 0xff };

#define lcurly_width 5
#define lcurly_height 7
static unsigned char lcurly_bits[] = { 0x18, 0x04, 0x04, 0x02, 0x04, 0x04, 0x18 };

#define rcurly_width 5
#define rcurly_height 7
static unsigned char rcurly_bits[] = { 0x03, 0x04, 0x04, 0x08, 0x04, 0x04, 0x03 };

#define sqr_gx_width 11
#define sqr_gx_height 13
static unsigned char sqr_gx_bits[] = { 0x00, 0x03, 0x80, 0x04, 0x00, 0x04, 0x00, 0x02, 0x00, 0x01, 0x80, 0x07, 0x00,
                                       0x00, 0x66, 0x00, 0x14, 0x00, 0x08, 0x00, 0x14, 0x00, 0x53, 0x00, 0x21, 0x00 };

#define root_gx_width 18
#define root_gx_height 15
static unsigned char root_gx_bits[] = { 0x66, 0x00, 0x00, 0x14, 0x00, 0x00, 0x08, 0x00, 0x00, 0x14, 0x00, 0x00, 0x53, 0xfe, 0x03,
                                        0x21, 0x02, 0x02, 0x00, 0x02, 0x00, 0x00, 0x99, 0x00, 0x00, 0x91, 0x00, 0x10, 0x91, 0x00,
                                        0xa0, 0x50, 0x00, 0xc0, 0x60, 0x00, 0x80, 0x20, 0x00, 0x00, 0x14, 0x00, 0x00, 0x0c, 0x00 };

#define pow10_gx_width 13
#define pow10_gx_height 12
static unsigned char pow10_gx_bits[] = { 0x00, 0x12, 0x00, 0x0c, 0x00, 0x04, 0x00, 0x0a, 0x00, 0x09, 0x32, 0x00,
                                         0x4b, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x4a, 0x00, 0x32, 0x00 };

#define exp_gx_width 13
#define exp_gx_height 12
static unsigned char exp_gx_bits[] = { 0x00, 0xfb, 0x00, 0xf6, 0x00, 0xe6, 0x00, 0xf6, 0x80, 0xed, 0x18, 0xe0,
                                       0x36, 0xe0, 0x36, 0xe0, 0x1f, 0xe0, 0x03, 0xe0, 0x13, 0xe0, 0x0e, 0xe0 };
#define parens_gx_width 20
#define parens_gx_height 12
static unsigned char parens_gx_bits[] = { 0x0c, 0x00, 0x03, 0x06, 0x00, 0x06, 0x06, 0x00, 0x06, 0x03, 0x00, 0x0c,
                                          0x03, 0x00, 0x0c, 0x03, 0x00, 0x0c, 0x03, 0x00, 0x0c, 0x03, 0x00, 0x0c,
                                          0x03, 0x00, 0x0c, 0x06, 0x00, 0x06, 0x06, 0x00, 0x06, 0x0c, 0x00, 0x03 };

#define hash_gx_width 8
#define hash_gx_height 12
static unsigned char hash_gx_bits[] = { 0x00, 0x00, 0x48, 0x48, 0xfe, 0x24, 0x24, 0x7f, 0x12, 0x12, 0x00, 0x00 };

#define bracket_gx_width 12
#define bracket_gx_height 12
static unsigned char bracket_gx_bits[] = { 0x0f, 0x0f, 0x03, 0x0c, 0x03, 0x0c, 0x03, 0x0c, 0x03, 0x0c, 0x03, 0x0c,
                                           0x03, 0x0c, 0x03, 0x0c, 0x03, 0x0c, 0x03, 0x0c, 0x03, 0x0c, 0x0f, 0x0f };

#define under_gx_width 10
#define under_gx_height 12
static unsigned char under_gx_bits[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x03, 0xff, 0x03 };

#define prog_gx_width 24
#define prog_gx_height 12
static unsigned char prog_gx_bits[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0xc3, 0x18,
                                        0x8c, 0x81, 0x31, 0xc6, 0x00, 0x63, 0x63, 0x00, 0xc6, 0xc6, 0x00, 0x63,
                                        0x8c, 0x81, 0x31, 0x18, 0xc3, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

#define quote_gx_width 12
#define quote_gx_height 12
static unsigned char quote_gx_bits[] = { 0x05, 0x0a, 0x05, 0x0a, 0x05, 0x0a, 0x05, 0x0a, 0x00, 0x00, 0x00, 0x00,
                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

#define curly_gx_width 14
#define curly_gx_height 12
static unsigned char curly_gx_bits[] = { 0x0c, 0x0c, 0x06, 0x18, 0x06, 0x18, 0x06, 0x18, 0x06, 0x18, 0x03, 0x30,
                                         0x06, 0x18, 0x06, 0x18, 0x06, 0x18, 0x06, 0x18, 0x06, 0x18, 0x0c, 0x0c };

#define colon_gx_width 8
#define colon_gx_height 12
static unsigned char colon_gx_bits[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0xc3, 0xc3, 0x00, 0x00, 0xc3, 0xc3, 0x00 };

#define angle_gx_width 12
#define angle_gx_height 12
static unsigned char angle_gx_bits[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x01, 0xc0, 0x00, 0xe0, 0x01,
                                         0xb0, 0x03, 0x18, 0x03, 0x0c, 0x03, 0x06, 0x03, 0xff, 0x0f, 0xff, 0x0f };

#define pi_gx_width 10
#define pi_gx_height 12
static unsigned char pi_gx_bits[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xfe, 0x03, 0xff, 0x01,
                                      0xcc, 0x00, 0xcc, 0x00, 0xcc, 0x00, 0xcc, 0x00, 0xcc, 0x00, 0xcc, 0x00 };

#define nl_gx_width 18
#define nl_gx_height 12
static unsigned char nl_gx_bits[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x03, 0xf0, 0x00, 0x03,
                                      0xfc, 0x00, 0x03, 0xff, 0xff, 0x03, 0xff, 0xff, 0x03, 0xfc, 0x00, 0x00,
                                      0xf0, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

#define comma_gx_width 3
#define comma_gx_height 12
static unsigned char comma_gx_bits[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x07, 0x07, 0x04, 0x04, 0x02 };

#define arrow_gx_width 18
#define arrow_gx_height 12
static unsigned char arrow_gx_bits[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x3c, 0x00,
                                         0x00, 0xfc, 0x00, 0xff, 0xff, 0x03, 0xff, 0xff, 0x03, 0x00, 0xfc, 0x00,
                                         0x00, 0x3c, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

#define equal_gx_width 8
#define equal_gx_height 12
static unsigned char equal_gx_bits[] = { 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00 };

#endif /* _UI_SMALL_FONT_H 1 */