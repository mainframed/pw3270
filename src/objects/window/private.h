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

#ifndef PRIVATE_H_INCLUDED

#define PRIVATE_H_INCLUDED

#include <config.h>

#ifndef GETTEXT_PACKAGE
#define GETTEXT_PACKAGE PACKAGE_NAME
#endif

#include <libintl.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include <pw3270/window.h>
#include <v3270.h>
#include <lib3270.h>
#include <lib3270/log.h>

enum pw3270ApplicationWindowPopup {
	PW3270_APP_WINDOW_POPUP_OVER_SELECTED_AREA,
	PW3270_APP_WINDOW_POPUP_OVER_UNSELECTED_AREA,
	PW3270_APP_WINDOW_POPUP_OVER_OIA,
	PW3270_APP_WINDOW_POPUP_WHEN_OFFLINE,

	PW3270_APP_WINDOW_POPUP_COUNT
};

#define PW3270_APP_WINDOW_POPUP_DEFAULT PW3270_APP_WINDOW_POPUP_OVER_UNSELECTED_AREA

struct _pw3270ApplicationWindow {

	GtkApplicationWindow parent;

	GtkWidget	* terminal;	///< @brief The active terminal.
	GtkNotebook * notebook;
	GtkToolbar	* toolbar;

	GList		* keypads;		///< @brief Keypads.

	struct {
		int width;				///< @brief Window width.
		int height;				///< @brief Window height.
		int is_maximized : 1;	///< @brief Non zero if window is maximized.
		int is_fullscreen : 1;	///< @brief Non zero if window is fullscreen.
	} state;

	GtkWidget	* popups[PW3270_APP_WINDOW_POPUP_COUNT];

};

struct _pw3270ApplicationWindowClass {

	GtkApplicationWindowClass parent_class;

};

// Actions
G_GNUC_INTERNAL GAction * pw3270_action_host_properties_new(void);
G_GNUC_INTERNAL GAction * pw3270_set_color_action_new(void);
G_GNUC_INTERNAL GAction * pw3270_file_transfer_action_new(void);
G_GNUC_INTERNAL GAction * pw3270_action_window_close_new(void);
G_GNUC_INTERNAL GAction	* pw3270_action_connect_new(void);
G_GNUC_INTERNAL GAction	* pw3270_action_save_session_preferences_new(void);
G_GNUC_INTERNAL GAction * pw3270_action_save_desktop_icon_new(void);

GAction * pw3270_action_session_properties_new(void);

// Header bar
G_GNUC_INTERNAL void	  pw3270_window_set_header_action_names(GtkWidget *window, const gchar *action_names);
G_GNUC_INTERNAL	gchar	* pw3270_window_get_action_names(GtkWidget *window);

// Terminal actions.
G_GNUC_INTERNAL GAction * pw3270_model_number_action_new(GtkWidget *terminal);

G_GNUC_INTERNAL void pw3270_window_open_activated(GSimpleAction * action, GVariant *parameter, gpointer window);
G_GNUC_INTERNAL void pw3270_window_close_activated(GSimpleAction * action, GVariant *parameter, gpointer window);

// Window widgets & tools.
G_GNUC_INTERNAL GtkWidget * pw3270_header_button_new_from_builder(GtkWidget *widget, GtkBuilder * builder, const gchar *action_name);

#endif // PRIVATE_H_INCLUDED
