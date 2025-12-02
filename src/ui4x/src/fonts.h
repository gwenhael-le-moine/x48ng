#ifndef _UI4x_FONTS_H
#  define _UI4x_FONTS_H 1

typedef struct letter_t {
    unsigned int w, h;
    unsigned char* bits;
} letter_t;

/**************/
/* SMALL FONT */
/**************/

#  define greater_than_width 6
#  define greater_than_height 7
extern unsigned char greater_than_bits[];

#  define greater_than_or_eq_width 6
#  define greater_than_or_eq_height 7
extern unsigned char greater_than_or_eq_bits[];

#  define lesser_than_width 6
#  define lesser_than_height 7
extern unsigned char lesser_than_bits[];

#  define lesser_than_or_eq_width 6
#  define lesser_than_or_eq_height 7
extern unsigned char lesser_than_or_eq_bits[];

#  define infinity_width 7
#  define infinity_height 7
extern unsigned char infinity_bits[];

#  define not_equal_width 7
#  define not_equal_height 7
extern unsigned char not_equal_bits[];

#  define ampersand_width 7
#  define ampersand_height 7
extern unsigned char ampersand_bits[];

#  define dot_width 3
#  define dot_height 7
extern unsigned char dot_bits[];

#  define single_quote_width 3
#  define single_quote_height 7
extern unsigned char single_quote_bits[];

#  define blank_width 4
#  define blank_height 7
extern unsigned char blank_bits[];

#  define hash_width 5
#  define hash_height 7
extern unsigned char hash_bits[];

#  define lbrace_width 3
#  define lbrace_height 7
extern unsigned char lbrace_bits[];

#  define rbrace_width 3
#  define rbrace_height 7
extern unsigned char rbrace_bits[];

#  define comma_width 3
#  define comma_height 7
extern unsigned char comma_bits[];

#  define slash_width 3
#  define slash_height 7
extern unsigned char slash_bits[];

#  define zero_width 5
#  define zero_height 7
extern unsigned char zero_bits[];

#  define one_width 5
#  define one_height 7
extern unsigned char one_bits[];

#  define two_width 5
#  define two_height 7
extern unsigned char two_bits[];

#  define three_width 5
#  define three_height 7
extern unsigned char three_bits[];

#  define four_width 5
#  define four_height 7
extern unsigned char four_bits[];

#  define five_width 5
#  define five_height 7
extern unsigned char five_bits[];

#  define six_width 5
#  define six_height 7
extern unsigned char six_bits[];

#  define seven_width 5
#  define seven_height 7
extern unsigned char seven_bits[];

#  define eight_width 5
#  define eight_height 7
extern unsigned char eight_bits[];

#  define nine_width 5
#  define nine_height 7
extern unsigned char nine_bits[];

#  define small_colon_width 2
#  define small_colon_height 7
extern unsigned char small_colon_bits[];

#  define d_width 5
#  define d_height 7
extern unsigned char d_bits[];

#  define e_width 5
#  define e_height 7
extern unsigned char e_bits[];

#  define i_width 5
#  define i_height 7
extern unsigned char i_bits[];

#  define p_width 5
#  define p_height 7
extern unsigned char p_bits[];

#  define r_width 5
#  define r_height 7
extern unsigned char r_bits[];

#  define s_width 5
#  define s_height 7
extern unsigned char s_bits[];

#  define t_width 5
#  define t_height 7
extern unsigned char t_bits[];

#  define v_width 5
#  define v_height 7
extern unsigned char v_bits[];

#  define w_width 5
#  define w_height 7
extern unsigned char w_bits[];

#  define y_width 5
#  define y_height 7
extern unsigned char y_bits[];

#  define A_width 5
#  define A_height 7
extern unsigned char A_bits[];

#  define B_width 5
#  define B_height 7
extern unsigned char B_bits[];

#  define C_width 5
#  define C_height 7
extern unsigned char C_bits[];

#  define D_width 5
#  define D_height 7
extern unsigned char D_bits[];

#  define E_width 5
#  define E_height 7
extern unsigned char E_bits[];

#  define F_width 5
#  define F_height 7
extern unsigned char F_bits[];

#  define G_width 5
#  define G_height 7
extern unsigned char G_bits[];

#  define H_width 5
#  define H_height 7
extern unsigned char H_bits[];

#  define I_width 1
#  define I_height 7
extern unsigned char I_bits[];

#  define J_width 4
#  define J_height 7
extern unsigned char J_bits[];

#  define K_width 5
#  define K_height 7
extern unsigned char K_bits[];

#  define L_width 4
#  define L_height 7
extern unsigned char L_bits[];

