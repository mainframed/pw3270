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

#include "private.h"
#include <pw3270/application.h>
#include <v3270/keyfile.h>

void pw3270_application_open(GApplication *application, GFile **files, gint n_files, const gchar G_GNUC_UNUSED(*hint)) {

	gint file;
	GtkWindow *window = NULL;

	debug("\n\n%s files=%d",__FUNCTION__,n_files);

	for(file = 0; file < n_files; file++) {

		debug("%s(%d,%p)",__FUNCTION__,file,files[file]);
		pw3270_application_open_file(GTK_APPLICATION(application),&window,files[file]);

		/*

		g_autofree gchar *path = g_file_get_path(files[file]);

		if(!path) {

			// It's not a session file descriptor, is it an URL?
			g_autofree gchar * scheme = g_file_get_uri_scheme(files[file]);

			if(!(g_ascii_strcasecmp(scheme,"tn3270") && g_ascii_strcasecmp(scheme,"tn3270s"))) {

				// It's an URL, load it in the default session.

				g_autofree gchar * uri = g_file_get_uri(files[file]);
				size_t sz = strlen(uri);

				if(sz > 0 && uri[sz-1] == '/')
					uri[sz-1] = 0;

				if(!(window && pw3270_application_allow_tabs(application))) {
					debug("%s: Creating new window",__FUNCTION__);
					window = pw3270_application_window_new(GTK_APPLICATION(application), NULL);
				} else {
					debug("%s: Creating new tab",__FUNCTION__);
					window = pw3270_application_window_new_tab(window, NULL);
				}

				// Load default
				GtkWidget * terminal = pw3270_application_window_get_active_terminal(window);

				v3270_set_default_session(terminal);
				v3270_set_url(terminal,uri);

			}

			continue;

		}

		if(g_file_test(path,G_FILE_TEST_IS_REGULAR)) {

			// The file exists, use it.
			debug("%s: Loading '%s'",__FUNCTION__,path);

			if(!(window && pw3270_application_allow_tabs(application))) {
				debug("%s: Creating new window",__FUNCTION__);
				window = pw3270_application_window_new(GTK_APPLICATION(application), path);
			} else {
				debug("%s: Creating new tab",__FUNCTION__);
				pw3270_application_window_new_tab(window,path);
			}

			continue;
		}

		{
			g_autofree gchar * basename = g_file_get_basename(files[file]);
			g_autofree gchar * filename = v3270_keyfile_find(basename);

			if(filename) {

				if(!window) {
					debug("%s: Creating new window",__FUNCTION__);
					window = pw3270_application_window_new(GTK_APPLICATION(application), filename);
				} else {
					debug("%s: Creating new tab",__FUNCTION__);
					pw3270_application_window_new_tab(window, filename);
				}

				continue;
			}

		}

		*/
	}

	if(window)
		gtk_window_present(GTK_WINDOW(window));

}

