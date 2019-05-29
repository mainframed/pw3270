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
 * Este programa está nomeado como main.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 *
 */

#include "private.h"

#ifdef _WIN32
	#include <windows.h>
#endif // _WIN32

#include <glib.h>
#include <glib/gstdio.h>

#ifdef HAVE_GTKMAC
 #include <gtkosxapplication.h>
#endif // HAVE_GTKMAC

#include <v3270.h>
#include <pw3270/plugin.h>
#include "v3270/accessible.h"
#include <stdlib.h>

#if defined( HAVE_SYSLOG )
	#include <syslog.h>
#endif // HAVE_SYSLOG

#define ERROR_DOMAIN g_quark_from_static_string(PACKAGE_NAME)

/*--[ Statics ]--------------------------------------------------------------------------------------*/

 static GtkWidget		* toplevel		= NULL;
 static unsigned int	  syscolors		= 16;
 static unsigned int	  timer			= 0;
 static const gchar		* systype		= NULL;
 static const gchar		* toggleset		= NULL;
 static const gchar		* togglereset	= NULL;
 static const gchar     * logfile       = NULL;
 static const gchar		* charset		= NULL;
 static const gchar		* remap			= NULL;
 static const gchar		* model			= NULL;

 const gchar			* tracefile		= NULL;

#ifdef HAVE_GTKMAC
 GtkOSXApplication		* osxapp		= NULL;
#endif // HAVE_GTKMAC

#if defined( HAVE_SYSLOG )
 static gboolean	  	  log_to_syslog	= FALSE;
#endif // HAVE_SYSLOG



/*--[ Implement ]------------------------------------------------------------------------------------*/

static int initialize(void)
{
	const gchar * msg = gtk_check_version(GTK_MAJOR_VERSION, GTK_MINOR_VERSION, GTK_MICRO_VERSION);

	if(msg)
	{
		// Invalid GTK version, notify user
		int rc;

		GtkWidget *dialog = gtk_message_dialog_new(	NULL,
													GTK_DIALOG_DESTROY_WITH_PARENT,
													GTK_MESSAGE_WARNING,
													GTK_BUTTONS_OK_CANCEL,
													_( "%s requires GTK version %d.%d.%d" ),PACKAGE_NAME,GTK_MAJOR_VERSION,GTK_MINOR_VERSION,GTK_MICRO_VERSION );


		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),"%s",msg);
		gtk_window_set_title(GTK_WINDOW(dialog),_( "GTK Version mismatch" ));
		gtk_window_set_deletable(GTK_WINDOW(dialog),FALSE);

        rc = gtk_dialog_run(GTK_DIALOG (dialog));
        gtk_widget_destroy(dialog);

        if(rc != GTK_RESPONSE_OK)
			return EINVAL;
	}

	return 0;
}

static void toplevel_setup(GtkWindow *window)
{
 	g_autofree gchar * name	= g_strdup_printf("%s.png",g_get_application_name());
 	g_autofree gchar * role	= g_strdup_printf("%s_top",g_get_application_name());

	gtk_window_set_type_hint(window,GDK_WINDOW_TYPE_HINT_NORMAL);
	gtk_window_set_position(window,GTK_WIN_POS_CENTER);
	gtk_window_set_role(window,role);

#ifndef _WIN32
	// Set default icon
	g_autofree gchar * filename = pw3270_build_filename(GTK_WIDGET(window),name,NULL);
	if(g_file_test(filename,G_FILE_TEST_EXISTS))
	{
		GError * error = NULL;

		trace("Loading default icon from %s",filename);

		if(!gtk_window_set_default_icon_from_file(filename,&error))
		{
			g_warning("Error %s loading default icon from %s",error->message,filename);
			g_error_free(error);
		}
	}
#endif // _WIN32

}