#  define M_width 5
#  define M_height 7
extern unsigned char M_bits[];

#  define N_width 5
#  define N_height 7
extern unsigned char N_bits[];

#  define O_width 5
#  define O_height 7
extern unsigned char O_bits[];

#  define P_width 5
#  define P_height 7
extern unsigned char P_bits[];

#  define Q_width 5
#  define Q_height 7
extern unsigned char Q_bits[];

#  define R_width 5
#  define R_height 7
extern unsigned char R_bits[];

#  define S_width 5
#  define S_height 7
extern unsigned char S_bits[];

#  define T_width 5
#  define T_height 7
extern unsigned char T_bits[];

#  define U_width 5
#  define U_height 7
extern unsigned char U_bits[];

#  define V_width 5
#  define V_height 7
extern unsigned char V_bits[];

#  define W_width 5
#  define W_height 7
extern unsigned char W_bits[];

#  define X_width 5
#  define X_height 7
extern unsigned char X_bits[];

#  define Y_width 5
#  define Y_height 7
extern unsigned char Y_bits[];

#  define Z_width 5
#  define Z_height 7
extern unsigned char Z_bits[];

#  define lbracket_width 3
#  define lbracket_height 7
extern unsigned char lbracket_bits[];

#  define rbracket_width 3
#  define rbracket_height 7
extern unsigned char rbracket_bits[];

#  define arrow_width 7
#  define arrow_height 7
extern unsigned char arrow_bits[];

#  define diff_width 5
#  define diff_height 7
extern unsigned char diff_bits[];

#  define integral_width 5
#  define integral_height 8
extern unsigned char integral_bits[];

#  define sigma_width 6
#  define sigma_height 9
extern unsigned char sigma_bits[];

#  define sqr_width 11
#  define sqr_height 10
extern unsigned char sqr_bits[];

#  define root_width 18
#  define root_height 13
extern unsigned char root_bits[];

#  define pow10_width 13
#  define pow10_height 9
extern unsigned char pow10_bits[];

#  define exp_width 11
#  define exp_height 9
extern unsigned char exp_bits[];

#  define under_width 6
#  define under_height 7
extern unsigned char under_bits[];

#  define prog_width 16
#  define prog_height 7
extern unsigned char prog_bits[];

#  define string_width 10
#  define string_height 7
extern unsigned char string_bits[];

#  define equal_width 5
#  define equal_height 7
extern unsigned char equal_bits[];

#  define nl_width 8
#  define nl_height 7
extern unsigned char nl_bits[];

#  define pi_width 6
#  define pi_height 7
extern unsigned char pi_bits[];

#  define angle_width 8
#  define angle_height 7
extern unsigned char angle_bits[];

#  define lcurly_width 5
#  define lcurly_height 7
extern unsigned char lcurly_bits[];

#  define rcurly_width 5
#  define rcurly_height 7
extern unsigned char rcurly_bits[];

#  define sqr_48gx_width 11
#  define sqr_48gx_height 13
extern unsigned char sqr_48gx_bits[];

#  define root_48gx_width 18
#  define root_48gx_height 15
extern unsigned char root_48gx_bits[];

#  define pow10_48gx_width 13
#  define pow10_48gx_height 12
extern unsigned char pow10_48gx_bits[];

#  define parens_48gx_width 20
#  define parens_48gx_height 12
extern unsigned char parens_48gx_bits[];

#  define exp_48gx_width 13
#  define exp_48gx_height 12
extern unsigned char exp_48gx_bits[];

#  define hash_48gx_width 8
#  define hash_48gx_height 12
extern unsigned char hash_48gx_bits[];

#  define bracket_48gx_width 12
#  define bracket_48gx_height 12
extern unsigned char bracket_48gx_bits[];

#  define under_48gx_width 10
#  define under_48gx_height 12
extern unsigned char under_48gx_bits[];

#  define prog_48gx_width 24
#  define prog_48gx_height 12
extern unsigned char prog_48gx_bits[];

#  define quote_48gx_width 12
#  define quote_48gx_height 12
extern unsigned char quote_48gx_bits[];

#  define curly_48gx_width 14
#  define curly_48gx_height 12
extern unsigned char curly_48gx_bits[];

#  define colon_48gx_width 8
#  define colon_48gx_height 12
extern unsigned char colon_48gx_bits[];

#  define angle_48gx_width 12
#  define angle_48gx_height 12
extern unsigned char angle_48gx_bits[];

#  define pi_48gx_width 10
#  define pi_48gx_height 12
extern unsigned char pi_48gx_bits[];

