#include <cairo.h>
#include <gdk/gdkkeysyms.h>
#include <glib.h>
#include <gtk/gtk.h>

#include "api.h"
#include "inner.h"

// #define TEST_PASTE true

typedef struct {
    const button_t* key;
    GtkWidget* button;
    bool down;
    bool hold;
} gtk_button_t;

/*************/
/* Variables */
/*************/
static GtkWidget* gtk_ui_annunciators[ NB_ANNUNCIATORS ] = { NULL, NULL, NULL, NULL, NULL, NULL };

static gtk_button_t* gtk_ui_buttons;

static GtkWidget* gtk_ui_window;

static GtkWidget* gtk_ui_lcd_canvas;
static cairo_surface_t* gtk_ui_lcd_surface;

static char last_annunciators = 0;
static int display_buffer_grayscale[ LCD_WIDTH * 80 ];

/*************************/
/* Functions' prototypes */
/*************************/
static void gtk_ui_open_menu( int x, int y, void* data );

/*************/
/* Functions */
/*************/
static void gtk_ui_release_button( gtk_button_t* button )
{
    if ( !button->down )
        return;

    const button_t* key = button->key;

    button->down = false;
    button->hold = false;

    gtk_widget_remove_css_class( button->button, "key-down" );

    ui4x_emulator_api.release_key( key->hpkey );
}

static bool gtk_ui_press_button( gtk_button_t* button, bool hold )
{
    const button_t* key = button->key;

    if ( button->down ) {
        if ( button->hold && hold ) {
            gtk_ui_release_button( button );
            return GDK_EVENT_STOP;
        } else
            return GDK_EVENT_PROPAGATE;
    }

    button->down = true;
    button->hold = hold;

    gtk_widget_add_css_class( button->button, "key-down" );

    ui4x_emulator_api.press_key( key->hpkey );

    return GDK_EVENT_STOP;
}

static void gtk_ui_react_to_button_press( GtkGesture* _gesture, int _n_press, double _x, double _y, gtk_button_t* button )
{
    const button_t* key = button->key;

    gtk_ui_press_button( button, false );

    ui4x_emulator_api.press_key( key->hpkey );
}

static void gtk_ui_react_to_button_release( GtkGesture* _gesture, int _n_press, double _x, double _y, gtk_button_t* button )
{
    gtk_ui_release_button( button );
}

static void gtk_ui_react_to_button_right_click_release( gtk_button_t* button, GtkGesture* _gesture, int _n_press, double _x, double _y )
{
    const button_t* key = button->key;

    button->down = true;
    button->hold = true;

    gtk_ui_press_button( button, true );

    ui4x_emulator_api.press_key( key->hpkey );
}

static void gtk_ui_mount_sd_folder_file_dialog_callback( GtkFileDialog* dialog, GAsyncResult* result, void* _data )
{
    if ( ui4x_emulator_api.do_mount_sd == NULL )
        return;
    g_autoptr( GFile ) file = gtk_file_dialog_select_folder_finish( dialog, result, NULL );

    if ( file != NULL )
        ui4x_emulator_api.do_mount_sd( ( char* )g_file_peek_path( file ) );
}

static void gtk_ui_do_select_and_mount_sd_folder( void* data, GMenuItem* _menuitem )
{
    g_autoptr( GtkFileDialog ) dialog =
        g_object_new( GTK_TYPE_FILE_DIALOG, "title", "Choose SD folder…", "accept-label", "_Open", "modal", TRUE, NULL );

    gtk_file_dialog_select_folder( dialog, GTK_WINDOW( gtk_ui_window ), NULL,
                                   ( GAsyncReadyCallback )gtk_ui_mount_sd_folder_file_dialog_callback, data );
}

static void gtk_ui_do_start_gdb_server( GMenuItem* _menuitem, void* _data )
{
    if ( ui4x_emulator_api.do_debug == NULL )
        return;

    ui4x_emulator_api.do_debug();
}

static void gtk_ui_do_reset( void* _data, GMenuItem* _menuitem )
{
    if ( ui4x_emulator_api.do_reset == NULL )
        return;

    ui4x_emulator_api.do_reset();
}

#ifdef TEST_PASTE
static void x50g_string_to_keys_sequence( void* _data, const char* input )
{
    for ( int i = 0; i < strlen( input ); i++ )
        fprintf( stderr, "%c", input[ i ] );
    fprintf( stderr, "\n" );
}

static void gtk_ui_paste_callback( GdkClipboard* source, GAsyncResult* result, void* data )
{
    g_autofree char* text = NULL;
    g_autoptr( GError ) error = NULL;

    text = gdk_clipboard_read_text_finish( source, result, &error );

    if ( error ) {
        g_critical( "Couldn't paste text: %s\n", error->message );
        return;
    }

    x50g_string_to_keys_sequence( data, text );
}

