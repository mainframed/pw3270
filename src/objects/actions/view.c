/*
 * "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
 * (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
 * aplicativos mainframe. Registro no INPI sob o nome G3270.
 *
 * Copyright (C) <2008> <Banco do Brasil S.A.>
 *
 * Este programa é software livre. Você pode redistribuí-lo e/ou modificá-lo sob
 * os termos da GPL v.2 - Licença Pública Geral  GNU,  conforme  publicado  pela
 * Free Software Foundation.
 *
 * Este programa é distribuído na expectativa de  ser  útil,  mas  SEM  QUALQUER
 * GARANTIA; sem mesmo a garantia implícita de COMERCIALIZAÇÃO ou  de  ADEQUAÇÃO
 * A QUALQUER PROPÓSITO EM PARTICULAR. Consulte a Licença Pública Geral GNU para
 * obter mais detalhes.
 *
 * Você deve ter recebido uma cópia da Licença Pública Geral GNU junto com este
 * programa; se não, escreva para a Free Software Foundation, Inc., 51 Franklin
 * St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Este programa está nomeado como - e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

/**
 * @brief Implement PW3270 Action view widget and related tools.
 *
 */

#include <config.h>
#include <pw3270.h>
#include <pw3270/actions.h>
#include <lib3270/log.h>

enum {
	COLUMN_PIXBUF,
	COLUMN_LABEL,
	COLUMN_ACTION_NAME,
	COLUMN_FLAGS
};

struct ListElement {
// 	GAction		* action;
	GdkPixbuf	* pixbuf;
	gchar		* action_name;
	gchar		* action_label;
};

static void list_element_free(struct ListElement *element);

static gint view_sort(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer G_GNUC_UNUSED(user_data)) {

	gint rc = 0;
	GValue value[] = { G_VALUE_INIT, G_VALUE_INIT };

	gtk_tree_model_get_value(model, a, COLUMN_LABEL, &value[0]);
	gtk_tree_model_get_value(model, b, COLUMN_LABEL, &value[1]);

	rc = g_ascii_strcasecmp(g_value_get_string(&value[0]),g_value_get_string(&value[1]));

	g_value_unset(&value[0]);
	g_value_unset(&value[1]);

	return rc;

}

GtkWidget * pw3270_action_view_new() {

	GtkTreeModel * model = GTK_TREE_MODEL(gtk_list_store_new(4,G_TYPE_OBJECT,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_INT));
	GtkWidget * view = GTK_WIDGET(gtk_tree_view_new_with_model(model));

	gtk_widget_set_hexpand(view,TRUE);
	gtk_widget_set_vexpand(view,TRUE);
	gtk_tree_view_set_fixed_height_mode(GTK_TREE_VIEW(view),FALSE);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view),FALSE);

	// Create Renderers
	GtkCellRenderer * text_renderer = gtk_cell_renderer_text_new();
	GtkCellRenderer * pixbuf_renderer = gtk_cell_renderer_pixbuf_new();

	gtk_tree_view_insert_column_with_attributes(
	    GTK_TREE_VIEW(view),
	    -1,
	    _("Icon"),
	    pixbuf_renderer,
	    "pixbuf",COLUMN_PIXBUF,
	    NULL
	);

	gtk_tree_view_insert_column_with_attributes(
	    GTK_TREE_VIEW(view),
	    -1,
	    _("Label"),
	    text_renderer,
	    "text",COLUMN_LABEL,
	    NULL
	);

	gtk_tree_sortable_set_default_sort_func(GTK_TREE_SORTABLE(model),view_sort,NULL,NULL);

	return view;
}

void pw3270_action_view_append(GtkWidget *widget, const gchar *label, GdkPixbuf *pixbuf, const gchar *action_name, PW3270ActionViewFlag flags) {

	GtkListStore * store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)));

	GtkTreeIter iter;
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(
	    store,
	    &iter,
	    COLUMN_PIXBUF,			pixbuf,
	    COLUMN_LABEL, 			label,
	    COLUMN_ACTION_NAME,		action_name,
	    COLUMN_FLAGS,			(gint) flags,
	    -1
	);


}

void pw3270_action_view_order_by_label(GtkWidget *view) {

	gtk_tree_view_set_reorderable(GTK_TREE_VIEW(view),FALSE);

	gtk_tree_sortable_set_sort_column_id(
	    GTK_TREE_SORTABLE(gtk_tree_view_get_model(GTK_TREE_VIEW(view))),
	    COLUMN_LABEL,
	    GTK_SORT_ASCENDING
	);
}

static void pw3270_action_view_append_element(GtkListStore * store, struct ListElement * element) {

	GtkTreeIter iter;
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(
	    store,
	    &iter,
	    COLUMN_PIXBUF,		element->pixbuf,
	    COLUMN_LABEL, 		element->action_label,
	    COLUMN_ACTION_NAME,	element->action_name,
	    COLUMN_FLAGS,		(gint) PW3270_ACTION_VIEW_ALLOW_MOVE,
	    -1
	);

}