#  define nl_48gx_width 18
#  define nl_48gx_height 12
extern unsigned char nl_48gx_bits[];

#  define comma_48gx_width 3
#  define comma_48gx_height 12
extern unsigned char comma_48gx_bits[];

#  define arrow_48gx_width 18
#  define arrow_48gx_height 12
extern unsigned char arrow_48gx_bits[];

#  define equal_48gx_width 8
#  define equal_48gx_height 12
extern unsigned char equal_48gx_bits[];

extern letter_t small_font[ 128 ];

/************/
/* BIG FONT */
/************/

#  define big_font_dot_width 8
#  define big_font_dot_height 13
extern unsigned char big_font_dot_bits[];

#  define big_font_0_width 8
#  define big_font_0_height 13
extern unsigned char big_font_0_bits[];

#  define big_font_1_width 8
#  define big_font_1_height 13
extern unsigned char big_font_1_bits[];

#  define big_font_2_width 8
#  define big_font_2_height 13
extern unsigned char big_font_2_bits[];

#  define big_font_3_width 8
#  define big_font_3_height 13
extern unsigned char big_font_3_bits[];

#  define big_font_4_width 8
#  define big_font_4_height 13
extern unsigned char big_font_4_bits[];

#  define big_font_5_width 8
#  define big_font_5_height 13
extern unsigned char big_font_5_bits[];

#  define big_font_6_width 8
#  define big_font_6_height 13
extern unsigned char big_font_6_bits[];

#  define big_font_7_width 8
#  define big_font_7_height 13
extern unsigned char big_font_7_bits[];

#  define big_font_8_width 8
#  define big_font_8_height 13
extern unsigned char big_font_8_bits[];

#  define big_font_9_width 8
#  define big_font_9_height 13
extern unsigned char big_font_9_bits[];

#  define big_font_A_width 8
#  define big_font_A_height 10
extern unsigned char big_font_A_bits[];

#  define big_font_B_width 8
#  define big_font_B_height 10
extern unsigned char big_font_B_bits[];

#  define big_font_C_width 8
#  define big_font_C_height 10
extern unsigned char big_font_C_bits[];

#  define big_font_D_width 8
#  define big_font_D_height 10
extern unsigned char big_font_D_bits[];

#  define big_font_E_width 8
#  define big_font_E_height 10
extern unsigned char big_font_E_bits[];

#  define big_font_F_width 8
#  define big_font_F_height 10
extern unsigned char big_font_F_bits[];

#  define big_font_G_width 8
#  define big_font_G_height 10
extern unsigned char big_font_G_bits[];

#  define big_font_H_width 8
#  define big_font_H_height 10
extern unsigned char big_font_H_bits[];

#  define big_font_I_width 8
#  define big_font_I_height 10
extern unsigned char big_font_I_bits[];

#  define big_font_J_width 8
#  define big_font_J_height 10
extern unsigned char big_font_J_bits[];

#  define big_font_K_width 8
#  define big_font_K_height 10
extern unsigned char big_font_K_bits[];

#  define big_font_L_width 8
#  define big_font_L_height 10
extern unsigned char big_font_L_bits[];

#  define big_font_M_width 8
#  define big_font_M_height 10
extern unsigned char big_font_M_bits[];

#  define big_font_N_width 8
#  define big_font_N_height 10
extern unsigned char big_font_N_bits[];

#  define big_font_O_width 8
#  define big_font_O_height 10
extern unsigned char big_font_O_bits[];

#  define big_font_P_width 8
#  define big_font_P_height 10
extern unsigned char big_font_P_bits[];

#  define big_font_Q_width 8
#  define big_font_Q_height 10
extern unsigned char big_font_Q_bits[];

#  define big_font_R_width 8
#  define big_font_R_height 10
extern unsigned char big_font_R_bits[];

#  define big_font_S_width 8
#  define big_font_S_height 10
extern unsigned char big_font_S_bits[];

#  define big_font_T_width 8
#  define big_font_T_height 10
extern unsigned char big_font_T_bits[];

#  define big_font_U_width 8
#  define big_font_U_height 10
extern unsigned char big_font_U_bits[];

#  define big_font_V_width 8
#  define big_font_V_height 10
extern unsigned char big_font_V_bits[];

#  define big_font_W_width 8
#  define big_font_W_height 10
extern unsigned char big_font_W_bits[];

#  define big_font_X_width 8
#  define big_font_X_height 10
extern unsigned char big_font_X_bits[];

#  define big_font_Y_width 8
#  define big_font_Y_height 10
extern unsigned char big_font_Y_bits[];

extern letter_t big_font[ 128 ];

#endif /* !(_UI4x_FONTS_H) */