static void gtk_ui_do_paste( void* data, GtkWidget* _menuitem )
{
    gdk_clipboard_read_text_async( gdk_display_get_clipboard( gdk_display_get_default() ), NULL,
                                   ( GAsyncReadyCallback )gtk_ui_paste_callback, data );
}
#endif

static void gtk_ui_do_quit( void* _data, GtkWidget* _menuitem )
{
    if ( ui4x_emulator_api.do_stop == NULL )
        return;

    ui4x_emulator_api.do_stop();
}

static void gtk_ui_open_menu( int x, int y, void* data )
{
    g_autoptr( GMenu ) menu = g_menu_new();
    g_autoptr( GSimpleActionGroup ) action_group = g_simple_action_group_new();

#ifdef TEST_PASTE
    g_autoptr( GSimpleAction ) act_paste = g_simple_action_new( "paste", NULL );
    g_signal_connect_swapped( act_paste, "activate", G_CALLBACK( gtk_ui_do_paste ), data );
    g_action_map_add_action( G_ACTION_MAP( action_group ), G_ACTION( act_paste ) );
    g_menu_append( menu, "Paste", "app.paste" );
#endif

    if ( ui4x_emulator_api.do_mount_sd != NULL && ui4x_emulator_api.do_unmount_sd != NULL && ui4x_emulator_api.is_sd_mounted != NULL &&
         ui4x_emulator_api.get_sd_path != NULL ) {
        g_autoptr( GSimpleAction ) act_mount_SD = g_simple_action_new( "mount_SD", NULL );
        g_signal_connect_swapped( act_mount_SD, "activate", G_CALLBACK( gtk_ui_do_select_and_mount_sd_folder ), data );
        if ( !ui4x_emulator_api.is_sd_mounted() )
            g_action_map_add_action( G_ACTION_MAP( action_group ), G_ACTION( act_mount_SD ) );
        g_menu_append( menu, "Mount SD folder…", "app.mount_SD" );

        g_autoptr( GSimpleAction ) act_unmount_SD = g_simple_action_new( "unmount_SD", NULL );
        g_signal_connect_swapped( act_unmount_SD, "activate", G_CALLBACK( ui4x_emulator_api.do_unmount_sd ), data );
        char* unmount_label;
        if ( ui4x_emulator_api.is_sd_mounted() ) {
            g_action_map_add_action( G_ACTION_MAP( action_group ), G_ACTION( act_unmount_SD ) );
            char* sd_path;
            ui4x_emulator_api.get_sd_path( &sd_path );
            if ( -1 == asprintf( &unmount_label, "Unmount SD (%s)", sd_path ) )
                exit( EXIT_FAILURE );
            free( sd_path );
        } else if ( -1 == asprintf( &unmount_label, "Unmount SD" ) )
            exit( EXIT_FAILURE );
        g_menu_append( menu, unmount_label, "app.unmount_SD" );
        free( unmount_label );
    }

    if ( ui4x_emulator_api.do_debug != NULL ) {
        g_autoptr( GSimpleAction ) act_debug = g_simple_action_new( "debug", NULL );
        g_signal_connect_swapped( act_debug, "activate", G_CALLBACK( gtk_ui_do_start_gdb_server ), data );
        /* if ( ui4x_config.debug_port != 0 ) */
        g_action_map_add_action( G_ACTION_MAP( action_group ), G_ACTION( act_debug ) );
        g_menu_append( menu, "Start gdb server", "app.debug" );
    }

    if ( ui4x_emulator_api.do_reset != NULL ) {
        g_autoptr( GSimpleAction ) act_reset = g_simple_action_new( "reset", NULL );
        g_signal_connect_swapped( act_reset, "activate", G_CALLBACK( gtk_ui_do_reset ), data );
        g_action_map_add_action( G_ACTION_MAP( action_group ), G_ACTION( act_reset ) );
        g_menu_append( menu, "Reset", "app.reset" );
    }

    if ( ui4x_emulator_api.do_stop != NULL ) {
        g_autoptr( GSimpleAction ) act_quit = g_simple_action_new( "quit", NULL );
        g_signal_connect_swapped( act_quit, "activate", G_CALLBACK( gtk_ui_do_quit ), data );
        g_action_map_add_action( G_ACTION_MAP( action_group ), G_ACTION( act_quit ) );
        g_menu_append( menu, "Quit", "app.quit" );
    }

    GtkWidget* popup = gtk_popover_menu_new_from_model( G_MENU_MODEL( menu ) );
    gtk_widget_insert_action_group( popup, "app", G_ACTION_GROUP( action_group ) );

    GdkRectangle rect;
    rect.x = x;
    rect.y = y;
    rect.width = rect.height = 1;
    gtk_popover_set_pointing_to( GTK_POPOVER( popup ), &rect );

    gtk_widget_set_parent( GTK_WIDGET( popup ), gtk_ui_window );
    gtk_popover_set_position( GTK_POPOVER( popup ), GTK_POS_BOTTOM );
    gtk_popover_popup( GTK_POPOVER( popup ) );
}

