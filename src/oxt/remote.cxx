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
 * programa;  se  não, escreva para a Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA, 02111-1307, USA
 *
 * Este programa está nomeado como remote.cxx e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 *
 */

 #include "globals.hpp"
 #include <errno.h>
 #include <string.h>

/*---[ Statics ]-------------------------------------------------------------------------------------------*/

#if defined(HAVE_DBUS)
 static const char * prefix_dest	= "br.com.bb.";
 static const char * prefix_path	= "/br/com/bb/";
#endif // HAVE_DBUS

/*---[ Implement ]-----------------------------------------------------------------------------------------*/

#if defined(HAVE_DBUS)
DBusMessage * pw3270::ipc3270_session::create_message(const char *method)
{
	DBusMessage * msg = dbus_message_new_method_call(	this->dest,		// Destination
														this->path,		// Path
														this->intf,		// Interface
														method);		// method

	if (!msg)
		log("Error creating message for method %s",method);

	return msg;
}

DBusMessage	* pw3270::ipc3270_session::call(DBusMessage *msg)
{
	DBusMessage		* reply;
	DBusError		  error;

	dbus_error_init(&error);
	reply = dbus_connection_send_with_reply_and_block(conn,msg,10000,&error);
	dbus_message_unref(msg);

	if(!reply)
	{
		log("%s",error.message);
		dbus_error_free(&error);
	}

	return reply;

}

static char * get_string(DBusMessage * msg)
{
	char *rc = NULL;
	if(msg)
	{
		DBusMessageIter iter;

		if(dbus_message_iter_init(msg, &iter))
		{
			if(dbus_message_iter_get_arg_type(&iter) == DBUS_TYPE_STRING)
			{
				const char * str;
				dbus_message_iter_get_basic(&iter, &str);
				trace("Response: [%s]",str);
				rc = strdup(str);
			}
#ifdef DEBUG
			else
			{
				trace("Return type is %c, expecting %c",dbus_message_iter_get_arg_type(&iter),DBUS_TYPE_STRING);
			}
#endif
		}

		dbus_message_unref(msg);
	}
	return rc;
}

char * pw3270::ipc3270_session::query_string(const char *method)
{
	if(conn)
		return get_string(call(create_message(method)));
	return NULL;
}

static int get_intval(DBusMessage * msg)
{
	int rc = -1;

	if(msg)
	{
		DBusMessageIter iter;

		if(dbus_message_iter_init(msg, &iter))
		{
			if(dbus_message_iter_get_arg_type(&iter) == DBUS_TYPE_INT32)
			{
				dbus_int32_t iSigned;
				dbus_message_iter_get_basic(&iter, &iSigned);
				rc = (int) iSigned;
			}
#ifdef DEBUG
			else
			{
				trace("Return type is %c, expecting %c",dbus_message_iter_get_arg_type(&iter),DBUS_TYPE_INT32);
			}
#endif
		}

		dbus_message_unref(msg);
	}

	return rc;
}

int pw3270::ipc3270_session::query_intval(const char *method)
{
	if(conn)
		return get_intval(call(create_message(method)));
	return -1;
}

#endif // HAVE_DBUS


