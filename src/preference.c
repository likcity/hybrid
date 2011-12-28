/***************************************************************************
 *   Copyright (C) 2011 by levin                                           *
 *   levin108@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.            *
 ***************************************************************************/

#include "pref.h"
#include "util.h"

#include "conv.h"
#include "preference.h"
#include "gtkutils.h"

static HybridPrefWin *main_pref_window = NULL;

guint bool_pref_add_entry(GtkWidget *section, guint pos,
                          HybridPrefEntry *entry);
void bool_pref_save(HybridPrefEntry *entry);
void bool_pref_destroy(HybridPrefEntry *entry);

static PrefAddFuncs bool_add_funcs = {
    .add_entry = bool_pref_add_entry,
    .save = bool_pref_save,
    .destroy = bool_pref_destroy
};

guint string_pref_add_entry(GtkWidget *section, guint pos,
                            HybridPrefEntry *entry);
void string_pref_save(HybridPrefEntry *entry);
void string_pref_destroy(HybridPrefEntry *entry);

static PrefAddFuncs string_add_funcs = {
    .add_entry = string_pref_add_entry,
    .save = string_pref_save,
    .destroy = string_pref_destroy
};

guint int_pref_add_entry(GtkWidget *section, guint pos, HybridPrefEntry *entry);
void int_pref_save(HybridPrefEntry *entry);
void int_pref_destroy(HybridPrefEntry *entry);

static PrefAddFuncs int_add_funcs = {
    .add_entry = int_pref_add_entry,
    .save = int_pref_save,
    .destroy = int_pref_destroy
};

guint select_pref_add_entry(GtkWidget *section, guint pos,
                            HybridPrefEntry *entry);
void select_pref_save(HybridPrefEntry *entry);
void select_pref_destroy(HybridPrefEntry *entry);

static PrefAddFuncs select_add_funcs = {
    .add_entry = select_pref_add_entry,
    .save = select_pref_save,
    .destroy = select_pref_destroy
};

static PrefAddFuncs *pref_types[] = {
    [PREF_KEY_NONE] = NULL,
    [PREF_KEY_BOOL] = &bool_add_funcs,
    [PREF_KEY_STRING] = &string_add_funcs,
    [PREF_KEY_INT] = &int_add_funcs,
    [PREF_KEY_SELECT] = &select_add_funcs
};

guint
bool_pref_add_entry(GtkWidget *section, guint pos, HybridPrefEntry *entry)
{
    GtkWidget *checkbutton;

    checkbutton = gtk_check_button_new_with_label(entry->name);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton),
                                 hybrid_pref_get_boolean(entry->win->pref,
                                                         entry->key));
    if (entry->tooltip)
        gtk_widget_set_tooltip_markup(checkbutton, entry->tooltip);
    entry->data = checkbutton;

    gtk_table_attach_defaults(GTK_TABLE(section), checkbutton, 0, 2,
                             pos, pos + 1);

    return 1;
}

void
bool_pref_save(HybridPrefEntry *entry)
{
    hybrid_pref_set_boolean(entry->win->pref, entry->key,
                            gtk_toggle_button_get_active(
                                GTK_TOGGLE_BUTTON(entry->data)));
}

void
bool_pref_destroy(HybridPrefEntry *entry)
{
    return;
}

guint
string_pref_add_entry(GtkWidget *section, guint pos, HybridPrefEntry *entry)
{
    GtkWidget *text;
    GtkWidget *label;
    gchar *value;

    label = gtk_label_new(entry->name);
    if (entry->tooltip)
        gtk_widget_set_tooltip_markup(label, entry->tooltip);
    text = gtk_entry_new();

    value = hybrid_pref_get_string(entry->win->pref, entry->key);
    if (value) {
        gtk_entry_set_text(GTK_ENTRY(text), value);
        g_free(value);
    }

    entry->data = text;

    gtk_table_attach_defaults(GTK_TABLE(section), label, 0, 1, pos, pos + 1);
    gtk_table_attach_defaults(GTK_TABLE(section), text, 1, 2, pos, pos + 1);
    return 1;
}