static void gtk_ui_redraw_lcd( GtkDrawingArea* _widget, cairo_t* cr, int width, int height, gpointer _user_data )
{
    cairo_pattern_t* lcd_pattern = cairo_pattern_create_for_surface( gtk_ui_lcd_surface );
    cairo_pattern_set_filter( lcd_pattern, CAIRO_FILTER_FAST );
    cairo_scale( cr, ( double )width / ( double )LCD_WIDTH, ( double )height / ( double )LCD_HEIGHT );
    cairo_set_source( cr, lcd_pattern );

    cairo_paint( cr );
}

static bool gtk_ui_handle_key_event( int keyval, void* data, key_event_t event_type )
{
    int hpkey;
    switch ( keyval ) {
        case GDK_KEY_a:
        case GDK_KEY_F1:
            hpkey = UI4X_KEY_A;
            break;
        case GDK_KEY_b:
        case GDK_KEY_F2:
            hpkey = UI4X_KEY_B;
            break;
        case GDK_KEY_c:
        case GDK_KEY_F3:
            hpkey = UI4X_KEY_C;
            break;
        case GDK_KEY_d:
        case GDK_KEY_F4:
            hpkey = UI4X_KEY_D;
            break;
        case GDK_KEY_e:
        case GDK_KEY_F5:
            hpkey = UI4X_KEY_E;
            break;
        case GDK_KEY_f:
        case GDK_KEY_F6:
            hpkey = UI4X_KEY_F;
            break;
        case GDK_KEY_g:
            hpkey = UI4X_KEY_G;
            break;
        case GDK_KEY_h:
            hpkey = UI4X_KEY_H;
            break;
        case GDK_KEY_i:
            hpkey = UI4X_KEY_I;
            break;
        case GDK_KEY_j:
            hpkey = UI4X_KEY_J;
            break;
        case GDK_KEY_k:
            hpkey = UI4X_KEY_K;
            break;
        case GDK_KEY_l:
            hpkey = UI4X_KEY_L;
            break;
        case GDK_KEY_Up:
        case GDK_KEY_KP_Up:
            hpkey = UI4X_KEY_UP;
            break;
        case GDK_KEY_Left:
        case GDK_KEY_KP_Left:
            hpkey = UI4X_KEY_LEFT;
            break;
        case GDK_KEY_Down:
        case GDK_KEY_KP_Down:
            hpkey = UI4X_KEY_DOWN;
            break;
        case GDK_KEY_Right:
        case GDK_KEY_KP_Right:
            hpkey = UI4X_KEY_RIGHT;
            break;
        case GDK_KEY_m:
            hpkey = UI4X_KEY_M;
            break;
        case GDK_KEY_n:
            hpkey = UI4X_KEY_N;
            break;
        case GDK_KEY_o:
        case GDK_KEY_apostrophe:
            hpkey = UI4X_KEY_O;
            break;
        case GDK_KEY_p:
            hpkey = UI4X_KEY_P;
            break;
        case GDK_KEY_BackSpace:
        case GDK_KEY_Delete:
        case GDK_KEY_KP_Delete:
            hpkey = UI4X_KEY_BACKSPACE;
            break;
        case GDK_KEY_dead_circumflex:
        case GDK_KEY_asciicircum:
        case GDK_KEY_q:
        case GDK_KEY_caret:
            hpkey = UI4X_KEY_Q;
            break;
        case GDK_KEY_r:
            hpkey = UI4X_KEY_R;
            break;
        case GDK_KEY_s:
            hpkey = UI4X_KEY_S;
            break;
        case GDK_KEY_t:
            hpkey = UI4X_KEY_T;
            break;
        case GDK_KEY_u:
            hpkey = UI4X_KEY_U;
            break;
        case GDK_KEY_v:
            hpkey = UI4X_KEY_V;
            break;
        case GDK_KEY_w:
            hpkey = UI4X_KEY_W;
            break;
        case GDK_KEY_x:
            hpkey = UI4X_KEY_X;
            break;
        case GDK_KEY_y:
            hpkey = UI4X_KEY_Y;
            break;
        case GDK_KEY_z:
        case GDK_KEY_slash:
        case GDK_KEY_KP_Divide:
            hpkey = UI4X_KEY_Z;
            break;
        case GDK_KEY_Tab:
#ifndef __APPLE__
        case GDK_KEY_Alt_L:
        case GDK_KEY_Alt_R:
        case GDK_KEY_Meta_L:
        case GDK_KEY_Meta_R:
        case GDK_KEY_Mode_switch:
#endif
            hpkey = UI4X_KEY_ALPHA;
            break;
        case GDK_KEY_7:
        case GDK_KEY_KP_7:
            hpkey = UI4X_KEY_7;
            break;
        case GDK_KEY_8:
        case GDK_KEY_KP_8:
            hpkey = UI4X_KEY_8;
            break;
        case GDK_KEY_9:
        case GDK_KEY_KP_9:
            hpkey = UI4X_KEY_9;
            break;
        case GDK_KEY_multiply:
        case GDK_KEY_KP_Multiply:
            hpkey = UI4X_KEY_MULTIPLY;
            break;
        case GDK_KEY_Shift_L:
            if ( ui4x_config.shiftless )
                return GDK_EVENT_PROPAGATE;
            else
                hpkey = UI4X_KEY_LSHIFT;
            break;
        case GDK_KEY_Shift_R:
            if ( ui4x_config.shiftless )
                return GDK_EVENT_PROPAGATE;
            else
                hpkey = UI4X_KEY_RSHIFT;
            break;
        case GDK_KEY_4:
        case GDK_KEY_KP_4:
            hpkey = UI4X_KEY_4;
            break;
        case GDK_KEY_5:
        case GDK_KEY_KP_5:
            hpkey = UI4X_KEY_5;
            break;
        case GDK_KEY_6:
        case GDK_KEY_KP_6:
            hpkey = UI4X_KEY_6;
            break;
        case GDK_KEY_minus:
        case GDK_KEY_KP_Subtract:
            hpkey = UI4X_KEY_MINUS;
            break;
        case GDK_KEY_Control_L:
            hpkey = UI4X_KEY_RSHIFT;
            break;
        case GDK_KEY_Control_R:
            hpkey = UI4X_KEY_LSHIFT;
            break;
        case GDK_KEY_1:
        case GDK_KEY_KP_1:
            hpkey = UI4X_KEY_1;
            break;
        case GDK_KEY_2:
        case GDK_KEY_KP_2:
            hpkey = UI4X_KEY_2;
            break;
        case GDK_KEY_3:
        case GDK_KEY_KP_3:
            hpkey = UI4X_KEY_3;
            break;
        case GDK_KEY_plus:
        case GDK_KEY_KP_Add:
            hpkey = UI4X_KEY_PLUS;
            break;
        case GDK_KEY_Escape:
            hpkey = UI4X_KEY_ON;
            break;
        case GDK_KEY_0:
        case GDK_KEY_KP_0:
            hpkey = UI4X_KEY_0;
            break;
        case GDK_KEY_period:
        case GDK_KEY_comma:
        case GDK_KEY_KP_Decimal:
        case GDK_KEY_KP_Separator:
            hpkey = UI4X_KEY_PERIOD;
            break;
        case GDK_KEY_space:
        case GDK_KEY_KP_Space:
            hpkey = UI4X_KEY_SPACE;
            break;
        case GDK_KEY_Return:
        case GDK_KEY_KP_Enter:
            hpkey = UI4X_KEY_ENTER;
            break;

        /* QWERTY compat: US English, UK English, International English */
        case GDK_KEY_backslash:
            hpkey = UI4X_KEY_MULTIPLY;
            break;
        case GDK_KEY_equal:
            hpkey = UI4X_KEY_PLUS;
            break;

        /* QWERTZ compat: German */
        case GDK_KEY_ssharp:
            hpkey = UI4X_KEY_Z;
            break;
        case GDK_KEY_numbersign:
            hpkey = UI4X_KEY_MULTIPLY;
            break;

        case GDK_KEY_F7:
        case GDK_KEY_F10:
            if ( ui4x_emulator_api.do_stop != NULL )
                ui4x_emulator_api.do_stop();
            return GDK_EVENT_STOP;

        case GDK_KEY_F12:
            switch ( event_type ) {
                case KEY_PRESS:
                    if ( ui4x_emulator_api.do_reset != NULL )
                        ui4x_emulator_api.do_reset();
                    if ( ui4x_emulator_api.do_sleep != NULL )
                        ui4x_emulator_api.do_sleep();
                    break;
                case KEY_RELEASE:
                    if ( ui4x_emulator_api.do_wake != NULL )
                        ui4x_emulator_api.do_wake();
                    break;
                default:
                    break;
            }
            return GDK_EVENT_STOP;

        case GDK_KEY_Menu:
            gtk_ui_open_menu( ( LCD_WIDTH * ui4x_config.zoom ) / 2, ( LCD_HEIGHT * ui4x_config.zoom ) / 2, data );
            return GDK_EVENT_STOP;

        default:
            return GDK_EVENT_PROPAGATE;
    }

    // Using GUI buttons:
    switch ( event_type ) {
        case KEY_PRESS:
            gtk_ui_react_to_button_press( NULL, 0, 0, 0, &gtk_ui_buttons[ hpkey ] );
            break;
        case KEY_RELEASE:
            gtk_ui_react_to_button_release( NULL, 0, 0, 0, &gtk_ui_buttons[ hpkey ] );
            break;
        default:
            return GDK_EVENT_PROPAGATE;
    }

    return GDK_EVENT_STOP;
}