static gboolean optcolors(const gchar *option_name, const gchar *value, gpointer data, GError **error)
{
	static const unsigned short	valid[] = { 2,8,16 };
	int 		 				f;
	unsigned short				optval = (unsigned short) atoi(value);

	for(f=0;f<G_N_ELEMENTS(valid);f++)
	{
		if(optval == valid[f])
		{
			syscolors = optval;
			return TRUE;
		}
	}

	*error = g_error_new(ERROR_DOMAIN,EINVAL, _("Unexpected or invalid color value \"%s\""), value );
	return FALSE;
}

#if defined( HAVE_SYSLOG )
static void g_syslog(const gchar *log_domain,GLogLevelFlags log_level,const gchar *message,gpointer user_data)
{
 	static const struct _logtype
 	{
 		GLogLevelFlags	  log_level;
 		int 			  priority;
 		const gchar		* msg;
 	} logtype[] =
 	{
		{ G_LOG_FLAG_RECURSION,	LOG_INFO,		"recursion"			},
		{ G_LOG_FLAG_FATAL,		LOG_ERR,		"fatal error"		},

		/* GLib log levels */
		{ G_LOG_LEVEL_ERROR,	LOG_ERR,		"error"				},
		{ G_LOG_LEVEL_CRITICAL,	LOG_ERR,		"critical error"	},
		{ G_LOG_LEVEL_WARNING,	LOG_ERR,		"warning"			},
		{ G_LOG_LEVEL_MESSAGE,	LOG_ERR,		"message"			},
		{ G_LOG_LEVEL_INFO,		LOG_INFO,		"info"				},
		{ G_LOG_LEVEL_DEBUG,	LOG_DEBUG,		"debug"				},
 	};

	int f;

	for(f=0;f<G_N_ELEMENTS(logtype);f++)
	{
		if(logtype[f].log_level == log_level)
		{
			gchar *ptr;
			gchar *text = g_strdup_printf("%s: %s %s",logtype[f].msg,log_domain ? log_domain : "",message);
			for(ptr = text;*ptr;ptr++)
			{
				if(*ptr < ' ')
					*ptr = ' ';
			}

			syslog(logtype[f].priority,"%s",text);
			g_free(text);
			return;
		}
	}

	syslog(LOG_INFO,"%s %s",log_domain ? log_domain : "", message);
}
#endif // HAVE_SYSLOG

static void g_logfile(const gchar *log_domain,GLogLevelFlags log_level,const gchar *message,gpointer user_data)
{
    FILE *out = fopen(logfile,"a");
    if(out)
    {
        time_t  ltime;
        char    wrk[40];
        time(&ltime);
        strftime(wrk, 39, "%d/%m/%Y %H:%M:%S", localtime(&ltime));
        fprintf(out,"%s\t%s\n",wrk,message);
        fclose(out);
    }
}

static gboolean startup(GtkWidget *toplevel)
{
	trace("%s",__FUNCTION__);

	gtk_window_present(GTK_WINDOW(toplevel));

#ifdef HAVE_GTKMAC
	gtk_osxapplication_ready(osxapp);
#endif // HAVE_GTKMAC

	pw3270_start_plugins(toplevel);

	return FALSE;
}