pw3270::ipc3270_session::ipc3270_session(uno_impl *obj, const char *name) throw( RuntimeException ) : pw3270::session()
{
#ifdef HAVE_DBUS

	DBusError	  err;
	int			  rc;
	char		* str = strdup(name);
	char		* ptr;

	for(ptr=str;*ptr;ptr++)
		*ptr = tolower(*ptr);

	ptr = strchr(str,':');

	if(ptr)
	{
		size_t 		  sz;

		*(ptr++) = 0;

		// Build destination
		sz		= strlen(ptr)+strlen(str)+strlen(prefix_dest)+2;
		dest	= (char *) malloc(sz+1);
		strncpy(dest,prefix_dest,sz);
		strncat(dest,str,sz);
		strncat(dest,".",sz);
		strncat(dest,ptr,sz);

		// Build path
		sz		= strlen(str)+strlen(prefix_path);
		path	= (char *) malloc(sz+1);
		strncpy(path,prefix_path,sz);
		strncat(path,str,sz);

		// Build intf
		sz		= strlen(str)+strlen(prefix_dest)+1;
		intf	= (char *) malloc(sz+1);
		strncpy(intf,prefix_dest,sz);
		strncat(intf,str,sz);

	}
	else
	{
		size_t sz;

		// Build destination
		sz		= strlen(str)+strlen(prefix_dest)+2;
		dest	= (char *) malloc(sz+1);
		strncpy(dest,prefix_dest,sz);
		strncat(dest,str,sz);

		// Build path
		sz		= strlen(str)+strlen(prefix_path);
		path	= (char *) malloc(sz+1);
		strncpy(path,prefix_path,sz);
		strncat(path,str,sz);

		// Build intf
		sz		= strlen(str)+strlen(prefix_dest)+1;
		intf	= (char *) malloc(sz+1);
		strncpy(intf,prefix_dest,sz);
		strncat(intf,str,sz);

	}

	trace("DBUS:\nDestination:\t[%s]\nPath:\t\t[%s]\nInterface:\t[%s]",dest,path,intf);

	free(str);

	dbus_error_init(&err);

	conn = dbus_bus_get(DBUS_BUS_SESSION, &err);

	trace("conn=%p",conn);

	if (dbus_error_is_set(&err))
	{
		trace("DBUS Connection Error (%s)", err.message);
		obj->failed("DBUS Connection Error (%s)", err.message);
		dbus_error_free(&err);
		return;
	}

	if(!conn)
	{
		obj->failed("%s", "DBUS Connection failed");
		return;
	}

	rc = dbus_bus_request_name(conn, "br.com.bb." PACKAGE_NAME ".oo", DBUS_NAME_FLAG_REPLACE_EXISTING , &err);

	if (dbus_error_is_set(&err))
	{
		obj->failed("DBUS Name Error (%s)", err.message);
		dbus_error_free(&err);
		conn = NULL;
		return;
	}

	if(rc != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)
	{
		obj->failed("%s", "DBUS request name failed");
		conn = NULL;
		return;
	}
	else
	{
		DBusMessage		* reply;
		DBusMessage		* msg		= create_message("getRevision");
		DBusError		  error;

		dbus_error_init(&error);
		reply = dbus_connection_send_with_reply_and_block(conn,msg,10000,&error);
		dbus_message_unref(msg);

		if(reply)
		{
			log("%s","PW3270 DBus object found");
			dbus_message_unref(reply);
		}
		else
		{
			obj->failed("DBUS error: %s",error.message);
			dbus_error_free(&error);
		}
	}
#else

#endif // HAVE_DBUS
}

pw3270::ipc3270_session::~ipc3270_session()
{
#ifdef HAVE_DBUS

	free(dest);
	free(path);
	free(intf);

#endif // HAVE_DBUS
}

int pw3270::ipc3270_session::get_revision(void)
{
#ifdef HAVE_DBUS
	char *ptr = query_string("getRevision");
	if(ptr)
	{
		int rc = atoi(ptr);
		free(ptr);
		return rc;
	}
	return -1;

#else

	return -1;

#endif // HAVE_DBUS
}

LIB3270_MESSAGE pw3270::ipc3270_session::get_state(void)
{
#ifdef HAVE_DBUS

	return (LIB3270_MESSAGE) query_intval("getMessageID");

#else

	return (LIB3270_MESSAGE) -1;

#endif // HAVE_DBUS
}

char * pw3270::ipc3270_session::get_text_at(int row, int col, int len)
{
#ifdef HAVE_DBUS

	dbus_int32_t r = (dbus_int32_t) row;
	dbus_int32_t c = (dbus_int32_t) col;
	dbus_int32_t l = (dbus_int32_t) len;

	DBusMessage * msg = create_message("getTextAt");
	if(!msg)
		return NULL;

	trace("%s(%d,%d,%d)",__FUNCTION__,r,c,l);
	dbus_message_append_args(msg, DBUS_TYPE_INT32, &r, DBUS_TYPE_INT32, &c, DBUS_TYPE_INT32, &l, DBUS_TYPE_INVALID);

	return get_string(call(msg));

#else

	return NULL;

#endif // HAVE_DBUS
}