static bool gtk_ui_react_to_key_press( GtkEventControllerKey* controller, guint keyval, guint keycode, GdkModifierType state, void* data )
{
    return gtk_ui_handle_key_event( keyval, data, KEY_PRESS );
}

static bool gtk_ui_react_to_key_release( GtkEventControllerKey* controller, guint keyval, guint keycode, GdkModifierType state, void* data )
{
    return gtk_ui_handle_key_event( keyval, data, KEY_RELEASE );
}

static void gtk_ui_react_to_display_click( void* data, GtkEventController* _gesture, gdouble x, gdouble y )
{
    gtk_ui_open_menu( ( int )x, ( int )y, data );
}

static GtkWidget* _gtk_ui_activate__create_annunciator_widget( const char* label )
{
    GtkWidget* gtk_ui_ann = gtk_label_new( NULL );
    gtk_widget_add_css_class( gtk_ui_ann, "annunciator" );
    gtk_widget_set_name( gtk_ui_ann, label );

    gtk_label_set_use_markup( GTK_LABEL( gtk_ui_ann ), true );
    gtk_label_set_markup( GTK_LABEL( gtk_ui_ann ), label );

    gtk_widget_set_opacity( gtk_ui_ann, 0 );

    return gtk_ui_ann;
}

static GtkWidget* _gtk_ui_activate__create_label( const char* css_class, const char* text )
{
    GtkWidget* gtk_ui_label = gtk_label_new( NULL );
    gtk_widget_add_css_class( gtk_ui_label, css_class );

    gtk_label_set_use_markup( GTK_LABEL( gtk_ui_label ), true );
    gtk_label_set_markup( GTK_LABEL( gtk_ui_label ), text );

    return gtk_ui_label;
}