void
string_pref_save(HybridPrefEntry *entry)
{
    hybrid_pref_set_string(entry->win->pref, entry->key,
                           gtk_entry_get_text(GTK_ENTRY(entry->data)));
}

void
string_pref_destroy(HybridPrefEntry *entry)
{
    return;
}

guint
int_pref_add_entry(GtkWidget *section, guint pos, HybridPrefEntry *entry)
{
    GtkWidget *number;
    GtkWidget *label;
    gint value;
    IntRange *range;
    gint upper;
    gint lower;
    gint step;

    label = gtk_label_new(entry->name);
    if (entry->tooltip)
        gtk_widget_set_tooltip_markup(label, entry->tooltip);

    if ((range = entry->data)) {
        upper = range->upper;
        lower = range->lower;
        step = range->step;
    } else {
        /* The largest range I can think of now is the number of port. */
        upper = 65535;
        lower = 0;
        step = 1;
    }
    number = gtk_spin_button_new_with_range(lower, upper, step);

    value = hybrid_pref_get_int(entry->win->pref, entry->key);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(number), value);

    entry->data = number;

    gtk_table_attach_defaults(GTK_TABLE(section), label, 0, 1, pos, pos + 1);
    gtk_table_attach_defaults(GTK_TABLE(section), number, 1, 2, pos, pos + 1);
    return 1;
}

void
int_pref_save(HybridPrefEntry *entry)
{
    hybrid_pref_set_int(entry->win->pref, entry->key,
                        gtk_spin_button_get_value_as_int(
                            GTK_SPIN_BUTTON(entry->data)));
}

void
int_pref_destroy(HybridPrefEntry *entry)
{
    return;
}

guint
select_pref_add_entry(GtkWidget *section, guint pos, HybridPrefEntry *entry)
{
    GtkWidget *combo;
    GtkWidget *label;
    gchar *value;
    gint i;
    gint active = -1;
    SelectOption *options = entry->data;
    gpointer *data;

    label = gtk_label_new(entry->name);
    if (entry->tooltip)
        gtk_widget_set_tooltip_markup(label, entry->tooltip);

    combo = gtk_combo_box_text_new();
    value = hybrid_pref_get_string(entry->win->pref, entry->key);
    for (i = 0;options[i].name;i++) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo),
                                       options[i].name);
        if (!g_strcmp0(options[i].value, value)) {
            active = i;
        }
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), active);

    entry->data = data = g_new0(gpointer, i + 2);
    data[0] = data + i + 1;
    data[1] = combo;
    for (i++;i > 1;i--) {
        data[i] = g_strdup(options[i - 2].value);
    }

    gtk_table_attach_defaults(GTK_TABLE(section), label, 0, 1, pos, pos + 1);
    gtk_table_attach_defaults(GTK_TABLE(section), combo, 1, 2, pos, pos + 1);
    if (value)
        g_free(value);
    return 1;
}

void
select_pref_save(HybridPrefEntry *entry)
{
    gpointer *data = entry->data;
    gint active = gtk_combo_box_get_active(GTK_COMBO_BOX(data[1]));
    if (active < 0 || (gpointer)(active + data + 2) > data[0]) {
        hybrid_debug_error("select_save", "active id out of range.");
        return;
    }
    hybrid_pref_set_string(entry->win->pref, entry->key,
                           (gchar*)data[active + 2]);
}

void
select_pref_destroy(HybridPrefEntry *entry)
{
    gpointer *p;
    gpointer *data = entry->data;
    for (p = (gpointer*)data[0];p > (data + 1);p--) {
        g_free(*p);
    }
    g_free(data);
    return;
}

