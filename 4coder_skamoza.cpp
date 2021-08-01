#pragma once

#include "4coder_default_include.cpp"

// NOTE(allen): Users can declare their own managed IDs here.

#include "generated/managed_id_metadata.cpp"

String_ID mapid_shared;
String_ID mapid_normal;
String_ID mapid_insert;
String_ID mapid_visual;

void set_current_mapid( Application_Links* app, Command_Map_ID mapid ) {
    
    View_ID view = get_active_view( app, 0 );
    Buffer_ID buffer = view_get_buffer( app, view, 0 );
    Managed_Scope scope = buffer_get_managed_scope( app, buffer );
    Command_Map_ID* map_id_ptr = scope_attachment( app, scope, buffer_map_id, Command_Map_ID );
    *map_id_ptr = mapid;
}

CUSTOM_COMMAND_SIG( go_to_normal_mode ) {
    
    set_current_mapid( app, mapid_normal );
    
    active_color_table.arrays[ defcolor_cursor ].vals[ 0 ] = 0xffff5533;
    active_color_table.arrays[ defcolor_at_cursor ].vals[ 0 ] = 0xff00aacc;
    active_color_table.arrays[ defcolor_margin_active ].vals[ 0 ] = 0xffff5533;
}

CUSTOM_COMMAND_SIG( go_to_insert_mode ) {
    
    set_current_mapid( app, mapid_insert );
    
    active_color_table.arrays[ defcolor_cursor ].vals[ 0 ] = 0xff80ff80;
    active_color_table.arrays[ defcolor_at_cursor ].vals[ 0 ] = 0xff293134;
    active_color_table.arrays[ defcolor_margin_active ].vals[ 0 ] = 0xff80ff80;
}

CUSTOM_COMMAND_SIG( go_to_visual_mode ) {
    
    set_current_mapid( app, mapid_visual );
    
    active_color_table.arrays[ defcolor_cursor ].vals[ 0 ] = 0xffffff00;
    active_color_table.arrays[ defcolor_at_cursor ].vals[ 0 ] = 0xff0000ff;
}

void
custom_layer_init(Application_Links *app){
    Thread_Context *tctx = get_thread_context(app);
    
    // NOTE(allen): setup for default framework
    default_framework_init(app);
    
    // NOTE(allen): default hooks and command maps
    set_all_default_hooks(app);
    mapping_init(tctx, &framework_mapping);
    
    String_ID global_map_id = vars_save_string_lit("keys_global");
    String_ID file_map_id = vars_save_string_lit("keys_file");
    String_ID code_map_id = vars_save_string_lit("keys_code");
    
    mapid_shared = vars_save_string_lit( "mapid_shared" );
    mapid_normal = vars_save_string_lit( "mapid_normal" );
    mapid_insert = vars_save_string_lit( "mapid_insert" );
    mapid_visual = vars_save_string_lit( "mapid_visual" );
    
    MappingScope( );
    SelectMapping( &framework_mapping );
    
    SelectMap( global_map_id );
    
    SelectMap( mapid_shared );
    BindCore( default_startup, CoreCode_Startup );
    BindCore( default_try_exit, CoreCode_TryExit );
    Bind( go_to_normal_mode, KeyCode_Escape );
    //Bind( move_left, KeyCode_Left );
    //Bind( move_right, KeyCode_Right );
    //Bind( move_up, KeyCode_Up );
    //Bind( move_down, KeyCode_Down );
    Bind( toggle_fullscreen,             KeyCode_Return, KeyCode_Alt);
    Bind( exit_4coder,                   KeyCode_F4, KeyCode_Alt);
    
    SelectMap( mapid_normal );
    ParentMap( mapid_shared );
    Bind( go_to_insert_mode,             KeyCode_I );
    Bind( go_to_visual_mode,             KeyCode_V );
    Bind( move_left,                     KeyCode_H );
    Bind( move_down,                     KeyCode_J );
    Bind( move_up,                       KeyCode_K );
    Bind( move_right,                    KeyCode_L );
    Bind( delete_char,                   KeyCode_X );
    Bind( backspace_char,                KeyCode_Backspace );
    Bind( delete_range,                  KeyCode_D );
    Bind( backspace_char,                KeyCode_Backspace );
    Bind( center_view,                   KeyCode_M );
    Bind( move_up_to_blank_line,         KeyCode_LeftBracket );
    Bind( comment_line_toggle,           KeyCode_ForwardSlash );
    Bind( search,                        KeyCode_Space );
    Bind( interactive_new,               KeyCode_N, KeyCode_Control);
    Bind( interactive_open_or_new,       KeyCode_O, KeyCode_Control);
    Bind( interactive_switch_buffer,     KeyCode_I, KeyCode_Control);
    Bind( move_left_whitespace_or_token_boundary, KeyCode_B);
    Bind( move_right_whitespace_or_token_boundary, KeyCode_W);
    Bind( move_left_whitespace_boundary, KeyCode_B, KeyCode_Shift);
    Bind( move_right_whitespace_boundary, KeyCode_W, KeyCode_Shift);
    Bind( undo, KeyCode_U );
    Bind( redo, KeyCode_U, KeyCode_Shift );
    
    SelectMap( mapid_insert );
    ParentMap( mapid_shared );
    BindTextInput( write_text_input );
    
    SelectMap( mapid_visual );
    
    /* This is to make sure that the default bindings on the buffers will be mapid_normal. */
    SelectMap( file_map_id );
    ParentMap( mapid_normal );
    
    SelectMap( code_map_id );
    ParentMap( mapid_normal );
}


