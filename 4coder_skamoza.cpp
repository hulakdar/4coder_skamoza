#pragma once

#include "4coder_default_include.cpp"

// #define VIM_MOUSE_SELECT_MODE             [Visual / VisualLine / VisualBlock]        [default: Visual]
// #define VIM_ESCAPE_SEQUENCE               [two character string]                     [default: "jk"]
// #define VIM_AUTO_LINE_COMMENTS            [0 / 1]                                    [default: 1]
// #define VIM_CASE_SENSITIVE_CHARACTER_SEEK [0 / 1]                                    [default: 1]
// #define VIM_AUTO_INDENT_ON_PASTE          [0 / 1]                                    [default: 1]
// #define VIM_DEFAULT_REGISTER              [unnamed_register / clipboard_register]    [default: unnamed_register]
// #define VIM_JUMP_HISTORY_SIZE             [integer]                                  [default: 100]
// #define VIM_MAXIMUM_OP_COUNT              [integer]                                  [default: 256]
// #define VIM_USE_CUSTOM_COLORS             [0 / 1]                                    [default: 1]
// #define VIM_USE_CHARACTER_SEEK_HIGHLIGHTS [0 / 1]                                    [default: 1]
// #define VIM_DRAW_PANEL_MARGINS            [0 / 1 / 2]                                [default: 2] (0: don't draw, 1: minimal vertical margins, 2: full margins)
// #define VIM_PANEL_MARGIN_THICKNESS        [float]                                    [default: 3.0f]

#define VIM_FILE_BAR_ON_BOTTOM 1
#define VIM_CURSOR_ROUNDNESS 0
#define VIM_USE_ECHO_BAR 1

#define q vim_q
#define wq vim_wq

#include "../4coder-vimmish/4coder_vimmish.cpp"

#undef q
#undef wq

CUSTOM_COMMAND_SIG(q)
CUSTOM_DOC("Kills current buffer instad of killing the pane, as 4coder_vimmish did.")
{
    View_ID view = get_active_view(app, Access_Always);
    Buffer_ID buffer = view_get_buffer(app, view, Access_Always);
    Scratch_Block scratch(app);
    String_Const_u8 file_name = push_buffer_file_name(app, scratch, buffer);
    
    if (file_name.size > 0
        && file_name.str[0] == '*'
        && file_name.str[file_name.size - 1] == '*')
    {
        // we don't want to kill it, but rather go somewhere else
        interactive_switch_buffer(app);
    }
    else
    {
        kill_buffer(app);
    }
    
}

CUSTOM_COMMAND_SIG(wq)
CUSTOM_DOC("Save buffer then kill it.")
{
    save(app);
    q(app);
}


// NOTE(allen): Users can declare their own managed IDs here.

#include "generated/managed_id_metadata.cpp"

VIM_OPERATOR(skamoza_vim_substitute)
{
    if (selection.kind == VimSelectionKind_None)
    {
        vim_delete_character(app, state, view, buffer, selection, count, count_was_set);
    }
    else
    {
        vim_change(app, state, view, buffer, selection, count, count_was_set);
    }
    vim_enter_insert_mode(app);
}

CUSTOM_COMMAND_SIG(open_file_in_remedy)
CUSTOM_DOC("Open current view in existing instance of remedy.")
{
    View_ID view = get_active_view(app, Access_Always);
    Buffer_ID buffer = view_get_buffer(app, view, Access_Always);
    
    Scratch_Block scratch(app);
    String_Const_u8 file_name = push_buffer_file_name(app, scratch.arena, buffer);
    
    String_Const_u8 hot = push_hot_directory(app, scratch);
    {
        u64 size = clamp_top(hot.size, sizeof(hot_directory_space));
        block_copy(hot_directory_space, hot.str, size);
        hot_directory_space[hot.size] = 0;
    }
    String_Const_u8 hot_directory = SCu8(hot_directory_space);
    
    i64 pos = view_get_cursor_pos(app, view);
    Buffer_Cursor cursor = view_compute_cursor(app, view, seek_pos(pos));
    
    exec_system_command(app,
                        view,
                        buffer_identifier(string_u8_litexpr("")),
                        hot_directory,
                        push_stringf(scratch.arena, "remedybg open-file %s %d", file_name.str, cursor.line),
                        0
                        );
}