static void _gtk_ui_activate__load_and_apply_CSS( void )
{
    if ( ui4x_config.style_filename == NULL ) {
        fprintf( stderr, "No style defined\n" );
        return;
    }
    if ( !g_file_test( ui4x_config.style_filename, G_FILE_TEST_EXISTS ) ) {
        fprintf( stderr, "Can't load style %s\n", ui4x_config.style_filename );
        return;
    }

    g_autoptr( GtkCssProvider ) style_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_path( style_provider, ui4x_config.style_filename );

    gtk_style_context_add_provider_for_display( gdk_display_get_default(), GTK_STYLE_PROVIDER( style_provider ),
                                                GTK_STYLE_PROVIDER_PRIORITY_USER + 1 );

    if ( ui4x_config.verbose )
        fprintf( stderr, "Loaded style from %s\n", ui4x_config.style_filename );
}

static void gtk_ui_activate( GtkApplication* app, void* data )
{
    // create gtk_ui_window and widgets/stuff
    if ( app == NULL )
        gtk_ui_window = gtk_window_new();
    else
        gtk_ui_window = gtk_application_window_new( app );

    gtk_window_set_decorated( GTK_WINDOW( gtk_ui_window ), true );
    gtk_window_set_resizable( GTK_WINDOW( gtk_ui_window ), true );
    gtk_window_set_title( GTK_WINDOW( gtk_ui_window ), ui4x_config.name );
    gtk_window_set_decorated( GTK_WINDOW( gtk_ui_window ), true );
    // Sets the title of this instance
    g_set_application_name( ui4x_config.name );
    // Sets the app_id of all instances
    g_set_prgname( ui4x_config.progname );

    GtkWidget* window_container = gtk_box_new( ui4x_config.netbook ? GTK_ORIENTATION_HORIZONTAL : GTK_ORIENTATION_VERTICAL, 0 );
    gtk_widget_add_css_class( window_container, "window-container" );
    gtk_widget_set_name( window_container, "window-container" );

    gtk_window_set_child( ( GtkWindow* )gtk_ui_window, window_container );

    g_signal_connect_swapped( G_OBJECT( gtk_ui_window ), "destroy", G_CALLBACK( gtk_ui_do_quit ), data );

    GtkEventController* keys_controller = gtk_event_controller_key_new();
    g_signal_connect( keys_controller, "key-pressed", G_CALLBACK( gtk_ui_react_to_key_press ), data );
    g_signal_connect( keys_controller, "key-released", G_CALLBACK( gtk_ui_react_to_key_release ), data );
    gtk_widget_add_controller( gtk_ui_window, keys_controller );

    /* for --netbook */
    GtkWidget* upper_left_container = gtk_box_new( GTK_ORIENTATION_VERTICAL, 0 );
    gtk_widget_add_css_class( upper_left_container, "upper-left-container" );
    gtk_widget_set_name( upper_left_container, "upper-left-container" );
    gtk_box_append( ( GTK_BOX( window_container ) ), upper_left_container );

    GtkWidget* downer_right_container = gtk_box_new( GTK_ORIENTATION_VERTICAL, 0 );
    gtk_widget_set_vexpand( GTK_WIDGET( downer_right_container ), true );
    gtk_widget_add_css_class( downer_right_container, "downer-right-container" );
    gtk_widget_set_name( downer_right_container, "downer-right-container" );
    gtk_box_append( GTK_BOX( window_container ), downer_right_container );

    GtkWidget* header_container = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 0 );
    gtk_widget_add_css_class( header_container, "header-container" );
    gtk_widget_set_name( header_container, "header-container" );
    gtk_box_append( ( GTK_BOX( upper_left_container ) ), header_container );

    gtk_ui_lcd_canvas = gtk_drawing_area_new();
    gtk_widget_add_css_class( gtk_ui_lcd_canvas, "lcd" );
    gtk_widget_set_name( gtk_ui_lcd_canvas, "lcd" );

    gtk_drawing_area_set_content_width( GTK_DRAWING_AREA( gtk_ui_lcd_canvas ), ( LCD_WIDTH * ui4x_config.zoom ) );
    gtk_drawing_area_set_content_height( GTK_DRAWING_AREA( gtk_ui_lcd_canvas ), ( LCD_HEIGHT * ui4x_config.zoom ) );
    gtk_drawing_area_set_draw_func( GTK_DRAWING_AREA( gtk_ui_lcd_canvas ), gtk_ui_redraw_lcd, data, NULL );

    GtkWidget* lcd_container = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 0 );
    gtk_widget_set_halign( lcd_container, GTK_ALIGN_CENTER );
    gtk_widget_add_css_class( lcd_container, "lcd-container" );
    gtk_widget_set_name( lcd_container, "lcd-container" );

    gtk_widget_set_size_request( lcd_container, ( LCD_WIDTH * ui4x_config.zoom ), ( LCD_HEIGHT * ui4x_config.zoom ) );
    gtk_box_append( GTK_BOX( lcd_container ), gtk_ui_lcd_canvas );
    gtk_widget_set_halign( GTK_WIDGET( gtk_ui_lcd_canvas ), GTK_ALIGN_CENTER );
    gtk_widget_set_hexpand( GTK_WIDGET( gtk_ui_lcd_canvas ), false );

    GtkWidget* annunciators_container = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 0 );
    gtk_box_set_homogeneous( GTK_BOX( annunciators_container ), true );
    gtk_widget_add_css_class( annunciators_container, "annunciators-container" );
    gtk_widget_set_name( annunciators_container, "annunciators-container" );

    for ( int i = 0; i < NB_ANNUNCIATORS; i++ ) {
        gtk_ui_annunciators[ i ] = _gtk_ui_activate__create_annunciator_widget( ui_annunciators[ i ] );
        gtk_box_append( GTK_BOX( annunciators_container ), gtk_ui_annunciators[ i ] );
    }

    GtkWidget* display_container = gtk_box_new( GTK_ORIENTATION_VERTICAL, 0 );
    gtk_widget_add_css_class( annunciators_container, "display-container" );
    gtk_widget_set_name( display_container, "display-container" );

    gtk_box_append( GTK_BOX( display_container ), annunciators_container );
    gtk_box_append( GTK_BOX( display_container ), lcd_container );

    gtk_box_append( GTK_BOX( upper_left_container ), display_container );

    GtkGesture* right_click_controller = gtk_gesture_click_new();
    gtk_gesture_single_set_button( GTK_GESTURE_SINGLE( right_click_controller ), 3 );
    g_signal_connect_swapped( right_click_controller, "pressed", G_CALLBACK( gtk_ui_react_to_display_click ), data );
    gtk_widget_add_controller( display_container, GTK_EVENT_CONTROLLER( right_click_controller ) );

    // keyboard
    GtkWidget* high_keyboard_container = gtk_box_new( GTK_ORIENTATION_VERTICAL, 0 );
    gtk_widget_add_css_class( high_keyboard_container, "keyboard-container" );
    gtk_widget_set_name( high_keyboard_container, "high-keyboard-container" );

    gtk_box_set_homogeneous( GTK_BOX( high_keyboard_container ), true );

    gtk_box_append( GTK_BOX( upper_left_container ), high_keyboard_container );

    GtkWidget* low_keyboard_container = gtk_box_new( GTK_ORIENTATION_VERTICAL, 0 );
    gtk_widget_set_valign( GTK_WIDGET( low_keyboard_container ), GTK_ALIGN_END );
    gtk_widget_set_vexpand( GTK_WIDGET( low_keyboard_container ), true );
    gtk_widget_add_css_class( low_keyboard_container, "keyboard-container" );
    gtk_widget_set_name( low_keyboard_container, "low-keyboard-container" );

    gtk_box_set_homogeneous( GTK_BOX( low_keyboard_container ), true );

    gtk_box_append( GTK_BOX( downer_right_container ), low_keyboard_container );

    gtk_button_t* button;

    GtkWidget* rows_containers[ 10 ]; /* max rows is 10 */
    GtkWidget* keys_containers[ NB_HP4950_KEYS ];
    GtkWidget* keys_top_labels_containers[ NB_HP4950_KEYS ];

    gtk_ui_buttons = malloc( NB_KEYS * sizeof( gtk_button_t ) );
    if ( NULL == gtk_ui_buttons ) {
        fprintf( stderr, "%s:%u: Out of memory\n", __func__, __LINE__ );
        return;
    }
    memset( gtk_ui_buttons, 0, NB_KEYS * sizeof( gtk_button_t ) );

    int key_index = 0;
    int nb_keys_in_row = 0;
    for ( int row = 0; row < ( ui4x_config.model == MODEL_48GX || ui4x_config.model == MODEL_48SX ? 9 : 10 ); row++ ) {
        rows_containers[ row ] = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 0 );
        gtk_widget_add_css_class( rows_containers[ row ], "row-container" );
        gtk_box_set_homogeneous( GTK_BOX( rows_containers[ row ] ), true );
        gtk_box_append( ( GTK_BOX( row < ui4x_config.netbook_pivot_line ? high_keyboard_container : low_keyboard_container ) ),
                        rows_containers[ row ] );

        if ( ui4x_config.model == MODEL_49G || ui4x_config.model == MODEL_50G )
            switch ( row ) {
                case 1:
                    nb_keys_in_row = 4;
                    break;
                case 0:
                case 2:
                    nb_keys_in_row = 6;
                    break;
                default:
                    nb_keys_in_row = 5;
            }
        else
            nb_keys_in_row = row < 4 ? 6 : 5;

        for ( int column = 0; column < nb_keys_in_row; column++ ) {
            keys_containers[ key_index ] = gtk_box_new( GTK_ORIENTATION_VERTICAL, 0 );
            gtk_widget_add_css_class( keys_containers[ key_index ], "key-container" );
            gtk_box_set_homogeneous( GTK_BOX( keys_containers[ key_index ] ), false );
            if ( ( ui4x_config.model == MODEL_49G || ui4x_config.model == MODEL_50G ) && row == 1 && column == 3 )
                gtk_box_append( GTK_BOX( rows_containers[ row ] ), gtk_box_new( GTK_ORIENTATION_VERTICAL, 2 ) );
            gtk_box_append( GTK_BOX( rows_containers[ row ] ), keys_containers[ key_index ] );
            if ( ( ui4x_config.model == MODEL_49G || ui4x_config.model == MODEL_50G ) && row == 1 && column == 3 )
                gtk_box_append( GTK_BOX( rows_containers[ row ] ), gtk_box_new( GTK_ORIENTATION_VERTICAL, 2 ) );

            button = &gtk_ui_buttons[ key_index ];
            button->key = &BUTTONS[ key_index ];

            keys_top_labels_containers[ key_index ] = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 0 );
            gtk_widget_add_css_class( keys_top_labels_containers[ key_index ], "top-labels-container" );

            gtk_box_append( GTK_BOX( keys_containers[ key_index ] ), keys_top_labels_containers[ key_index ] );

            GtkWidget* label_left = NULL;
            if ( button->key->left )
                label_left = _gtk_ui_activate__create_label( "label-left", button->key->left );

            GtkWidget* label_right = NULL;
            if ( button->key->right )
                label_right = _gtk_ui_activate__create_label( "label-right", button->key->right );

            if ( button->key->right )
                label_right = _gtk_ui_activate__create_label( "label-right", button->key->right );

            if ( button->key->left && button->key->right ) {
                gtk_box_append( GTK_BOX( keys_top_labels_containers[ key_index ] ), label_left );
                gtk_widget_set_halign( GTK_WIDGET( label_left ), GTK_ALIGN_START );

                gtk_box_append( GTK_BOX( keys_top_labels_containers[ key_index ] ), label_right );
                gtk_widget_set_halign( GTK_WIDGET( label_right ), GTK_ALIGN_END );
                gtk_widget_set_hexpand( label_right, true );
            } else if ( button->key->left ) {
                gtk_widget_set_halign( GTK_WIDGET( keys_top_labels_containers[ key_index ] ), GTK_ALIGN_CENTER );

                gtk_box_append( GTK_BOX( keys_top_labels_containers[ key_index ] ), label_left );
                gtk_widget_set_halign( GTK_WIDGET( label_left ), GTK_ALIGN_CENTER );
                gtk_widget_set_hexpand( GTK_WIDGET( label_left ), false );
            } else if ( button->key->right ) {
                gtk_widget_set_halign( GTK_WIDGET( keys_top_labels_containers[ key_index ] ), GTK_ALIGN_CENTER );

                gtk_box_append( GTK_BOX( keys_top_labels_containers[ key_index ] ), label_right );
                gtk_widget_set_halign( GTK_WIDGET( label_right ), GTK_ALIGN_CENTER );
                gtk_widget_set_hexpand( GTK_WIDGET( label_right ), false );
            }

            button->button = gtk_button_new();
            gtk_widget_add_css_class( button->button, "key" );
            if ( button->key->css_class != NULL )
                gtk_widget_add_css_class( button->button, button->key->css_class );
            if ( button->key->css_id != NULL )
                gtk_widget_set_name( button->button, button->key->css_id );

            // There's always a label, even if it's empty.
            GtkWidget* label = _gtk_ui_activate__create_label( "label-key", button->key->label );
            gtk_button_set_child( GTK_BUTTON( button->button ), label );

            gtk_widget_set_can_focus( button->button, false );
            GtkGesture* btn_left_click_controller = gtk_gesture_click_new();
            gtk_gesture_single_set_button( GTK_GESTURE_SINGLE( btn_left_click_controller ), 1 );
            g_signal_connect( btn_left_click_controller, "pressed", G_CALLBACK( gtk_ui_react_to_button_press ), button );
            g_signal_connect( btn_left_click_controller, /* "released" */ "end", G_CALLBACK( gtk_ui_react_to_button_release ), button );
            /* Here we attach the controller to the label because… gtk4 reasons? gtk4 button only handles 'clicked' event now but we
             * actually need pressed and released (AKA end?) */
            gtk_widget_add_controller( label, GTK_EVENT_CONTROLLER( btn_left_click_controller ) );

            GtkGesture* btn_right_click_controller = gtk_gesture_click_new();
            gtk_gesture_single_set_button( GTK_GESTURE_SINGLE( btn_right_click_controller ), 3 );
            g_signal_connect_swapped( btn_right_click_controller, /* "released" */ "pressed",
                                      G_CALLBACK( gtk_ui_react_to_button_right_click_release ), button );
            gtk_widget_add_controller( label, GTK_EVENT_CONTROLLER( btn_right_click_controller ) );

            gtk_box_append( GTK_BOX( keys_containers[ key_index ] ), button->button );

            if ( button->key->below )
                gtk_box_append( GTK_BOX( keys_containers[ key_index ] ),
                                _gtk_ui_activate__create_label( "label-below", button->key->below ) );
            if ( button->key->letter )
                gtk_box_append( GTK_BOX( keys_containers[ key_index ] ),
                                _gtk_ui_activate__create_label( "label-letter", button->key->letter ) );

            key_index++;
        }
    }

    _gtk_ui_activate__load_and_apply_CSS();

    // finally show the window
    gtk_widget_realize( gtk_ui_window );
    gtk_window_present( GTK_WINDOW( gtk_ui_window ) );
}