GtkWidget*
hybrid_pref_win_add_tab(HybridPrefWin *pref_win, const gchar *name)
{
    GtkWidget *page;
    GtkWidget *label;

    page = gtk_vbox_new(FALSE, 0);
    label = gtk_label_new(name);
    gtk_notebook_append_page(GTK_NOTEBOOK(pref_win->notebook),
                             page, label);

    return page;
}

GtkWidget*
hybrid_pref_tab_add_section(GtkWidget *tab, const gchar *name)
{
    GtkWidget *child;
    GtkWidget *frame;

    frame = gtk_frame_new(name ? name : "");
    child = gtk_table_new(1, 2, FALSE);
    gtk_container_add(GTK_CONTAINER(frame), child);
    gtk_container_add(GTK_CONTAINER(tab), frame);

    return child;
}

static void
entry_destroy_cb(HybridPrefEntry *entry)
{
    if (entry->type && entry->type->destroy)
        entry->type->destroy(entry);
    if (entry->name)
        g_free(entry->name);
    if (entry->key)
        g_free(entry->key);
    if (entry->tooltip)
        g_free(entry->tooltip);
    g_free(entry);
}

static void
entry_response_cb(GtkDialog *dialog, gint response_id, HybridPrefEntry *entry)
{
    fprintf(stderr, "callback\n");
    if (response_id == GTK_RESPONSE_ACCEPT)
        entry->type->save(entry);
}

void
hybrid_pref_section_add_entry(HybridPrefWin *pref_win, GtkWidget *section,
                              PrefKeyType type, gchar *name, gchar *key,
                              gchar *tooltip, gpointer data)
{
    PrefAddFuncs *funcs;
    HybridPrefEntry *entry;
    guint width;
    guint height;
    guint delta;
    guint tmp;

    if (type >= PREF_KEY_MAX || type < PREF_KEY_NONE) {
        hybrid_debug_error("pref_add_entry", "Unknown type %d", type);
        return;
    }

    if (type == PREF_KEY_NONE) {
        hybrid_debug_error("pref_add_entry",
                           "Doesn't support custom type yet.");
        return;
    }

    /* Dosen't make sence for these two field to be NULL. */
    if (!(name || key)) {
        hybrid_debug_error("pref_add_entry", "name or key is NULL.");
        return;
    }

    funcs = pref_types[type];

    entry = g_new0(HybridPrefEntry, 1);
    entry->name = g_strdup(name);
    entry->key = g_strdup(key);
    entry->tooltip = tooltip ? g_strdup(tooltip) : NULL;
    entry->data = data;
    entry->type = funcs;
    entry->win = pref_win;

    gtk_table_get_size(GTK_TABLE(section), &height, &width);
    delta = funcs->add_entry(section, height - 1, entry);
    if (!delta) {
        entry->type = NULL;
        entry_destroy_cb(entry);
        hybrid_debug_error("pref_add_entry", "Cannot add entry.");
        return;
    }
    gtk_table_get_size(GTK_TABLE(section), &tmp, &width);
    gtk_table_resize(GTK_TABLE(section), height + delta, width);

    g_signal_connect(pref_win->window, "response",
                     G_CALLBACK(entry_response_cb), entry);
    g_signal_connect_swapped(section, "destroy",
                             G_CALLBACK(entry_destroy_cb), entry);
}

void hybrid_pref_win_finish(HybridPrefWin *pref_win)
{
    gtk_widget_show_all(pref_win->window);
}

static GtkTreeModel*
create_tab_pos_model(void)
{
    return NULL;
}

/**
 * Create the combo box for choosing the tab positons.
 */