// taken directly from 4code_sandvich_maker.cpp
CUSTOM_COMMAND_SIG(skamoza_startup)
{
    ProfileScope(app, "skamoza startup");
    
    User_Input input = get_current_input(app);
    if (match_core_code(&input, CoreCode_Startup))
    {
        String_Const_u8_Array file_names = input.event.core.file_names;
        load_themes_default_folder(app);
        default_4coder_initialize(app, file_names);
        default_4coder_side_by_side_panels(app, file_names);
        
        set_mouse_suppression(true);
        system_set_fullscreen(true);
        toggle_highlight_line_at_cursor(app);
        
        clear_all_layouts(app);
    }
}

void skamoza_setup_default_mapping(Application_Links* app, Mapping *mapping, Vim_Key vim_leader)
{
    MappingScope();
    SelectMapping(mapping);
    
    String_ID global_map_id = vars_save_string_lit("keys_global");
    String_ID file_map_id = vars_save_string_lit("keys_file");
    String_ID code_map_id = vars_save_string_lit("keys_code");
    
    //
    // Global Map
    //
    
    SelectMap(global_map_id);
    BindCore(skamoza_startup, CoreCode_Startup);
    
    BindCore(default_try_exit,         CoreCode_TryExit);
    Bind(project_go_to_root_directory, KeyCode_H,      KeyCode_Control);
    Bind(toggle_fullscreen,            KeyCode_Alt,    KeyCode_Return);
    Bind(save_all_dirty_buffers,       KeyCode_S,      KeyCode_Control, KeyCode_Shift);
    Bind(change_to_build_panel,        KeyCode_Period, KeyCode_Alt);
    Bind(close_build_panel,            KeyCode_Comma,  KeyCode_Alt);
    Bind(goto_next_jump,               KeyCode_N,      KeyCode_Control);
    Bind(goto_prev_jump,               KeyCode_P,      KeyCode_Control);
    Bind(build_in_build_panel,         KeyCode_M,      KeyCode_Alt);
    
    Bind(goto_first_jump,                   KeyCode_M, KeyCode_Alt, KeyCode_Shift);
    Bind(toggle_filebar,                    KeyCode_B, KeyCode_Alt);
    Bind(execute_any_cli,                   KeyCode_Z, KeyCode_Alt);
    Bind(execute_previous_cli,              KeyCode_Z, KeyCode_Alt, KeyCode_Shift);
    Bind(command_lister,                    KeyCode_X, KeyCode_Alt);
    Bind(project_command_lister,            KeyCode_X, KeyCode_Alt, KeyCode_Shift);
    Bind(list_all_functions_current_buffer, KeyCode_I, KeyCode_Control, KeyCode_Shift);
    Bind(project_fkey_command,              KeyCode_F1);
    Bind(project_fkey_command,              KeyCode_F2);
    Bind(project_fkey_command,              KeyCode_F3);
    Bind(project_fkey_command,              KeyCode_F4);
    Bind(project_fkey_command,              KeyCode_F5);
    Bind(project_fkey_command,              KeyCode_F6);
    Bind(project_fkey_command,              KeyCode_F7);
    Bind(project_fkey_command,              KeyCode_F8);
    Bind(project_fkey_command,              KeyCode_F9);
    Bind(project_fkey_command,              KeyCode_F10);
    Bind(project_fkey_command,              KeyCode_F11);
    Bind(project_fkey_command,              KeyCode_F12);
    Bind(project_fkey_command,              KeyCode_F13);
    Bind(project_fkey_command,              KeyCode_F14);
    Bind(project_fkey_command,              KeyCode_F15);
    Bind(project_fkey_command,              KeyCode_F16);
    Bind(exit_4coder,                       KeyCode_F4, KeyCode_Alt);
    BindMouseWheel(mouse_wheel_scroll);
    BindMouseWheel(mouse_wheel_change_face_size, KeyCode_Control);
    
    //
    // File Map
    //
    
    SelectMap(file_map_id);
    ParentMap(global_map_id);
    
    BindMouse(click_set_cursor_and_mark, MouseCode_Left);
    BindMouseRelease(click_set_cursor, MouseCode_Left);
    BindCore(click_set_cursor_and_mark, CoreCode_ClickActivateView);
    BindMouseMove(click_set_cursor_if_lbutton);
    
    BindTextInput(write_text_input);
    
    Bind(delete_char,                                 KeyCode_Delete);
    Bind(backspace_char,                              KeyCode_Backspace);
    Bind(move_up,                                     KeyCode_Up);
    Bind(move_down,                                   KeyCode_Down);
    Bind(move_left,                                   KeyCode_Left);
    Bind(move_right,                                  KeyCode_Right);
    Bind(seek_end_of_line,                            KeyCode_End);
    Bind(seek_beginning_of_line,                      KeyCode_Home);
    Bind(page_up,                                     KeyCode_PageUp);
    Bind(page_down,                                   KeyCode_PageDown);
    Bind(goto_beginning_of_file,                      KeyCode_PageUp, KeyCode_Control);
    Bind(goto_end_of_file,                            KeyCode_PageDown, KeyCode_Control);
    Bind(move_up_to_blank_line_end,                   KeyCode_Up, KeyCode_Control);
    Bind(move_down_to_blank_line_end,                 KeyCode_Down, KeyCode_Control);
    Bind(move_left_whitespace_boundary,               KeyCode_Left, KeyCode_Control);
    Bind(move_right_whitespace_boundary,              KeyCode_Right, KeyCode_Control);
    Bind(move_line_up,                                KeyCode_Up, KeyCode_Alt);
    Bind(move_line_down,                              KeyCode_Down, KeyCode_Alt);
    Bind(backspace_alpha_numeric_boundary,            KeyCode_Backspace, KeyCode_Control);
    Bind(delete_alpha_numeric_boundary,               KeyCode_Delete, KeyCode_Control);
    Bind(snipe_backward_whitespace_or_token_boundary, KeyCode_Backspace, KeyCode_Alt);
    Bind(snipe_forward_whitespace_or_token_boundary,  KeyCode_Delete, KeyCode_Alt);
    Bind(set_mark,                                    KeyCode_Space, KeyCode_Control);
    Bind(copy,                                        KeyCode_C, KeyCode_Control);
    Bind(delete_range,                                KeyCode_D, KeyCode_Control);
    Bind(delete_line,                                 KeyCode_D, KeyCode_Control, KeyCode_Shift);
    Bind(goto_line,                                   KeyCode_G, KeyCode_Control);
    Bind(paste_and_indent,                            KeyCode_V, KeyCode_Control);
    Bind(paste_next_and_indent,                       KeyCode_V, KeyCode_Control, KeyCode_Shift);
    Bind(cut,                                         KeyCode_X, KeyCode_Control);
    Bind(vim_redo,                                    KeyCode_Y, KeyCode_Control);
    Bind(vim_undo,                                    KeyCode_Z, KeyCode_Control);
    Bind(if_read_only_goto_position,                  KeyCode_Return);
    Bind(if_read_only_goto_position_same_panel,       KeyCode_Return, KeyCode_Shift);
    Bind(view_jump_list_with_lister,                  KeyCode_Period, KeyCode_Control, KeyCode_Shift);
    Bind(vim_enter_normal_mode_escape,                KeyCode_Escape);
    
    //
    // Code Map
    //
    
    SelectMap(code_map_id);
    ParentMap(file_map_id);
    
//    Bind(open_file_in_remedy, KeyCode_R);
    
    BindMouse(click_set_cursor_and_mark, MouseCode_Left);
    BindMouseRelease(click_set_cursor, MouseCode_Left);
    BindCore(click_set_cursor_and_mark, CoreCode_ClickActivateView);
    BindMouseMove(click_set_cursor_if_lbutton);
    
    BindTextInput(vim_write_text_abbrev_and_auto_indent);
    
    Bind(move_left_alpha_numeric_boundary,                    KeyCode_Left, KeyCode_Control);
    Bind(move_right_alpha_numeric_boundary,                   KeyCode_Right, KeyCode_Control);
    Bind(move_left_alpha_numeric_or_camel_boundary,           KeyCode_Left, KeyCode_Alt);
    Bind(move_right_alpha_numeric_or_camel_boundary,          KeyCode_Right, KeyCode_Alt);
    Bind(vim_backspace_char,                                  KeyCode_Backspace);
    Bind(comment_line_toggle,                                 KeyCode_Semicolon, KeyCode_Control);
    Bind(word_complete,                                       KeyCode_Tab);
    // Bind(word_complete_drop_down,                             KeyCode_N, KeyCode_Control);
    Bind(auto_indent_range,                                   KeyCode_Tab, KeyCode_Control);
    Bind(auto_indent_line_at_cursor,                          KeyCode_Tab, KeyCode_Shift);
    Bind(word_complete_drop_down,                             KeyCode_Tab, KeyCode_Shift, KeyCode_Control);
    Bind(write_block,                                         KeyCode_R, KeyCode_Alt);
    Bind(write_todo,                                          KeyCode_T, KeyCode_Alt);
    Bind(write_note,                                          KeyCode_Y, KeyCode_Alt);
    Bind(list_all_locations_of_type_definition,               KeyCode_D, KeyCode_Alt);
    Bind(list_all_locations_of_type_definition_of_identifier, KeyCode_T, KeyCode_Alt, KeyCode_Shift);
    Bind(open_long_braces,                                    KeyCode_LeftBracket, KeyCode_Control);
    Bind(open_long_braces_semicolon,                          KeyCode_LeftBracket, KeyCode_Control, KeyCode_Shift);
    Bind(open_long_braces_break,                              KeyCode_RightBracket, KeyCode_Control, KeyCode_Shift);
    Bind(select_surrounding_scope,                            KeyCode_LeftBracket, KeyCode_Alt);
    Bind(select_surrounding_scope_maximal,                    KeyCode_LeftBracket, KeyCode_Alt, KeyCode_Shift);
    Bind(select_prev_scope_absolute,                          KeyCode_RightBracket, KeyCode_Alt);
    Bind(select_prev_top_most_scope,                          KeyCode_RightBracket, KeyCode_Alt, KeyCode_Shift);
    Bind(select_next_scope_absolute,                          KeyCode_Quote, KeyCode_Alt);
    Bind(select_next_scope_after_current,                     KeyCode_Quote, KeyCode_Alt, KeyCode_Shift);
    Bind(place_in_scope,                                      KeyCode_ForwardSlash, KeyCode_Alt);
    Bind(delete_current_scope,                                KeyCode_Minus, KeyCode_Alt);
    Bind(if0_off,                                             KeyCode_I, KeyCode_Alt);
    Bind(open_file_in_quotes,                                 KeyCode_1, KeyCode_Alt);
    Bind(open_matching_file_cpp,                              KeyCode_2, KeyCode_Alt);
    Bind(write_zero_struct,                                   KeyCode_0, KeyCode_Control);
    
    //
    // Normal Map
    //
    
    SelectMap(vim_mapid_normal);
    ParentMap(global_map_id);
    
    BindMouse(vim_start_mouse_select, MouseCode_Left);
    BindMouseRelease(click_set_cursor, MouseCode_Left);
    BindCore(click_set_cursor_and_mark, CoreCode_ClickActivateView);
    BindMouseMove(vim_mouse_drag);
    
    //
    // Visual Map
    //
    
    SelectMap(vim_mapid_visual);
    ParentMap(vim_mapid_normal);
    
    //
    // ...
    //
    
    //
    // Vim Maps
    //
    
    VimMappingScope();
    
    //
    // Text Object Vim Map
    //
    
    VimSelectMap(vim_map_text_objects);
    
    VimBind(vim_text_object_inner_scope,                         vim_key(KeyCode_I), vim_key(KeyCode_LeftBracket, KeyCode_Shift));
    VimBind(vim_text_object_inner_scope,                         vim_key(KeyCode_I), vim_key(KeyCode_RightBracket, KeyCode_Shift));
    VimBind(vim_text_object_inner_paren,                         vim_key(KeyCode_I), vim_key(KeyCode_9, KeyCode_Shift));
    VimBind(vim_text_object_inner_paren,                         vim_key(KeyCode_I), vim_key(KeyCode_0, KeyCode_Shift));
    VimBind(vim_text_object_inner_single_quotes,                 vim_key(KeyCode_I), vim_key(KeyCode_Quote));
    VimBind(vim_text_object_inner_double_quotes,                 vim_key(KeyCode_I), vim_key(KeyCode_Quote, KeyCode_Shift));
    VimBind(vim_text_object_inner_word,                          vim_key(KeyCode_I), vim_key(KeyCode_W));
    VimBind(vim_text_object_isearch_repeat_forward,              vim_key(KeyCode_G), vim_key(KeyCode_N));
    VimBind(vim_text_object_isearch_repeat_backward,             vim_key(KeyCode_G), vim_key(KeyCode_N, KeyCode_Shift));
    
    //
    // Operator Pending Vim Map
    //
    
    VimSelectMap(vim_map_operator_pending);
    VimAddParentMap(vim_map_text_objects);
    
    VimBind(skamoza_vim_substitute,          vim_key(KeyCode_S));
    VimBind(move_up_to_blank_line_end,          vim_key(KeyCode_LeftBracket));
    VimBind(move_down_to_blank_line_end,          vim_key(KeyCode_RightBracket));
    
    VimBind(vim_motion_left,                                        vim_key(KeyCode_H));
    VimBind(vim_motion_down,                                        vim_key(KeyCode_J));
    VimBind(vim_motion_up,                                          vim_key(KeyCode_K));
    VimBind(vim_motion_right,                                       vim_key(KeyCode_L));
    VimBind(vim_motion_left,                                        vim_key(KeyCode_Left));
    VimBind(vim_motion_down,                                        vim_key(KeyCode_Down));
    VimBind(vim_motion_up,                                          vim_key(KeyCode_Up));
    VimBind(vim_motion_right,                                       vim_key(KeyCode_Right));
    VimBind(vim_motion_to_empty_line_down,                          vim_key(KeyCode_RightBracket, KeyCode_Shift));
    VimBind(vim_motion_to_empty_line_up,                            vim_key(KeyCode_LeftBracket,  KeyCode_Shift));
    VimBind(vim_motion_word,                                        vim_key(KeyCode_W));
    VimBind(vim_motion_big_word,                                    vim_key(KeyCode_W, KeyCode_Shift));
    VimBind(vim_motion_word_end,                                    vim_key(KeyCode_E));
    VimBind(vim_motion_word_backward,                               vim_key(KeyCode_B));
    VimBind(vim_motion_big_word_backward,                           vim_key(KeyCode_B, KeyCode_Shift));
    VimBind(vim_motion_line_start_textual,                          vim_key(KeyCode_0));
    VimBind(vim_motion_line_start_textual,                          vim_key(KeyCode_6, KeyCode_Shift));
    VimBind(vim_motion_line_end_textual,                            vim_key(KeyCode_4, KeyCode_Shift));
    VimBind(vim_motion_scope,                                       vim_key(KeyCode_5, KeyCode_Shift));
    VimBind(vim_motion_buffer_start_or_goto_line,                   vim_key(KeyCode_G),           vim_key(KeyCode_G));
    VimBind(vim_motion_buffer_end_or_goto_line,                     vim_key(KeyCode_G, KeyCode_Shift));
    VimBind(vim_motion_page_top,                                    vim_key(KeyCode_H, KeyCode_Shift));
    VimBind(vim_motion_page_mid,                                    vim_key(KeyCode_M, KeyCode_Shift));
    VimBind(vim_motion_page_bottom,                                 vim_key(KeyCode_L, KeyCode_Shift));
    VimBind(vim_motion_find_character_case_sensitive,               vim_key(KeyCode_F));
    VimBind(vim_motion_find_character_backward_case_sensitive,      vim_key(KeyCode_F, KeyCode_Shift));
    VimBind(vim_motion_to_character_case_sensitive,                 vim_key(KeyCode_T));
    VimBind(vim_motion_to_character_backward_case_sensitive,        vim_key(KeyCode_T, KeyCode_Shift));
//    VimBind(vim_motion_find_character_pair_backward_case_sensitive, vim_key(KeyCode_S, KeyCode_Shift));
    VimBind(vim_motion_repeat_character_seek_same_direction,        vim_key(KeyCode_Semicolon));
    VimBind(vim_motion_repeat_character_seek_reverse_direction,     vim_key(KeyCode_Comma));
    
    //
    // Normal Vim Map
    //
    
    VimSelectMap(vim_map_normal);
    VimAddParentMap(vim_map_operator_pending);
    
    VimBind(vim_register,                                        vim_key(KeyCode_Quote, KeyCode_Shift));
    VimBind(vim_change,                                          vim_key(KeyCode_C));
    VimBind(vim_change_eol,                                      vim_key(KeyCode_C, KeyCode_Shift));
    VimBind(vim_delete,                                          vim_key(KeyCode_D));
    VimBind(vim_delete_eol,                                      vim_key(KeyCode_D, KeyCode_Shift));
    VimBind(vim_delete_character,                                vim_key(KeyCode_X));
    VimBind(vim_yank,                                            vim_key(KeyCode_Y));
    VimBind(vim_yank_eol,                                        vim_key(KeyCode_Y, KeyCode_Shift));
    VimBind(vim_paste,                                           vim_key(KeyCode_P));
    VimBind(vim_paste_pre_cursor,                                vim_key(KeyCode_P, KeyCode_Shift));
    VimBind(vim_auto_indent,                                     vim_key(KeyCode_Equal));
    VimBind(vim_indent,                                          vim_key(KeyCode_Period, KeyCode_Shift));
    VimBind(vim_outdent,                                         vim_key(KeyCode_Comma, KeyCode_Shift));
    VimBind(vim_replace,                                         vim_key(KeyCode_R));
    VimBind(vim_new_line_below,                                  vim_key(KeyCode_O))->flags |= VimBindingFlag_TextCommand;
    VimBind(vim_new_line_above,                                  vim_key(KeyCode_O, KeyCode_Shift))->flags |= VimBindingFlag_TextCommand;
    VimBind(vim_join_line,                                       vim_key(KeyCode_J, KeyCode_Shift));
    VimBind(vim_align,                                           vim_key(KeyCode_G), vim_key(KeyCode_L));
    VimBind(vim_align_right,                                     vim_key(KeyCode_G), vim_key(KeyCode_L, KeyCode_Shift));
    VimBind(vim_align_string,                                    vim_key(KeyCode_G), vim_key(KeyCode_L, KeyCode_Control));
    VimBind(vim_align_string_right,                              vim_key(KeyCode_G), vim_key(KeyCode_L, KeyCode_Shift, KeyCode_Control));
    VimBind(vim_lowercase,                                       vim_key(KeyCode_G), vim_key(KeyCode_U));
    VimBind(vim_uppercase,                                       vim_key(KeyCode_G), vim_key(KeyCode_U, KeyCode_Shift));
    VimBind(vim_miblo_increment,                                 vim_key(KeyCode_A, KeyCode_Control));
    VimBind(vim_miblo_decrement,                                 vim_key(KeyCode_X, KeyCode_Control));
    VimBind(vim_miblo_increment_sequence,                        vim_key(KeyCode_G), vim_key(KeyCode_A, KeyCode_Control));
    VimBind(vim_miblo_decrement_sequence,                        vim_key(KeyCode_G), vim_key(KeyCode_X, KeyCode_Control));
    
    // @TODO: Doing that ->flags thing is a bit weird...
    VimBind(vim_enter_insert_mode,                               vim_key(KeyCode_I))->flags |= VimBindingFlag_TextCommand;
    VimBind(vim_enter_insert_sol_mode,                           vim_key(KeyCode_I, KeyCode_Shift))->flags |= VimBindingFlag_TextCommand;
    VimBind(vim_enter_append_mode,                               vim_key(KeyCode_A))->flags |= VimBindingFlag_TextCommand;
    VimBind(vim_enter_append_eol_mode,                           vim_key(KeyCode_A, KeyCode_Shift))->flags |= VimBindingFlag_TextCommand;
    VimBind(vim_toggle_visual_mode,                              vim_key(KeyCode_V));
    VimBind(vim_toggle_visual_line_mode,                         vim_key(KeyCode_V, KeyCode_Shift));
    VimBind(vim_toggle_visual_block_mode,                        vim_key(KeyCode_V, KeyCode_Control));
    
    VimBind(change_active_panel,                                 vim_key(KeyCode_W, KeyCode_Control), vim_key(KeyCode_W));
    VimBind(change_active_panel,                                 vim_key(KeyCode_W, KeyCode_Control), vim_key(KeyCode_W, KeyCode_Control));
    VimBind(swap_panels,                                         vim_key(KeyCode_W, KeyCode_Control), vim_key(KeyCode_X));
    VimBind(swap_panels,                                         vim_key(KeyCode_W, KeyCode_Control), vim_key(KeyCode_X, KeyCode_Control));
    VimBind(windmove_panel_left,                                 vim_key(KeyCode_W, KeyCode_Control), vim_key(KeyCode_H));
    VimBind(windmove_panel_left,                                 vim_key(KeyCode_W, KeyCode_Control), vim_key(KeyCode_H, KeyCode_Control));
    VimBind(windmove_panel_down,                                 vim_key(KeyCode_W, KeyCode_Control), vim_key(KeyCode_J));
    VimBind(windmove_panel_down,                                 vim_key(KeyCode_W, KeyCode_Control), vim_key(KeyCode_J, KeyCode_Control));
    VimBind(windmove_panel_up,                                   vim_key(KeyCode_W, KeyCode_Control), vim_key(KeyCode_K));
    VimBind(windmove_panel_up,                                   vim_key(KeyCode_W, KeyCode_Control), vim_key(KeyCode_K, KeyCode_Control));
    VimBind(windmove_panel_right,                                vim_key(KeyCode_W, KeyCode_Control), vim_key(KeyCode_L));
    VimBind(windmove_panel_right,                                vim_key(KeyCode_W, KeyCode_Control), vim_key(KeyCode_L, KeyCode_Control));
    VimBind(windmove_panel_swap_left,                            vim_key(KeyCode_W, KeyCode_Control), vim_key(KeyCode_H, KeyCode_Shift));
    VimBind(windmove_panel_swap_left,                            vim_key(KeyCode_W, KeyCode_Control), vim_key(KeyCode_H, KeyCode_Shift, KeyCode_Control));
    VimBind(windmove_panel_swap_down,                            vim_key(KeyCode_W, KeyCode_Control), vim_key(KeyCode_J, KeyCode_Shift));
    VimBind(windmove_panel_swap_down,                            vim_key(KeyCode_W, KeyCode_Control), vim_key(KeyCode_J, KeyCode_Shift, KeyCode_Control));
    VimBind(windmove_panel_swap_up,                              vim_key(KeyCode_W, KeyCode_Control), vim_key(KeyCode_K, KeyCode_Shift));
    VimBind(windmove_panel_swap_up,                              vim_key(KeyCode_W, KeyCode_Control), vim_key(KeyCode_K, KeyCode_Shift, KeyCode_Control));
    VimBind(windmove_panel_swap_right,                           vim_key(KeyCode_W, KeyCode_Control), vim_key(KeyCode_L, KeyCode_Shift));
    VimBind(windmove_panel_swap_right,                           vim_key(KeyCode_W, KeyCode_Control), vim_key(KeyCode_L, KeyCode_Shift, KeyCode_Control));
    VimBind(vim_split_window_vertical,                           vim_key(KeyCode_W, KeyCode_Control), vim_key(KeyCode_V));
    VimBind(vim_split_window_vertical,                           vim_key(KeyCode_W, KeyCode_Control), vim_key(KeyCode_V, KeyCode_Control));
    VimBind(vim_split_window_horizontal,                         vim_key(KeyCode_W, KeyCode_Control), vim_key(KeyCode_S));
    VimBind(vim_split_window_horizontal,                         vim_key(KeyCode_W, KeyCode_Control), vim_key(KeyCode_S, KeyCode_Control));
    
    VimBind(center_view,                                         vim_key(KeyCode_Z), vim_key(KeyCode_Z));
    VimBind(vim_view_move_line_to_top,                           vim_key(KeyCode_Z), vim_key(KeyCode_T));
    VimBind(vim_view_move_line_to_bottom,                        vim_key(KeyCode_Z), vim_key(KeyCode_B));
    
    VimBind(vim_page_up,                                         vim_key(KeyCode_B, KeyCode_Control));
    VimBind(vim_page_down,                                       vim_key(KeyCode_F, KeyCode_Control));
    VimBind(vim_half_page_up,                                    vim_key(KeyCode_U, KeyCode_Control));
    VimBind(vim_half_page_down,                                  vim_key(KeyCode_D, KeyCode_Control));
    
    VimBind(vim_step_back_jump_history,                          vim_key(KeyCode_O, KeyCode_Control));
    VimBind(vim_step_back_jump_history,                          vim_key(KeyCode_Minus));
    VimBind(vim_step_forward_jump_history,                       vim_key(KeyCode_I, KeyCode_Control));
    VimBind(vim_step_forward_jump_history,                       vim_key(KeyCode_Minus, KeyCode_Shift));
    VimBind(vim_previous_buffer,                                 vim_key(KeyCode_6, KeyCode_Control));
    
    VimBind(vim_record_macro,                                    vim_key(KeyCode_Q));
    VimBind(vim_replay_macro,                                    vim_key(KeyCode_2, KeyCode_Shift))->flags |= VimBindingFlag_TextCommand;
    VimBind(vim_set_mark,                                        vim_key(KeyCode_M));
    VimBind(vim_go_to_mark,                                      vim_key(KeyCode_Quote));
    VimBind(vim_go_to_mark,                                      vim_key(KeyCode_Tick));
    VimBind(vim_go_to_mark_less_history,                         vim_key(KeyCode_G), vim_key(KeyCode_Quote));
    VimBind(vim_go_to_mark_less_history,                         vim_key(KeyCode_G), vim_key(KeyCode_Tick));
    VimBind(vim_open_file_in_quotes_in_same_window,              vim_key(KeyCode_G), vim_key(KeyCode_F));
    VimBind(vim_jump_to_definition_under_cursor,                 vim_key(KeyCode_RightBracket, KeyCode_Control));
    
    VimNameBind(string_u8_litexpr("Files"),                      vim_leader, vim_key(KeyCode_F));
    VimBind(interactive_new,                                     vim_leader, vim_key(KeyCode_F), vim_key(KeyCode_N));
    VimBind(interactive_open_or_new,                             vim_leader, vim_key(KeyCode_F), vim_key(KeyCode_E));
    VimBind(interactive_switch_buffer,                           vim_leader, vim_key(KeyCode_F), vim_key(KeyCode_B));
    VimBind(interactive_kill_buffer,                             vim_leader, vim_key(KeyCode_F), vim_key(KeyCode_K));
    VimBind(kill_buffer,                                         vim_leader, vim_key(KeyCode_F), vim_key(KeyCode_D));
    VimBind(q,                                                   vim_leader, vim_key(KeyCode_F), vim_key(KeyCode_Q));
    VimBind(qa,                                                  vim_leader, vim_key(KeyCode_F), vim_key(KeyCode_Q, KeyCode_Shift));
    VimBind(qa,                                                  vim_leader, vim_key(KeyCode_F), vim_key(KeyCode_Q, KeyCode_Shift));
    VimBind(w,                                                   vim_leader, vim_key(KeyCode_F), vim_key(KeyCode_W));
    
    VimNameBind(string_u8_litexpr("Search"),                     vim_leader, vim_key(KeyCode_S));
    VimBind(list_all_substring_locations_case_insensitive,       vim_leader, vim_key(KeyCode_S), vim_key(KeyCode_S));
    
    VimNameBind(string_u8_litexpr("Tags"),                       vim_leader, vim_key(KeyCode_T));
    VimBind(jump_to_definition,                                  vim_leader, vim_key(KeyCode_T), vim_key(KeyCode_A));
    
    VimBind(vim_toggle_line_comment_range_indent_style,          vim_leader, vim_key(KeyCode_C), vim_key(KeyCode_Space));
    
    VimBind(vim_enter_normal_mode_escape,                        vim_key(KeyCode_Escape));
    VimBind(vim_isearch_word_under_cursor,                       vim_key(KeyCode_8, KeyCode_Shift));
    VimBind(vim_reverse_isearch_word_under_cursor,               vim_key(KeyCode_3, KeyCode_Shift));
    VimBind(vim_repeat_command,                                  vim_key(KeyCode_Period));
    VimBind(vim_move_line_up,                                    vim_key(KeyCode_K, KeyCode_Alt));
    VimBind(vim_move_line_down,                                  vim_key(KeyCode_J, KeyCode_Alt));
    VimBind(vim_isearch,                                         vim_key(KeyCode_Space));
    VimBind(vim_isearch_backward,                                vim_key(KeyCode_Space, KeyCode_Shift));
    VimBind(vim_isearch_repeat_forward,                          vim_key(KeyCode_N));
    VimBind(vim_isearch_repeat_backward,                         vim_key(KeyCode_N, KeyCode_Shift));
    VimBind(noh,                                                 vim_leader, vim_key(KeyCode_N));
    VimBind(goto_next_jump,                                      vim_key(KeyCode_N, KeyCode_Control));
    VimBind(goto_prev_jump,                                      vim_key(KeyCode_P, KeyCode_Control));
    VimBind(vim_undo,                                            vim_key(KeyCode_U));
    VimBind(vim_redo,                                            vim_key(KeyCode_R, KeyCode_Control));
    VimBind(command_lister,                                      vim_key(KeyCode_Semicolon, KeyCode_Shift));
    VimBind(if_read_only_goto_position,                          vim_key(KeyCode_Return));
    VimBind(if_read_only_goto_position_same_panel,               vim_key(KeyCode_Return, KeyCode_Shift));
    
    //
    // Visual Vim Map
    //
    
    VimSelectMap(vim_map_visual);
    VimAddParentMap(vim_map_normal);
    VimAddParentMap(vim_map_text_objects);
    
    VimBind(vim_lowercase,                                       vim_key(KeyCode_U));
    VimBind(vim_uppercase,                                       vim_key(KeyCode_U, KeyCode_Shift));
    
    VimBind(vim_isearch_selection,                               vim_key(KeyCode_Space));
    VimBind(vim_reverse_isearch_selection,                       vim_key(KeyCode_Space, KeyCode_Shift));
}

void custom_layer_init(Application_Links *app){
    Thread_Context *tctx = get_thread_context(app);
    //
    // Base 4coder Initialization
    //
    
    default_framework_init(app);
    set_all_default_hooks(app);
    mapping_init(tctx, &framework_mapping);
    
    //
    // Vim Layer Initialization
    //
    
    vim_init(app);
    vim_set_default_hooks(app);
    skamoza_setup_default_mapping(app, &framework_mapping, vim_key(KeyCode_ForwardSlash));
}

