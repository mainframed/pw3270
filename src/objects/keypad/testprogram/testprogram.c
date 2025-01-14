/*
 * "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
 * (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
 * aplicativos mainframe. Registro no INPI sob o nome G3270. Registro no INPI sob
 * o nome G3270.
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
 * Este programa está nomeado como testprogram.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

/**
 * @brief PW3270 Keypad module test program.
 *
 */

#include <config.h>
#include <lib3270/properties.h>
#include <pw3270/keypad.h>
#include <v3270.h>
#include <v3270/trace.h>

/*---[ Implement ]----------------------------------------------------------------------------------*/

static void activate(GtkApplication* app, G_GNUC_UNUSED gpointer user_data) {

	GtkWidget * window = gtk_application_window_new(app);

	// Load keypad
	GList *keypads = pw3270_keypad_model_new_from_xml(NULL,"keypad.xml");

	if(!keypads) {
		g_message("No keypad");
		g_application_quit(G_APPLICATION(app));
	}

	// Create keypad widget
	GObject * model = G_OBJECT(g_list_first(keypads)->data);
	if(model) {
		gtk_container_add(GTK_CONTAINER(window),pw3270_keypad_get_from_model(model));
	}

	// Setup and show main window
	gtk_window_set_title(GTK_WINDOW(window),"PW3270 Keypad test");
	// gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);
	// gtk_window_set_default_size (GTK_WINDOW (window), 800, 500);
	gtk_widget_show_all(window);

	g_list_free_full(keypads,g_object_unref);

}

int main (int argc, char **argv) {

	GtkApplication *app;
	int status;

	app = gtk_application_new ("br.com.bb.pw3270.keypad",G_APPLICATION_FLAGS_NONE);

	g_signal_connect (app, "activate", G_CALLBACK(activate), NULL);

	status = g_application_run (G_APPLICATION (app), argc, argv);
	g_object_unref (app);

	g_message("rc=%d",status);

	return 0;

}