static GtkWidget*
chat_theme_combo_create()
{
    /* GtkWidget *combo; */
    /* GtkTreeStore *store; */
    /* GtkTreeIter iter; */
    /* GtkCellRenderer *renderer; */
    /* HybridChatTheme *themes; */
    /* gchar *chat_theme_name = NULL; */
    /* gchar *name = NULL; */
    /* gint i; */

    /* store = gtk_tree_store_new(TAB_POS_COLS, */
    /*             G_TYPE_STRING, */
    /*             G_TYPE_INT); */

    /* themes = hybrid_chat_window_get_themes(); */

    /* for (i = 0; themes[i].name; i++) { */
    /*     gtk_tree_store_append(store, &iter, NULL); */
    /*     gtk_tree_store_set(store, &iter, TAB_POS_NAME_COL, themes[i].name, -1); */
    /* } */

    /* combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store)); */

    /* renderer = gtk_cell_renderer_text_new(); */
    /* gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo), renderer, FALSE); */
    /* gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combo), renderer, */
    /*                                "text", TAB_POS_NAME_COL, NULL); */
    /* gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0); */

    /* /\* */
    /*  * Set the active item with the local data. */
    /*  *\/ */
    /* if ((chat_theme_name = hybrid_pref_get_string(NULL, "chat_theme"))) { */
    /*     if(gtk_tree_model_get_iter_root(GTK_TREE_MODEL(store), &iter)) { */
    /*         do { */
    /*             gtk_tree_model_get(GTK_TREE_MODEL(store), &iter, */
    /*                     TAB_POS_NAME_COL, &name, -1); */

    /*             if (g_strcmp0(name, chat_theme_name) == 0) { */

    /*                 gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo), &iter); */

    /*                 break; */
    /*             } */

    /*         } while (gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter)); */
    /*     } */
    /* } */

    /* g_free(chat_theme_name); */
    /* g_object_unref(store); */

    /* return combo; */
}

static GtkWidget*
tab_pos_combo_create()
{
    /* GtkWidget *combo; */
    /* GtkTreeModel *model; */
    /* GtkCellRenderer *renderer; */
    /* gint value; */
    /* gint tab_pos; */

    /* model = create_tab_pos_model(); */
    /* combo = gtk_combo_box_new_with_model(model); */

    /* renderer = gtk_cell_renderer_text_new(); */
    /* gtk_cell_layout_pack_start( */
    /*         GTK_CELL_LAYOUT(combo), renderer, FALSE); */
    /* gtk_cell_layout_set_attributes( */
    /*         GTK_CELL_LAYOUT(combo), renderer, */
    /*         "text", TAB_POS_NAME_COL, NULL); */
    /* gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0); */

    /* /\* */
    /*  * Set the active item with the local data. */
    /*  *\/ */
    /* if ((tab_pos = hybrid_pref_get_int(NULL, "tab_pos")) != -1) { */

    /*     if(gtk_tree_model_get_iter_root(model, &iter)) { */

    /*         do { */
    /*             gtk_tree_model_get(model, &iter, TAB_POS_VALUE_COL, &value, -1); */

    /*             if (value == tab_pos) { */

    /*                 gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo), &iter); */

    /*                 break; */
    /*             } */

    /*         } while (gtk_tree_model_iter_next(model, &iter)); */
    /*     } */
    /* } */

    /* g_object_unref(model); */

    /* return combo; */
}

/**
 * Initialize the basic settings page.
 */
void
pref_basic_init(GtkWidget *tab)
{
    GtkWidget *section = hybrid_pref_tab_add_section(tab, _("Chat Window"));
    int i, j;

    hybrid_pref_section_add_entry(main_pref_window, section, PREF_KEY_BOOL,
                                  _("Hide Action Buttons"), "hide_chat_buttons",
                                  _("Hide Action Buttons"), NULL);

    HybridChatTheme *themes = hybrid_chat_window_get_themes();

    for (i = 0;themes[i].name;i++);

    SelectOption *options = g_new0(SelectOption, i+1);

    for (j = 0;j < i;j++) {
        options[j].name = options[j].value = themes[j].name;
    }

    hybrid_pref_section_add_entry(main_pref_window, section, PREF_KEY_SELECT,
                                  _("Chat Theme:"), "chat_theme",
                                  _("Chat Theme:"), options);
    g_free(options);

    section = hybrid_pref_tab_add_section(tab, _("Tabs"));
    hybrid_pref_section_add_entry(
        main_pref_window, section, PREF_KEY_BOOL,
        _("Show Messages In A Single Window With Tabs"), "single_chat_window",
        _("Show Messages In A Single Window With Tabs"), NULL);

    /* label = gtk_label_new(_("Tab Position:")); */
    /* gtk_fixed_put(fixed, label, 20, 165); */

    /* pref_window->tab_pos_combo = tab_pos_combo_create(); */
    /* gtk_fixed_put(fixed, pref_window->tab_pos_combo, 120, 160); */
}