int pw3270::ipc3270_session::set_text_at(int row, int col, const char *text)
{
#ifdef HAVE_DBUS

	dbus_int32_t r = (dbus_int32_t) row;
	dbus_int32_t c = (dbus_int32_t) col;

	DBusMessage * msg = create_message("setTextAt");
	if(msg)
	{
		dbus_message_append_args(msg, DBUS_TYPE_INT32, &r, DBUS_TYPE_INT32, &c, DBUS_TYPE_STRING, &text, DBUS_TYPE_INVALID);
		return get_intval(call(msg));
	}

#else

	return -1;

#endif // HAVE_DBUS
}

int pw3270::ipc3270_session::cmp_text_at(int row, int col, const char *text)
{
#ifdef HAVE_DBUS

	dbus_int32_t r = (dbus_int32_t) row;
	dbus_int32_t c = (dbus_int32_t) col;

	DBusMessage * msg = create_message("cmpTextAt");
	if(msg)
	{
		dbus_message_append_args(msg, DBUS_TYPE_INT32, &r, DBUS_TYPE_INT32, &c, DBUS_TYPE_STRING, &text, DBUS_TYPE_INVALID);
		return get_intval(call(msg));
	}

#else

	return -1;

#endif // HAVE_DBUS
}

void pw3270::ipc3270_session::set_toggle(LIB3270_TOGGLE toggle, bool state)
{
#ifdef HAVE_DBUS

	dbus_int32_t i = (dbus_int32_t) toggle;
	dbus_int32_t v = (dbus_int32_t) state;

	DBusMessage * msg = create_message("setToggle");
	if(msg)
	{
		dbus_message_append_args(msg, DBUS_TYPE_INT32, &i, DBUS_TYPE_INT32, &v, DBUS_TYPE_INVALID);
		get_intval(call(msg));
	}

#endif // HAVE_DBUS
}

int pw3270::ipc3270_session::connect(const char *uri)
{
#ifdef HAVE_DBUS

	int rc;
	DBusMessage * msg = create_message("connect");
	if(!msg)
		return -1;

	dbus_message_append_args(msg, DBUS_TYPE_STRING, &uri, DBUS_TYPE_INVALID);

	return get_intval(call(msg));

#else

	return -1;

#endif // HAVE_DBUS
}

int pw3270::ipc3270_session::disconnect(void)
{
#ifdef HAVE_DBUS

	return query_intval("disconnect");

#else

	return -1;

#endif // HAVE_DBUS
}

bool pw3270::ipc3270_session::connected(void)
{
#ifdef HAVE_DBUS

	return query_intval("isConnected") > 0;

#else

	return false;

#endif // HAVE_DBUS
}

int pw3270::ipc3270_session::enter(void)
{
#ifdef HAVE_DBUS

	return query_intval("enter");

#else

	return -1;

#endif // HAVE_DBUS
}

int pw3270::ipc3270_session::pfkey(int key)
{
#ifdef HAVE_DBUS

	dbus_int32_t k = (dbus_int32_t) key;

	DBusMessage * msg = create_message("pfKey");
	if(msg)
	{
		dbus_message_append_args(msg, DBUS_TYPE_INT32, &k, DBUS_TYPE_INVALID);
		return get_intval(call(msg));
	}

#endif // HAVE_DBUS

	return -1;

}

int	pw3270::ipc3270_session::pakey(int key)
{
#ifdef HAVE_DBUS

	dbus_int32_t k = (dbus_int32_t) key;

	DBusMessage * msg = create_message("paKey");
	if(msg)
	{
		dbus_message_append_args(msg, DBUS_TYPE_INT32, &k, DBUS_TYPE_INVALID);
		return get_intval(call(msg));
	}

#endif // HAVE_DBUS

	return -1;

}

bool pw3270::ipc3270_session::in_tn3270e()
{
#ifdef HAVE_DBUS

	return query_intval("inTN3270E") > 0;

#else

	return false;

#endif // HAVE_DBUS
}

void pw3270::ipc3270_session::mem_free(void *ptr)
{
#ifdef HAVE_DBUS

	free(ptr);

#else


#endif // HAVE_DBUS
}