int main(int argc, char *argv[])
{
	const gchar		* pluginpath	= NULL;
#ifdef APPLICATION_NAME
	const char		* app_name		= G_STRINGIFY(APPLICATION_NAME);
#else
	const char		* app_name		= NULL;
#endif // APPLICATION_NAME

#ifdef DEFAULT_SESSION_NAME
	const gchar		* session_name	= G_STRINGIFY(DEFAULT_SESSION_NAME);
#else
	const gchar		* session_name	= NULL;
#endif // DEFAULT_SESSION_NAME

	const gchar		* host			= lib3270_get_default_host(NULL);
	int 			  rc 			= 0;

    trace("%s",__FUNCTION__);

#if ! GLIB_CHECK_VERSION(2,32,0)
	g_thread_init(NULL);
#endif // !GLIB(2,32)

	// Setup locale
#ifdef LC_ALL
	setlocale( LC_ALL, "" );
#endif

	// OS
#if defined( _WIN32 )
	{
		g_autofree gchar * appdir = g_win32_get_package_installation_directory_of_module(NULL);
		g_autofree gchar * locdir = g_build_filename(appdir,"locale",NULL);

		trace("appdir=\"%s\"",appdir);
		trace("locdir=\"%s\"",locdir);

		g_chdir(appdir);

		bindtextdomain( PACKAGE_NAME, locdir );
		bind_textdomain_codeset(PACKAGE_NAME, "UTF-8");
		textdomain(PACKAGE_NAME);

#if defined(ENABLE_WINDOWS_REGISTRY)
		HKEY hMainKey;
		DWORD	disp;
		LSTATUS winRegError = RegCreateKeyEx(HKEY_CURRENT_USER,"SOFTWARE\\" PACKAGE_NAME,0,NULL,REG_OPTION_NON_VOLATILE,KEY_SET_VALUE,NULL,&hMainKey,&disp);
		if(winRegError == ERROR_SUCCESS)
		{
			HKEY hKey;

			winRegError = RegCreateKeyEx(HKEY_CURRENT_USER,"SOFTWARE\\" PACKAGE_NAME "\\application",0,NULL,REG_OPTION_NON_VOLATILE,KEY_SET_VALUE,NULL,&hKey,&disp);
			if(winRegError == ERROR_SUCCESS)
			{
				const struct _versions
				{
					const char * name;
					const char * value;
				}
				versions[] =
				{
					{ "version",	PACKAGE_VERSION					},
					{ "release",	G_STRINGIFY(PACKAGE_RELEASE)	},
					{ "path",		appdir							},
				};

				size_t ix;

				for(ix = 0; ix < G_N_ELEMENTS(versions); ix++)
				{
					RegSetValueEx(hKey,versions[ix].name,0,REG_SZ,(const BYTE *) versions[ix].value,strlen(versions[ix].value)+1);
				}

				RegCloseKey(hKey);

			}
#ifdef DEBUG
			else
			{
				g_error("Can't open HKCU\\SOFTWARE\\" PACKAGE_NAME ": %s", lib3270_win32_strerror(winRegError));
			}
#endif
			RegCloseKey(hMainKey);
		}
#ifdef DEBUG
		else
		{
			g_error("Can't open HKCU\\SOFTWARE\\" PACKAGE_NAME "\\application: %s",lib3270_win32_strerror(winRegError));
		}
#endif

#endif // ENABLE_WINDOWS_REGISTRY

	}
#elif defined(HAVE_GTKMAC)
	{
		GtkMacBundle * macbundle = gtk_mac_bundle_get_default();

		g_chdir(gtk_mac_bundle_get_datadir(macbundle));

		bind_textdomain_codeset(PACKAGE_NAME, "UTF-8");
		textdomain(PACKAGE_NAME);

		bindtextdomain(PACKAGE_NAME,gtk_mac_bundle_get_localedir(macbundle));

		osxapp = GTK_OSX_APPLICATION(g_object_new(GTK_TYPE_OSX_APPLICATION,NULL));

	}
#else
	{
		bind_textdomain_codeset(PACKAGE_NAME, "UTF-8");
		textdomain(PACKAGE_NAME);
	}
#endif

	// Process command-line options
	{
		const GOptionEntry app_options[] =
		{
#ifdef DEFAULT_SESSION_NAME
			{ "session",			's', 0, G_OPTION_ARG_STRING,	&session_name,		N_( "Session name" ),								G_STRINGIFY(DEFAULT_SESSION_NAME)	},
#else
			{ "session",			's', 0, G_OPTION_ARG_STRING,	&session_name,		N_( "Session name" ),								PACKAGE_NAME						},
#endif // DEFAULT_SESSION_NAME

			{ "host",				'h', 0, G_OPTION_ARG_STRING,	&host,				N_( "Host to connect"),								host								},
			{ "colors",				'c', 0, G_OPTION_ARG_CALLBACK,	optcolors,			N_( "Set reported colors (8/16)" ),					"16"								},
            { "systype",			't', 0, G_OPTION_ARG_STRING,	&systype,			N_( "Host system type" ),							"S390"								},
			{ "toggleset",			'S', 0, G_OPTION_ARG_STRING,	&toggleset,			N_( "Set toggles ON" ),								NULL								},
			{ "togglereset",		'R', 0, G_OPTION_ARG_STRING,	&togglereset,		N_( "Set toggles OFF" ),							NULL								},
			{ "charset",	    	'C', 0, G_OPTION_ARG_STRING,	&charset,		    N_( "Set host charset" ),							NULL								},
			{ "remap",		    	'm', 0, G_OPTION_ARG_FILENAME,	&remap,			    N_( "Remap charset from xml file" ),				NULL								},
			{ "model",		    	'M', 0, G_OPTION_ARG_STRING,	&model,			    N_( "The model of 3270 display to be emulated" ),	NULL								},
			{ "autodisconnect",		'D', 0, G_OPTION_ARG_INT,		&timer,			    N_( "Minutes for auto-disconnect" ),				0									},
			{ "pluginpath",			'P', 0, G_OPTION_ARG_STRING,	&pluginpath,	    N_( "Path for plugin files" ),						NULL								},

#ifdef APPLICATION_NAME
			{ "application-name",	'A', 0, G_OPTION_ARG_STRING,	&app_name,			N_( "Application name" ),							G_STRINGIFY(APPLICATION_NAME)		},
#else
			{ "application-name",	'A', 0, G_OPTION_ARG_STRING,	&app_name,			N_( "Application name" ),							NULL								},
#endif // APPLICATION_NAME

#if defined( HAVE_SYSLOG )
			{ "syslog",				'l', 0, G_OPTION_ARG_NONE,		&log_to_syslog,		N_( "Send messages to syslog" ),					NULL								},
#endif
			{ "tracefile",			'T', 0, G_OPTION_ARG_FILENAME,	&tracefile,			N_( "Set trace filename" ),							NULL								},
			{ "log",		    	'L', 0, G_OPTION_ARG_FILENAME,	&logfile,		    N_( "Log to file" ),								NULL        						},

			{ NULL }
		};

		GOptionContext	* context		= g_option_context_new (_("- 3270 Emulator for Gtk"));
		GError			* error			= NULL;
		GOptionGroup 	* group			= g_option_group_new( PACKAGE_NAME, NULL, NULL, NULL, NULL);

		g_option_context_set_main_group(context, group);

		g_option_context_add_main_entries(context, app_options, NULL);

		if(!g_option_context_parse( context, &argc, &argv, &error ))
		{
			int f;
			GString 	* str;
			GtkWidget 	* dialog = gtk_message_dialog_new(	NULL,
														GTK_DIALOG_DESTROY_WITH_PARENT,
														GTK_MESSAGE_ERROR,
														GTK_BUTTONS_CANCEL,
														"%s", error->message);

			gtk_window_set_title(GTK_WINDOW(dialog),_( "Parse error" ));

			str = g_string_new( _( "<b>Valid options:</b>\n\n" ) );

			for(f=0;app_options[f].description;f++)
				g_string_append_printf(str,"--%-20s\t%s\n",app_options[f].long_name,gettext(app_options[f].description));

			gtk_message_dialog_format_secondary_markup(GTK_MESSAGE_DIALOG(dialog), "%s", str->str);

			gtk_dialog_run(GTK_DIALOG (dialog));
			gtk_widget_destroy(dialog);

			g_error_free(error);
			g_string_free(str,TRUE);

			return -1;
		}
	}

	if(app_name)
	{
		g_set_application_name(app_name);
		g_message( _( "Application name set to \"%s\"" ), app_name);
	}
#ifdef _WIN32
	else
	{
		g_set_application_name(PACKAGE_NAME);
	}
#endif // _WIN32

	// Init GTK
	gtk_init(&argc, &argv);

#if defined( HAVE_SYSLOG )
	if(log_to_syslog)
	{
		openlog(g_get_prgname(), LOG_NDELAY, LOG_USER);
		g_log_set_default_handler(g_syslog,NULL);
	}
	else if(logfile)
#else
	if(logfile)
#endif // HAVE_SYSLOG
	{
		g_log_set_default_handler(g_logfile,NULL);

#ifdef _WIN32
		g_autofree gchar * appdir = g_win32_get_package_installation_directory_of_module(NULL);

		g_message("Windows Application directory is \"%s\"",appdir);
		g_message("Application name is \"%s\"", g_get_application_name());
		g_message("Session name is \"%s\"", session_name ? session_name : "undefined");

#if defined(ENABLE_WINDOWS_REGISTRY)
		g_message("Registry path is \"HKCU\\%s\"",PACKAGE_NAME);
#endif  // ENABLE_WINDOWS_REGISTRY

#endif // _WIN32


	}

	// Check GTK Version
	{
		const gchar	*msg = gtk_check_version(GTK_MAJOR_VERSION, GTK_MINOR_VERSION, GTK_MICRO_VERSION);

		if(msg)
		{
			// Invalid GTK version, notify user and exit
			int 		  rc;
			GtkWidget	* dialog = gtk_message_dialog_new(	NULL,
															GTK_DIALOG_DESTROY_WITH_PARENT,
															GTK_MESSAGE_WARNING,
															GTK_BUTTONS_OK_CANCEL,
															_( "This program requires GTK version %d.%d.%d" ),GTK_MAJOR_VERSION,GTK_MINOR_VERSION,GTK_MICRO_VERSION );


			gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),"%s",msg);
			gtk_window_set_title(GTK_WINDOW(dialog),_( "GTK Version mismatch" ));

#if GTK_CHECK_VERSION(2,10,0)
			gtk_window_set_deletable(GTK_WINDOW(dialog),FALSE);
#endif

			rc = gtk_dialog_run(GTK_DIALOG (dialog));
			gtk_widget_destroy(dialog);

			if(rc != GTK_RESPONSE_OK)
				return EINVAL;
		}

	}

	// Just in case!
	g_mkdir_with_parents(g_get_tmp_dir(),0777);

	if(!session_name)
		session_name = PACKAGE_NAME;

	rc = initialize();
	if(!rc)
	{
		GtkSettings *settings = gtk_settings_get_default();

		if(settings)
		{
			// http://developer.gnome.org/gtk/2.24/GtkSettings.html
			gtk_settings_set_string_property(settings,"gtk-menu-bar-accel","Menu","");
		}

		pw3270_load_plugins(pluginpath);
		toplevel = pw3270_new(host,systype,syscolors);
		pw3270_set_session_name(toplevel,session_name);

#ifdef _WIN32
		pw3270_set_string(toplevel,"application","session",session_name);
#endif // _WIN32

		if(toggleset)
		{
			gchar **str = g_strsplit(toggleset,",",-1);
			int f;

			for(f=0;str[f];f++)
				pw3270_set_toggle_by_name(toplevel,str[f],TRUE);

			g_strfreev(str);
		}

		if(togglereset)
		{
			gchar **str = g_strsplit(togglereset,",",-1);
			int f;

			for(f=0;str[f];f++)
				pw3270_set_toggle_by_name(toplevel,str[f],FALSE);

			g_strfreev(str);
		}

		pw3270_set_host_charset(toplevel,charset);
		pw3270_remap_from_xml(toplevel,remap);

		toplevel_setup(GTK_WINDOW(toplevel));

		if(pw3270_get_toggle(toplevel,LIB3270_TOGGLE_FULL_SCREEN))
			gtk_window_fullscreen(GTK_WINDOW(toplevel));
		else
			pw3270_restore_window(toplevel,"toplevel");

		if(model)
			lib3270_set_model(pw3270_get_session(toplevel),model);

		v3270_set_auto_disconnect(pw3270_get_terminal_widget(toplevel),timer);

		g_idle_add((GSourceFunc) startup, toplevel);

		gtk_main();

		pw3270_stop_plugins(toplevel);
		pw3270_unload_plugins();

	}

	return rc;
}