/**
 * Initialize the sound settings panel.
 */
static void
pref_sound_init(GtkWidget *tab)
{
    GtkWidget *section = hybrid_pref_tab_add_section(tab, NULL);

    hybrid_pref_section_add_entry(main_pref_window, section, PREF_KEY_BOOL,
                                  _("Mute"), "mute", _("Mute"), NULL);
}

/**
 * Callback function for the save button.
 */
static void
save_cb(GtkWidget *widget, gpointer user_data)
{
    /* GtkTreeModel *model; */
    /* GtkTreeIter iter; */
    /* gint tab_pos; */
    /* gchar *chat_theme; */

    /* g_return_if_fail(pref_window != NULL); */

    /* if (gtk_toggle_button_get_active( */
    /*             GTK_TOGGLE_BUTTON(pref_window->mute_check))) { */

    /*     hybrid_pref_set_boolean(NULL, "mute", TRUE); */

    /* } else { */
    /*     hybrid_pref_set_boolean(NULL, "mute", FALSE); */
    /* } */

    /* if (gtk_toggle_button_get_active( */
    /*             GTK_TOGGLE_BUTTON(pref_window->hcb_check))) { */

    /*     hybrid_pref_set_boolean(NULL, "hide_chat_buttons", TRUE); */

    /* } else { */
    /*     hybrid_pref_set_boolean(NULL, "hide_chat_buttons", FALSE); */
    /* } */

    /* if (gtk_toggle_button_get_active( */
    /*             GTK_TOGGLE_BUTTON(pref_window->single_cw_check))) { */
    /*     hybrid_pref_set_boolean(NULL, "single_chat_window", TRUE); */
    /* } else { */
    /*     hybrid_pref_set_boolean(NULL, "single_chat_window", FALSE); */
    /* } */

    /* /\* */
    /*  * Tab position settings. */
    /*  *\/ */
    /* model = gtk_combo_box_get_model(GTK_COMBO_BOX(pref_window->tab_pos_combo)); */

    /* gtk_combo_box_get_active_iter( */
    /*         GTK_COMBO_BOX(pref_window->tab_pos_combo), &iter); */

    /* gtk_tree_model_get(model, &iter, TAB_POS_VALUE_COL, &tab_pos, -1); */

    /* hybrid_pref_set_int(NULL, "tab_pos", tab_pos); */

    /* /\* */
    /*  * Chat theme settings. */
    /*  *\/ */
    /* model = gtk_combo_box_get_model(GTK_COMBO_BOX(pref_window->chat_theme_combo)); */

    /* gtk_combo_box_get_active_iter( */
    /*         GTK_COMBO_BOX(pref_window->chat_theme_combo), &iter); */

    /* gtk_tree_model_get(model, &iter, TAB_POS_NAME_COL, &chat_theme, -1); */

    /* hybrid_pref_set_string(NULL, "chat_theme", chat_theme); */

    /* hybrid_pref_save(NULL); */

    /* gtk_widget_destroy(pref_window->window); */
}

/**
 * Initialize the preference window.
 */