Pw3270ActionList * pw3270_action_list_move_action(Pw3270ActionList *action_list, const gchar *action_name, GtkWidget *view) {

	GSList * item = (GSList *) action_list;
	GtkListStore * store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(view)));

	while(item) {

		struct ListElement * element = (struct ListElement *) item->data;
		if(!g_ascii_strcasecmp(action_name,element->action_name)) {

			pw3270_action_view_append_element(store, element);
			list_element_free(element);
			return (Pw3270ActionList *) g_slist_remove_link(action_list,item);
		}

		item = g_slist_next(item);

	}

	return action_list;

}

void pw3270_action_view_set_actions(GtkWidget *view, Pw3270ActionList *list) {

	GSList *item;
	GtkListStore * store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(view)));

	for(item = (GSList *) list; item; item = g_slist_next(item)) {

		pw3270_action_view_append_element(store, (struct ListElement *) item->data);

	}

}

static GSList * append_action(GSList * list, const gchar *prefix, GAction *action) {

	if(!action)
		return list;

	GdkPixbuf * pixbuf = g_action_get_pixbuf(action, GTK_ICON_SIZE_MENU, GTK_ICON_LOOKUP_GENERIC_FALLBACK);
	if(!pixbuf) {
		debug("Action \"%s\": Doesn't have a pixbuf",g_action_get_name(action));
		return list;
	}

	g_autofree gchar *action_name = g_strconcat(prefix,".",g_action_get_name(action),NULL);

	GValue value = G_VALUE_INIT;
	g_value_init(&value, G_TYPE_STRING);
	g_object_get_property(G_OBJECT(action),"label",&value);

	const gchar * label = g_value_get_string(&value);
	if(!(label && *label))
		label = g_action_get_name(action);

	list = pw3270_action_list_append(
	           list,
	           label,
	           pixbuf,
	           action_name,
	           PW3270_ACTION_VIEW_ALLOW_MOVE
	       );

	g_value_unset(&value);

	return list;
}

Pw3270ActionList * pw3270_action_list_new(GtkApplication *application) {

	GSList * list = NULL;

	gchar ** action_names;
	size_t ix;

	// Get application actions.

	action_names = g_action_group_list_actions(G_ACTION_GROUP(application));
	for(ix = 0; action_names[ix]; ix++) {
		list = append_action(list,"app",g_action_map_lookup_action(G_ACTION_MAP(application),action_names[ix]));
	}
	g_strfreev(action_names);

	// Get Windows actions.
	GtkWindow * window = gtk_application_get_active_window(application);

	if(window && G_IS_ACTION_GROUP(window)) {

		// Get window actions.
		action_names = g_action_group_list_actions(G_ACTION_GROUP(window));
		for(ix = 0; action_names[ix]; ix++) {
			list = append_action(list,"win",g_action_map_lookup_action(G_ACTION_MAP(window),action_names[ix]));
		}
		g_strfreev(action_names);

	}

	return (Pw3270ActionList *) list;
}

Pw3270ActionList * pw3270_action_list_append(Pw3270ActionList *action_list, const gchar *label, GdkPixbuf *pixbuf, const gchar *action_name, const PW3270ActionViewFlag G_GNUC_UNUSED(flags)) {

	struct ListElement * element = (struct ListElement *)
	                               g_malloc0(
	                                   sizeof(struct ListElement)
	                                   + strlen(action_name)
	                                   + strlen(label)
	                                   + 3
	                               );

	if(pixbuf) {
		element->pixbuf = pixbuf;
		g_object_ref_sink(G_OBJECT(element->pixbuf));
	}

	element->action_name = (char *) (element+1);
	strcpy(element->action_name,action_name);

	element->action_label = element->action_name + strlen(element->action_name) + 1;
	strcpy(element->action_label,label);

	return (Pw3270ActionList *) g_slist_prepend((GSList *) action_list, element);


};

void list_element_free(struct ListElement *element) {

	if(element->pixbuf) {
		g_object_unref(element->pixbuf);
		element->pixbuf = NULL;
	}

	g_free(element);

}

void pw3270_action_list_free(Pw3270ActionList *action_list) {
	g_slist_free_full((GSList *) action_list, (GDestroyNotify) list_element_free);
}