void gtk_ui_handle_pending_inputs( void )
{
    while ( g_main_context_pending( NULL ) )
        g_main_context_iteration( NULL, false );
}

static void gtk_ui_refresh_annunciators( void )
{
    int annunciators = ui4x_emulator_api.get_annunciators();

    if ( last_annunciators == annunciators )
        return;

    last_annunciators = annunciators;

    for ( int i = 0; i < NB_ANNUNCIATORS; i++ )
        gtk_widget_set_opacity( gtk_ui_annunciators[ i ], ( annunciators >> i ) & 0x01 ? 1 : 0 );
}

void gtk_ui_refresh_lcd( void )
{
    if ( !ui4x_emulator_api.is_display_on() )
        return;

    gtk_ui_refresh_annunciators();

    if ( NULL != gtk_ui_lcd_surface )
        g_free( gtk_ui_lcd_surface );
    gtk_ui_lcd_surface = cairo_image_surface_create( CAIRO_FORMAT_ARGB32, LCD_WIDTH, LCD_HEIGHT );
    cairo_t* cr = cairo_create( gtk_ui_lcd_surface );

    int n_levels_of_gray = N_LEVELS_OF_GRAY * 1.0;
    int pixel_on_r = ( ( COLORS[ COLOR_PIXEL_ON ].rgb >> 16 ) & 0xff ) / 256.0;
    int pixel_on_g = ( ( COLORS[ COLOR_PIXEL_ON ].rgb >> 8 ) & 0xff ) / 256.0;
    int pixel_on_b = ( COLORS[ COLOR_PIXEL_ON ].rgb & 0xff ) / 256.0;

    ui4x_emulator_api.get_lcd_buffer( display_buffer_grayscale );
    for ( int y = 0; y < LCD_HEIGHT; y++ ) {
        for ( int x = 0; x < LCD_WIDTH; x++ ) {
            cairo_set_source_rgba( cr, pixel_on_r, pixel_on_g, pixel_on_b,
                                   display_buffer_grayscale[ ( y * LCD_WIDTH ) + x ] / ( n_levels_of_gray - 1.0 ) );
            cairo_rectangle( cr, x, y, 1.0, 1.0 );
            cairo_fill( cr );
        }
    }

    cairo_destroy( cr );

    gtk_widget_queue_draw( gtk_ui_lcd_canvas );

    gdk_display_flush( gdk_display_get_default() );
}

void gtk_init_ui( void )
{
    /* g_autoptr( GtkApplication ) app = gtk_application_new( NULL, 0 ); */

    /* g_signal_connect( app, "activate", G_CALLBACK( gtk_ui_activate ), NULL ); */

    /* g_application_run( G_APPLICATION( app ), 0, NULL ); */

    gtk_init();
    gtk_ui_activate( NULL, NULL );
}

void gtk_exit_ui( void ) {}