void
pref_window_init(void)
{
    /* GtkWidget *fixed; */
    /* GtkWidget *label; */
    /* GtkWidget *vbox; */
    /* GtkWidget *action_area; */
    /* GtkWidget *button; */

    /* g_return_if_fail(pref_window != NULL); */

    /* label = gtk_label_new(_("Basic Settings")); */

    /* pref_basic_init(GTK_FIXED(fixed)); */

    /* fixed = gtk_fixed_new(); */
    /* label = gtk_label_new(_("Sound")); */
    /* gtk_notebook_append_page(GTK_NOTEBOOK(pref_window->notebook), */
    /*                         fixed, label); */

    /* pref_sound_init(GTK_FIXED(fixed)); */

    /* action_area = gtk_hbox_new(FALSE, 0); */
    /* gtk_box_pack_start(GTK_BOX(vbox), action_area, FALSE, FALSE, 5); */
}

/**
 * Callback function for destroying the preference window.
 */
static void
destroy_cb(GtkWidget *widget, gpointer pref_win)
{
    g_free(pref_win);
}

static void
main_destroy_cb(GtkWidget *widget, gpointer p)
{
    main_pref_window = NULL;
}

HybridPrefWin*
hybrid_pref_win_new(HybridPref *pref, const gchar *title)
{
    HybridPrefWin *pref_win;
    GtkWidget *content_area;
    GdkPixbuf *icon;
    title = title ? title : _("Preference");
    pref_win = g_new0(HybridPrefWin, 1);

    pref_win->pref = pref;
    /* Use dialog window in order to be tiling wm/filter friendly. */
    /* TODO put this into hybrid_create_window */
    pref_win->window = gtk_dialog_new_with_buttons(title, NULL, 0,
                                                   GTK_STOCK_OK,
                                                   GTK_RESPONSE_ACCEPT,
                                                   GTK_STOCK_CANCEL,
                                                   GTK_RESPONSE_REJECT,
                                                   NULL);
    icon = hybrid_create_default_icon(0);
    gtk_window_set_icon(GTK_WINDOW(pref_win->window), icon);
    gtk_window_set_resizable(GTK_WINDOW(pref_win->window), FALSE);
    gtk_window_set_position(GTK_WINDOW(pref_win->window), GTK_WIN_POS_CENTER);

    /* doesn't set the pointer to NULL, need to be handled elsewhere */
    /* by connect_after to the same signal? */
    g_signal_connect(pref_win->window, "destroy",
                     G_CALLBACK(destroy_cb), pref_win);
    /* g_signal_connect_swapped(pref_win->window, "response", */
    /*                          G_CALLBACK(gtk_widget_destroy), pref_win->window); */

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(pref_win->window));

    /* let's see how it looks */
    //gtk_container_set_border_width(GTK_CONTAINER(content_area), 8);

    pref_win->notebook = gtk_notebook_new();
    gtk_container_add(GTK_CONTAINER(content_area), pref_win->notebook);

    gtk_notebook_set_tab_pos(GTK_NOTEBOOK(pref_win->notebook), GTK_POS_TOP);
    gtk_notebook_set_scrollable(GTK_NOTEBOOK(pref_win->notebook), TRUE);
    gtk_notebook_set_show_border(GTK_NOTEBOOK(pref_win->notebook), TRUE);

    return pref_win;
}

void
hybrid_pref_create(void)
{
    GtkWidget *tab;
    if (main_pref_window) {
        gtk_window_present(GTK_WINDOW(main_pref_window->window));
        return;
    }

    main_pref_window = hybrid_pref_win_new(NULL, NULL);

    g_signal_connect_after(main_pref_window->window, "destroy",
                           G_CALLBACK(main_destroy_cb), NULL);

    tab = hybrid_pref_win_add_tab(main_pref_window, _("Basic Settings"));
    pref_basic_init(tab);

    tab = hybrid_pref_win_add_tab(main_pref_window, _("Sound"));
    pref_sound_init(tab);

    /* gtk_widget_set_size_request(main_pref_window->window, 450, 300); */

    hybrid_pref_win_finish(main_pref_window);
}