void pw3270_action_view_move_selected(GtkWidget *from, GtkWidget *to) {

	size_t ix;

	//
	// Get list of selected references.
	//
	GtkTreeModel * fromModel = gtk_tree_view_get_model(GTK_TREE_VIEW(from));
	GList * list = gtk_tree_selection_get_selected_rows(
	                   gtk_tree_view_get_selection(GTK_TREE_VIEW(from)),
	                   &fromModel);

	guint rowCount = g_list_length(list);

	g_autofree GtkTreeRowReference ** rows = g_new0(GtkTreeRowReference *,rowCount);
	GList * item = list;
	for(ix=0; ix < rowCount && item; ix++) {
		rows[ix] = gtk_tree_row_reference_new(fromModel,(GtkTreePath *) item->data);
		item = g_list_next(item);
	}

	g_list_free_full(list, (GDestroyNotify) gtk_tree_path_free);

	//
	// Move references
	//
	for(ix = 0; ix < rowCount && rows[ix]; ix++) {

		GtkTreePath * path = gtk_tree_row_reference_get_path(rows[ix]);

		if(path) {
			GtkTreeIter	iter;

			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(fromModel), &iter, path)) {

				GValue vFlags = G_VALUE_INIT;
				gtk_tree_model_get_value(fromModel,&iter,COLUMN_FLAGS,&vFlags);
				gint flags = g_value_get_int(&vFlags);
				g_value_unset(&vFlags);

				if(flags & PW3270_ACTION_VIEW_FLAG_ALLOW_ADD) {

					// Add on target widget.
					GValue pixbuf = G_VALUE_INIT;
					GValue label  = G_VALUE_INIT;
					GValue action_name = G_VALUE_INIT;

					gtk_tree_model_get_value(fromModel,&iter,COLUMN_PIXBUF,&pixbuf);
					gtk_tree_model_get_value(fromModel,&iter,COLUMN_LABEL,&label);
					gtk_tree_model_get_value(fromModel,&iter,COLUMN_ACTION_NAME,&action_name);

					GtkListStore * store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(to)));

					GtkTreeIter ins;
					gtk_list_store_append(store, &ins);
					gtk_list_store_set(
					    store,
					    &ins,
					    COLUMN_PIXBUF,		g_value_get_object(&pixbuf),
					    COLUMN_LABEL, 		g_value_get_string(&label),
					    COLUMN_ACTION_NAME,	g_value_get_string(&action_name),
					    COLUMN_FLAGS,		(flags == 3 ? 3 : 2),
					    -1
					);

					g_value_unset(&pixbuf);
					g_value_unset(&label);
					g_value_unset(&action_name);

				}

				if(flags & PW3270_ACTION_VIEW_ALLOW_REMOVE) {

					// Remove from source widget.
					gtk_list_store_remove(GTK_LIST_STORE(fromModel), &iter);

				}


			}

		}

	}

}

gboolean get_action_name(GtkTreeModel *model, GtkTreePath G_GNUC_UNUSED(*path), GtkTreeIter *iter, GString *str) {

	GValue value = G_VALUE_INIT;
	gtk_tree_model_get_value(model,iter,COLUMN_ACTION_NAME,&value);

	if(*str->str)
		g_string_append(str,",");

	g_string_append(str,g_value_get_string(&value));

	g_value_unset(&value);
	return FALSE;
}

gchar * pw3270_action_view_get_action_names(GtkWidget *widget) {

	GString *str = g_string_new("");

	gtk_tree_model_foreach(
	    gtk_tree_view_get_model(GTK_TREE_VIEW(widget)),
	    (GtkTreeModelForeachFunc) get_action_name,
	    str );

	return g_string_free(str,FALSE);
}

static void check_4_sensitive(GtkTreeModel *model, GtkTreePath G_GNUC_UNUSED(*path), GtkTreeIter *iter, gboolean *sensitive) {

	GValue value = { 0, };
	gtk_tree_model_get_value(model,iter,COLUMN_FLAGS,&value);

	if(!(g_value_get_int(&value) & PW3270_ACTION_VIEW_FLAG_ALLOW_ADD))
		*sensitive = FALSE;

	g_value_unset(&value);

}

static void selection_changed(GtkTreeSelection *selection, GtkWidget *button) {

	if(!gtk_tree_selection_count_selected_rows(selection)) {
		gtk_widget_set_sensitive(button,FALSE);
		return;
	}

	gboolean sensitive = TRUE;

	// Scan selected rows
	gtk_tree_selection_selected_foreach(selection,(GtkTreeSelectionForeachFunc) check_4_sensitive,&sensitive);
	gtk_widget_set_sensitive(button,sensitive);

}

struct MoveData {
	GtkWidget *from;
	GtkWidget *to;
};

static void move_clicked(GtkButton G_GNUC_UNUSED(*button), struct MoveData *args) {
	pw3270_action_view_move_selected(args->from,args->to);
}

GtkWidget * pw3270_action_view_move_button_new(GtkWidget *from, GtkWidget *to, const gchar *icon_name) {

	GtkWidget * button = gtk_button_new_from_icon_name(icon_name,GTK_ICON_SIZE_DND);

	struct MoveData * data = g_new0(struct MoveData,1);
	data->from	= from;
	data->to 	= to;
	g_object_set_data_full(G_OBJECT(button),"move-control-data",data,g_free);

	gtk_widget_set_focus_on_click(button,FALSE);
	gtk_button_set_relief(GTK_BUTTON(button),GTK_RELIEF_NONE);
	gtk_widget_set_sensitive(button,FALSE);
	gtk_widget_set_hexpand(button,FALSE);
	gtk_widget_set_vexpand(button,FALSE);

	g_signal_connect(
	    gtk_tree_view_get_selection(GTK_TREE_VIEW(from)),
	    "changed",
	    G_CALLBACK(selection_changed),
	    button
	);

	g_signal_connect(
	    button,
	    "clicked",
	    G_CALLBACK(move_clicked),
	    data
	);

	return button;
}
