/**
 * @file sipe.h
 *
 * pidgin-sipe
 *
 * Copyright (C) 2008 Novell, Inc.
 * Copyright (C) 2007 Anibal Avelar <avelar@gmail.com>
 * Copyright (C) 2005 Thomas Butter <butter@uni-mannheim.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _PIDGIN_SIPE_H
#define _PIDGIN_SIPE_H

#include <glib.h>
#include <time.h>

#ifdef _WIN32
#include "internal.h"
#endif

#include "cipher.h"
#include "circbuffer.h"
#include "dnsquery.h"
#include "dnssrv.h"
#include "network.h"
#include "proxy.h"
#include "prpl.h"
#include "sslconn.h"

#include "sipmsg.h"
#include "sip-sec.h"
#include "uuid.h"

#define SIPE_UNUSED_PARAMETER __attribute__((unused))

#define SIMPLE_BUF_INC 4096

#define SIPE_TYPING_RECV_TIMEOUT 6
#define SIPE_TYPING_SEND_TIMEOUT 4

struct sipe_buddy {
	gchar *name;
	gchar *activity;
	gchar *meeting_subject;
	gchar *meeting_location;
	gchar *annotation;

	/* Calendar related fields */
	gchar *cal_start_time;
	int cal_granularity;
	gchar *cal_free_busy_base64;
	gchar *cal_free_busy;
	/* for 2005 systems */
	gchar *last_non_cal_status_id;
	gchar *last_non_cal_activity;

	struct sipe_cal_working_hours *cal_working_hours;

	gchar *device_name;
	GSList *groups;
	 /** flag to control sending 'context' element in 2007 subscriptions */
	gboolean just_added;
};

struct sip_auth {
	SipSecAuthType type;
	SipSecContext gssapi_context;
	gchar *gssapi_data;
	gchar *opaque;
	gchar *realm;
	gchar *target;
	int nc;
	int retries;
	int ntlm_num;
	int expires;
};

typedef enum {
	SIPE_TRANSPORT_TLS,
	SIPE_TRANSPORT_TCP,
	SIPE_TRANSPORT_UDP,
} sipe_transport_type;

struct sipe_service_data {
	const char *service;
	const char *transport;
	sipe_transport_type type;
};

/** MS-PRES publication */
struct sipe_publication {
	gchar *category;
	guint instance;
	guint container;
	guint version;
	/** for 'state' category */
	int availability;
	/** for 'state:calendarState' category */
	char *cal_event_hash;
	/** for 'note' category */
	gchar *note;
	/** for 'calendarData' category; 300(Team) container */
	char *working_hours_xml_str;
	char *fb_start_str;
	char *free_busy_base64;
};

/** MS-PRES container */
struct sipe_container {
	guint id;
	guint version;
	GSList *members;
};
/** MS-PRES container member */
struct sipe_container_member {
	/** user, domain, sameEnterprise, federated, publicCloud; everyone */
	const gchar *type;
	const gchar *value;
};

struct sipe_account_data {
	PurpleConnection *gc;
	gchar *sipdomain;
	gchar *username;
	gchar *authdomain;
	gchar *authuser;
	gchar *password;
	gchar *epid;
	gchar *focus_factory_uri;
	/** Allowed server events to subscribe. From register OK response. */
	GSList *allow_events;
	PurpleDnsQueryData *query_data;
	PurpleSrvQueryData *srv_query_data;
	const struct sipe_service_data *service_data;
	PurpleNetworkListenData *listen_data;
	int fd;
	int cseq;
	time_t last_keepalive;
	int registerstatus; /* 0 nothing, 1 first registration send, 2 auth received, 3 registered */
	struct sip_auth registrar;
	struct sip_auth proxy;
	struct sip_csta *csta; /* For RCC - Remote Call Control */
	gboolean reregister_set; /* whether reregister timer set */
	gboolean reauthenticate_set; /* whether reauthenticate timer set */
	gboolean subscribed; /* whether subscribed to events, except buddies presence */
	gboolean subscribed_buddies; /* whether subscribed to buddies presence */
	gboolean access_level_set; /* whether basic access level set */
	gboolean initial_state_published; /* whether we published our initial state */
	GSList *our_publication_keys; /* [MS-PRES] */
	GHashTable *our_publications; /* [MS-PRES] */
	GHashTable *subscriptions;
	int listenfd;
	int listenport;
	int listenpa;
	int contacts_delta;
	int acl_delta;
	int presence_method_version;
	gchar *status;
	gboolean is_idle;
	gboolean was_idle;
	gchar *contact;
	gchar *server_version;
	gboolean ocs2007; /*if there is support for batched category subscription [SIP-PRES]*/
	gboolean batched_support; /*if there is support for batched subscription*/
	GSList *containers; /* MS-PRES containers */
	GHashTable *buddies;
	guint resendtimeout;
	guint keepalive_timeout;
	GSList *timeouts;
	gboolean connecting;
	PurpleAccount *account;
	PurpleCircBuffer *txbuf;
	guint tx_handler;
	gchar *regcallid;
	GSList *transactions;
	GSList *sessions;
	GSList *openconns;
	GSList *groups;
	sipe_transport_type transport;
	gboolean auto_transport;
	PurpleSslConnection *gsc;
	struct sockaddr *serveraddr;
	gchar *realhostname;
	int realport; /* port and hostname from SRV record */
	gboolean processing_input;
	struct sipe_ews *ews;
	gchar *email;
};

struct sip_connection {
	int fd;
	gchar *inbuf;
	int inbuflen;
	int inbufused;
	int inputhandler;
};

struct sipe_auth_job {
	gchar * who;
	struct sipe_account_data * sip;
};

struct transaction;

typedef gboolean (*TransCallback) (struct sipe_account_data *, struct sipmsg *, struct transaction *);

struct transaction_payload {
	GDestroyNotify destroy;
	void *data;
};

struct transaction {
	time_t time;
	int retries;
	int transport; /* 0 = tcp, 1 = udp */
	int fd;
	/** Not yet perfect, but surely better then plain CSeq
	 * Format is: <Call-ID><CSeq>
	 * (RFC3261 17.2.3 for matching server transactions: Request-URI, To tag, From tag, Call-ID, CSeq, and top Via)
	 */
	gchar *key;
	struct sipmsg *msg;
	TransCallback callback;
	struct transaction_payload *payload;
};

typedef void (*Action) (struct sipe_account_data *, void *);

/**
  * Do schedule action for execution in the future.
  * Non repetitive execution.
  *
  * @param   name of action (will be copied)
  * @param   timeout in seconds
  * @param   action  callback function
  * @param   destroy payload destroy function
  * @param   sip
  * @param   payload callback data (can be NULL, otherwise caller must allocate memory)
  */
void
sipe_schedule_action(const gchar *name,
		     int timeout,
		     Action action,
		     GDestroyNotify destroy,
		     struct sipe_account_data *sip,
		     void *payload);

struct sipe_group {
	gchar *name;
	int id;
    PurpleGroup *purple_group;
};

struct group_user_context {
	gchar * group_name;
	gchar * user_name;
};

GSList * slist_insert_unique_sorted(GSList *list, gpointer data, GCompareFunc func);

GList *sipe_actions(PurplePlugin *plugin, gpointer context);

gboolean purple_init_plugin(PurplePlugin *plugin);

/**
 * Publishes self status
 * based on own calendar information,
 * our Calendar information - FreeBusy, WorkingHours,
 * OOF note.
 *
 * For 2007+
 */
void
publish_calendar_status_self(struct sipe_account_data *sip);

/** 
 * For 2005-
 */
void
send_presence_soap(struct sipe_account_data *sip,
		   const char *note,
		   gboolean do_publish_calendar);

/**
 * THE BIG SPLIT - temporary interfaces
 *
 * Previously private functions in sipe.c that are
 *  - waiting to be factored out to an appropriate module
 *  - are needed by the already created new modules
 */

/* pier11:
 *
 * Since SIP (RFC3261) is extensible by its design,
 * and MS specs prove just that (they all are defined as SIP extensions),
 * it make sense to split functionality by extension (or close extension group).
 * For example: conference, presence (MS-PRES), etc.
 *
 * This way our code will not be monolithic, but potentially _reusable_. May be
 * a top of other SIP core, and/or other front-end (Telepathy framework?).
 */
/* Forward declarations */
struct sip_session;
struct sip_dialog;

/* SIP send module? */
struct transaction *
send_sip_request(PurpleConnection *gc, const gchar *method,
		 const gchar *url, const gchar *to, const gchar *addheaders,
		 const gchar *body, struct sip_dialog *dialog, TransCallback tc);
void
send_sip_response(PurpleConnection *gc, struct sipmsg *msg, int code,
		  const char *text, const char *body);
void
sipe_invite(struct sipe_account_data *sip, struct sip_session *session,
	    const gchar *who, const gchar *msg_body,
	    const gchar *referred_by, const gboolean is_triggered);
/* ??? module */
gboolean process_subscribe_response(struct sipe_account_data *sip,
				    struct sipmsg *msg,
				    struct transaction *tc);
/* Chat module */
void
sipe_invite_to_chat(struct sipe_account_data *sip,
		    struct sip_session *session,
		    const gchar *who);
/* Session module? */
void
sipe_present_message_undelivered_err(struct sipe_account_data *sip,
				     struct sip_session *session,
				     int sip_error,
				     const gchar *who,
				     const gchar *message);

void
sipe_present_info(struct sipe_account_data *sip,
		 struct sip_session *session,
		 const gchar *message);


void
sipe_process_pending_invite_queue(struct sipe_account_data *sip,
				  struct sip_session *session);

/*** THE BIG SPLIT END ***/

#define SIPE_XML_DATE_PATTERN	"%Y-%m-%dT%H:%M:%SZ"

#define SIPE_INVITE_TEXT "ms-text-format: text/plain; charset=UTF-8%s;ms-body=%s\r\n"

#define SIPE_SEND_TYPING \
"<?xml version=\"1.0\"?>"\
"<KeyboardActivity>"\
  "<status status=\"type\" />"\
"</KeyboardActivity>"

/**
 * Publishes categories.
 * @param uri		(%s) Self URI. Ex.: sip:alice7@boston.local
 * @param publications	(%s) XML publications
 */
#define SIPE_SEND_PRESENCE \
	"<publish xmlns=\"http://schemas.microsoft.com/2006/09/sip/rich-presence\">"\
		"<publications uri=\"%s\">"\
			"%s"\
		"</publications>"\
	"</publish>"

/**
 * Publishes 'device' category.
 * @param instance	(%u) Ex.: 1938468728
 * @param version	(%u) Ex.: 1
 * @param endpointId	(%s) Ex.: C707E38E-1E10-5413-94D9-ECAC260A0269
 * @param uri		(%s) Self URI. Ex.: sip:alice7@boston.local
 * @param timezone	(%s) Ex.: 00:00:00+01:00
 * @param machineName	(%s) Ex.: BOSTON-OCS07
 */
#define SIPE_PUB_XML_DEVICE \
	"<publication categoryName=\"device\" instance=\"%u\" container=\"2\" version=\"%u\" expireType=\"endpoint\">"\
		"<device xmlns=\"http://schemas.microsoft.com/2006/09/sip/device\" endpointId=\"%s\">"\
			"<capabilities preferred=\"false\" uri=\"%s\">"\
				"<text capture=\"true\" render=\"true\" publish=\"false\"/>"\
				"<gifInk capture=\"false\" render=\"true\" publish=\"false\"/>"\
				"<isfInk capture=\"false\" render=\"true\" publish=\"false\"/>"\
			"</capabilities>"\
			"<timezone>%s</timezone>"\
			"<machineName>%s</machineName>"\
		"</device>"\
	"</publication>"

/**
 * Publishes 'machineState' category.
 * @param instance	(%u) Ex.: 926460663
 * @param version	(%u) Ex.: 22
 * @param availability	(%d) Ex.: 3500
 * @param instance	(%u) Ex.: 926460663
 * @param version	(%u) Ex.: 22
 * @param availability	(%d) Ex.: 3500
 */
#define SIPE_PUB_XML_STATE_MACHINE \
	"<publication categoryName=\"state\" instance=\"%u\" container=\"2\" version=\"%u\" expireType=\"endpoint\">"\
		"<state xmlns=\"http://schemas.microsoft.com/2006/09/sip/state\" manual=\"false\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:type=\"machineState\">"\
			"<availability>%d</availability>"\
			"<endpointLocation/>"\
		"</state>"\
	"</publication>"\
	"<publication categoryName=\"state\" instance=\"%u\" container=\"3\" version=\"%u\" expireType=\"endpoint\">"\
		"<state xmlns=\"http://schemas.microsoft.com/2006/09/sip/state\" manual=\"false\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:type=\"machineState\">"\
			"<availability>%d</availability>"\
			"<endpointLocation/>"\
		"</state>"\
	"</publication>"

/**
 * Publishes 'userState' category.
 * @param instance	(%u) User. Ex.: 536870912
 * @param version	(%u) User Container 2. Ex.: 22
 * @param availability	(%d) User Container 2. Ex.: 15500
 * @param instance	(%u) User. Ex.: 536870912
 * @param version	(%u) User Container 3.Ex.: 22
 * @param availability	(%d) User Container 3. Ex.: 15500
 */
#define SIPE_PUB_XML_STATE_USER \
	"<publication categoryName=\"state\" instance=\"%u\" container=\"2\" version=\"%u\" expireType=\"static\">"\
		"<state xmlns=\"http://schemas.microsoft.com/2006/09/sip/state\" manual=\"true\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:type=\"userState\">"\
			"<availability>%d</availability>"\
			"<endpointLocation/>"\
		"</state>"\
	"</publication>"\
	"<publication categoryName=\"state\" instance=\"%u\" container=\"3\" version=\"%u\" expireType=\"static\">"\
		"<state xmlns=\"http://schemas.microsoft.com/2006/09/sip/state\" manual=\"true\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:type=\"userState\">"\
			"<availability>%d</availability>"\
			"<endpointLocation/>"\
		"</state>"\
	"</publication>"

/**
 * An availability XML entry for SIPE_PUB_XML_STATE_CALENDAR
 * @param availability		(%d) Ex.: 6500
 */
#define SIPE_PUB_XML_STATE_CALENDAR_AVAIL \
"<availability>%d</availability>"
/**
 * An activity XML entry for SIPE_PUB_XML_STATE_CALENDAR
 * @param token			(%s) Ex.: in-a-meeting
 * @param minAvailability_attr	(%s) Ex.: minAvailability="6500"
 * @param maxAvailability_attr	(%s) Ex.: maxAvailability="8999" or none
 */
#define SIPE_PUB_XML_STATE_CALENDAR_ACTIVITY \
"<activity token=\"%s\" %s %s></activity>"
/**
 * Publishes 'calendarState' category.
 * @param instance		(%u) Ex.: 1339299275
 * @param version		(%u) Ex.: 1
 * @param uri			(%s) Ex.: john@contoso.com
 * @param start_time_str	(%s) Ex.: 2008-01-11T19:00:00Z
 * @param availability		(%s) XML string as SIPE_PUB_XML_STATE_CALENDAR_AVAIL
 * @param activity		(%s) XML string as SIPE_PUB_XML_STATE_CALENDAR_ACTIVITY
 * @param meeting_subject	(%s) Ex.: Customer Meeting
 * @param meeting_location	(%s) Ex.: Conf Room 100
 *
 * @param instance		(%u) Ex.: 1339299275
 * @param version		(%u) Ex.: 1
 * @param uri			(%s) Ex.: john@contoso.com
 * @param start_time_str	(%s) Ex.: 2008-01-11T19:00:00Z
 * @param availability		(%s) XML string as SIPE_PUB_XML_STATE_CALENDAR_AVAIL
 * @param activity		(%s) XML string as SIPE_PUB_XML_STATE_CALENDAR_ACTIVITY
 * @param meeting_subject	(%s) Ex.: Customer Meeting
 * @param meeting_location	(%s) Ex.: Conf Room 100
 */
#define SIPE_PUB_XML_STATE_CALENDAR \
	"<publication categoryName=\"state\" instance=\"%u\" container=\"2\" version=\"%u\" expireType=\"endpoint\">"\
		"<state xmlns=\"http://schemas.microsoft.com/2006/09/sip/state\" manual=\"false\" uri=\"%s\" startTime=\"%s\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:type=\"calendarState\">"\
			"%s"\
			"%s"\
			"<endpointLocation/>"\
			"<meetingSubject>%s</meetingSubject>"\
			"<meetingLocation>%s</meetingLocation>"\
		"</state>"\
	"</publication>"\
	"<publication categoryName=\"state\" instance=\"%u\" container=\"3\" version=\"%u\" expireType=\"endpoint\">"\
		"<state xmlns=\"http://schemas.microsoft.com/2006/09/sip/state\" manual=\"false\" uri=\"%s\" startTime=\"%s\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:type=\"calendarState\">"\
			"%s"\
			"%s"\
			"<endpointLocation/>"\
			"<meetingSubject>%s</meetingSubject>"\
			"<meetingLocation>%s</meetingLocation>"\
		"</state>"\
	"</publication>"

/**
 * Publishes to clear 'calendarState' category
 * @param instance		(%u) Ex.: 1251210982
 * @param version		(%u) Ex.: 1
 */ 
#define SIPE_PUB_XML_STATE_CALENDAR_CLEAR \
	"<publication categoryName=\"state\" instance=\"%u\" container=\"2\" version=\"%u\" expireType=\"endpoint\" expires=\"0\"/>"\
	"<publication categoryName=\"state\" instance=\"%u\" container=\"3\" version=\"%u\" expireType=\"endpoint\" expires=\"0\"/>"


/**
 * Publishes 'note' category.
 * @param instance	(%u) Ex.: 2135971629; 0 for personal
 * @param version	(%u) Ex.: 2
 * @param type		(%s) Ex.: personal or OOF
 * @param body		(%s) Ex.: In the office
 *
 * @param instance	(%u) Ex.: 2135971629; 0 for personal
 * @param version	(%u) Ex.: 2
 * @param type		(%s) Ex.: personal or OOF
 * @param body		(%s) Ex.: In the office
 *
 * @param instance	(%u) Ex.: 2135971629; 0 for personal
 * @param version	(%u) Ex.: 2
 * @param type		(%s) Ex.: personal or OOF
 * @param body		(%s) Ex.: In the office
 */
#define SIPE_PUB_XML_NOTE \
	"<publication categoryName=\"note\" instance=\"%u\" container=\"200\" version=\"%d\" expireType=\"static\">"\
		"<note xmlns=\"http://schemas.microsoft.com/2006/09/sip/note\">"\
			"<body type=\"%s\" uri=\"\">%s</body>"\
		"</note>"\
	"</publication>"\
	"<publication categoryName=\"note\" instance=\"%u\" container=\"300\" version=\"%d\" expireType=\"static\">"\
		"<note xmlns=\"http://schemas.microsoft.com/2006/09/sip/note\">"\
			"<body type=\"%s\" uri=\"\">%s</body>"\
		"</note>"\
	"</publication>"\
	"<publication categoryName=\"note\" instance=\"%u\" container=\"400\" version=\"%d\" expireType=\"static\">"\
		"<note xmlns=\"http://schemas.microsoft.com/2006/09/sip/note\">"\
			"<body type=\"%s\" uri=\"\">%s</body>"\
		"</note>"\
	"</publication>"

/**
 * Publishes 'calendarData' category's WorkingHours.
 *
 * @param version	        (%u)  Ex.: 1
 * @param email	                (%s)  Ex.: alice@cosmo.local
 * @param working_hours_xml_str	(%s)  Ex.: <WorkingHours xmlns=.....
 *
 * @param version	        (%u)
 *
 * @param version	        (%u)
 * @param email	                (%s)
 * @param working_hours_xml_str	(%s)
 *
 * @param version	        (%u)
 * @param email	                (%s)
 * @param working_hours_xml_str	(%s)
 *
 * @param version	        (%u)
 * @param email	                (%s)
 * @param working_hours_xml_str	(%s)
 *
 * @param version	        (%u)
 */
#define SIPE_PUB_XML_WORKING_HOURS \
	"<publication categoryName=\"calendarData\" instance=\"0\" container=\"1\" version=\"%d\" expireType=\"static\">"\
		"<calendarData xmlns=\"http://schemas.microsoft.com/2006/09/sip/calendarData\" mailboxID=\"%s\">%s"\
		"</calendarData>"\
	"</publication>"\
	"<publication categoryName=\"calendarData\" instance=\"0\" container=\"100\" version=\"%d\" expireType=\"static\">"\
		"<calendarData xmlns=\"http://schemas.microsoft.com/2006/09/sip/calendarData\"/>"\
	"</publication>"\
	"<publication categoryName=\"calendarData\" instance=\"0\" container=\"200\" version=\"%d\" expireType=\"static\">"\
		"<calendarData xmlns=\"http://schemas.microsoft.com/2006/09/sip/calendarData\" mailboxID=\"%s\">%s"\
		"</calendarData>"\
	"</publication>"\
	"<publication categoryName=\"calendarData\" instance=\"0\" container=\"300\" version=\"%d\" expireType=\"static\">"\
		"<calendarData xmlns=\"http://schemas.microsoft.com/2006/09/sip/calendarData\" mailboxID=\"%s\">%s"\
		"</calendarData>"\
	"</publication>"\
	"<publication categoryName=\"calendarData\" instance=\"0\" container=\"400\" version=\"%d\" expireType=\"static\">"\
		"<calendarData xmlns=\"http://schemas.microsoft.com/2006/09/sip/calendarData\" mailboxID=\"%s\">%s"\
		"</calendarData>"\
	"</publication>"\
	"<publication categoryName=\"calendarData\" instance=\"0\" container=\"32000\" version=\"%d\" expireType=\"static\">"\
		"<calendarData xmlns=\"http://schemas.microsoft.com/2006/09/sip/calendarData\"/>"\
	"</publication>"

/**
 * Publishes 'calendarData' category's FreeBusy.
 *
 * @param instance	        (%u)  Ex.: 1300372959
 * @param version	        (%u)  Ex.: 1
 *
 * @param instance	        (%u)  Ex.: 1300372959
 * @param version	        (%u)  Ex.: 1
 *
 * @param instance	        (%u)  Ex.: 1300372959
 * @param version	        (%u)  Ex.: 1
 * @param email	                (%s)  Ex.: alice@cosmo.local
 * @param fb_start_time_str	(%s)  Ex.: 2009-12-03T00:00:00Z
 * @param free_busy_base64	(%s)  Ex.: AAAAAAAAAAAAAAAAAAAAA.....
 *
 * @param instance	        (%u)  Ex.: 1300372959
 * @param version	        (%u)  Ex.: 1
 * @param email	                (%s)  Ex.: alice@cosmo.local
 * @param fb_start_time_str	(%s)  Ex.: 2009-12-03T00:00:00Z
 * @param free_busy_base64	(%s)  Ex.: AAAAAAAAAAAAAAAAAAAAA.....
 *
 * @param instance	        (%u)  Ex.: 1300372959
 * @param version	        (%u)  Ex.: 1
 * @param email	                (%s)  Ex.: alice@cosmo.local
 * @param fb_start_time_str	(%s)  Ex.: 2009-12-03T00:00:00Z
 * @param free_busy_base64	(%s)  Ex.: AAAAAAAAAAAAAAAAAAAAA.....
 *
 * @param instance	        (%u)  Ex.: 1300372959
 * @param version	        (%u)  Ex.: 1
 */
#define SIPE_PUB_XML_FREE_BUSY \
	"<publication categoryName=\"calendarData\" instance=\"%u\" container=\"1\" version=\"%d\" expireType=\"endpoint\">"\
		"<calendarData xmlns=\"http://schemas.microsoft.com/2006/09/sip/calendarData\"/>"\
	"</publication>"\
	"<publication categoryName=\"calendarData\" instance=\"%u\" container=\"100\" version=\"%d\" expireType=\"endpoint\">"\
		"<calendarData xmlns=\"http://schemas.microsoft.com/2006/09/sip/calendarData\"/>"\
	"</publication>"\
	"<publication categoryName=\"calendarData\" instance=\"%u\" container=\"200\" version=\"%d\" expireType=\"endpoint\">"\
		"<calendarData xmlns=\"http://schemas.microsoft.com/2006/09/sip/calendarData\" mailboxID=\"%s\">"\
			"<freeBusy startTime=\"%s\" granularity=\"PT15M\" encodingVersion=\"1\">%s</freeBusy>"\
		"</calendarData>"\
	"</publication>"\
	"<publication categoryName=\"calendarData\" instance=\"%u\" container=\"300\" version=\"%d\" expireType=\"endpoint\">"\
		"<calendarData xmlns=\"http://schemas.microsoft.com/2006/09/sip/calendarData\" mailboxID=\"%s\">"\
			"<freeBusy startTime=\"%s\" granularity=\"PT15M\" encodingVersion=\"1\">%s</freeBusy>"\
		"</calendarData>"\
	"</publication>"\
	"<publication categoryName=\"calendarData\" instance=\"%u\" container=\"400\" version=\"%d\" expireType=\"endpoint\">"\
		"<calendarData xmlns=\"http://schemas.microsoft.com/2006/09/sip/calendarData\" mailboxID=\"%s\">"\
			"<freeBusy startTime=\"%s\" granularity=\"PT15M\" encodingVersion=\"1\">%s</freeBusy>"\
		"</calendarData>"\
	"</publication>"\
	"<publication categoryName=\"calendarData\" instance=\"%u\" container=\"32000\" version=\"%d\" expireType=\"endpoint\">"\
		"<calendarData xmlns=\"http://schemas.microsoft.com/2006/09/sip/calendarData\"/>"\
	"</publication>"


#define sipe_soap(method, body) \
"<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\">" \
  "<SOAP-ENV:Body>" \
    "<m:" method " xmlns:m=\"http://schemas.microsoft.com/winrtc/2002/11/sip\">" \
      body \
    "</m:" method ">" \
  "</SOAP-ENV:Body>" \
"</SOAP-ENV:Envelope>"

#define SIPE_SOAP_SET_CONTACT sipe_soap("setContact", \
	"<m:displayName>%s</m:displayName>"\
	"<m:groups>%s</m:groups>"\
	"<m:subscribed>%s</m:subscribed>"\
	"<m:URI>%s</m:URI>"\
	"<m:externalURI />"\
	"<m:deltaNum>%d</m:deltaNum>")

#define SIPE_SOAP_DEL_CONTACT sipe_soap("deleteContact", \
	"<m:URI>%s</m:URI>"\
	"<m:deltaNum>%d</m:deltaNum>")

#define SIPE_SOAP_ADD_GROUP sipe_soap("addGroup", \
	"<m:name>%s</m:name>"\
	"<m:externalURI />"\
	"<m:deltaNum>%d</m:deltaNum>")

#define SIPE_SOAP_MOD_GROUP sipe_soap("modifyGroup", \
	"<m:groupID>%d</m:groupID>"\
	"<m:name>%s</m:name>"\
	"<m:externalURI />"\
	"<m:deltaNum>%d</m:deltaNum>")

#define SIPE_SOAP_DEL_GROUP sipe_soap("deleteGroup", \
	"<m:groupID>%d</m:groupID>"\
	"<m:deltaNum>%d</m:deltaNum>")

// first/mask arg is sip:user@domain.com
// second/rights arg is AA for allow, BD for deny
#define SIPE_SOAP_ALLOW_DENY sipe_soap("setACE", \
	"<m:type>USER</m:type>"\
	"<m:mask>%s</m:mask>"\
	"<m:rights>%s</m:rights>"\
	"<m:deltaNum>%d</m:deltaNum>")

/**
 * Calendar publication entry. 2005 systems.
 *
 * @param legacy_dn		(%s) Ex.: /o=EXCHANGE/ou=BTUK02/cn=Recipients/cn=AHHBTT
 * @param fb_start_time_str	(%s) Ex.: 2009-12-06T17:15:00Z
 * @param free_busy_base64	(%s) Ex.: AAAAAAAAAAAAAAAAA......
 */
#define SIPE_SOAP_SET_PRESENCE_CALENDAR \
"<calendarInfo xmlns=\"http://schemas.microsoft.com/2002/09/sip/presence\" mailboxId=\"%s\" startTime=\"%s\" granularity=\"PT15M\">%s</calendarInfo>"
/**
 * Note publication entry. 2005 systems.
 *
 * @param note	(%s) Ex.: Working from home
 */
#define SIPE_SOAP_SET_PRESENCE_NOTE_XML  "<note>%s</note>"
/**
 * Note's OOF publication entry. 2005 systems.
 */
#define SIPE_SOAP_SET_PRESENCE_OOF_XML  "<oof></oof>"
/**
 * States publication entry for User State. 2005 systems.
 *
 * @param avail			(%d) Availability 2007-style. Ex.: 9500
 * @param since_time_str	(%s) Ex.: 2010-01-13T10:30:05Z
 * @param device_id		(%s) epid. Ex.: 4c77e6ec72
 * @param activity_token	(%s) Ex.: do-not-disturb
 */
#define SIPE_SOAP_SET_PRESENCE_STATES \
          "<states>"\
            "<state avail=\"%d\" since=\"%s\" validWith=\"any-device\" deviceId=\"%s\" set=\"manual\" xsi:type=\"userState\">%s</state>"\
          "</states>"
/**
 * Presentity publication entry. 2005 systems.
 *
 * @param uri			(%s) SIP URI without 'sip:' prefix. Ex.: fox@atlanta.local
 * @param aggr_availability	(%d) Ex.: 300
 * @param aggr_activity		(%d) Ex.: 600
 * @param host_name		(%s) Uppercased. Ex.: ATLANTA
 * @param note_xml_str		(%s) XML string as SIPE_SOAP_SET_PRESENCE_NOTE_XML
 * @param oof_xml_str		(%s) XML string as SIPE_SOAP_SET_PRESENCE_OOF_XML
 * @param states_xml_str	(%s) XML string as SIPE_SOAP_SET_PRESENCE_STATES
 * @param calendar_info_xml_str	(%s) XML string as SIPE_SOAP_SET_PRESENCE_CALENDAR
 * @param device_id		(%s) epid. Ex.: 4c77e6ec72
 */
#define SIPE_SOAP_SET_PRESENCE sipe_soap("setPresence", \
	"<m:presentity xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" m:uri=\"sip:%s\">"\
	"<m:availability m:aggregate=\"%d\"/>"\
	"<m:activity m:aggregate=\"%d\"/>"\
	"<deviceName xmlns=\"http://schemas.microsoft.com/2002/09/sip/presence\" name=\"%s\"/>"\
	"<rtc:devicedata xmlns:rtc=\"http://schemas.microsoft.com/winrtc/2002/11/sip\" namespace=\"rtcService\">"\
	"<![CDATA[<caps><renders_gif/><renders_isf/></caps>]]></rtc:devicedata>"\
	"<userInfo xmlns=\"http://schemas.microsoft.com/2002/09/sip/presence\">"\
	"%s%s" \
	"%s" \
        "</userInfo>"\
	"%s" \
	"<device xmlns=\"http://schemas.microsoft.com/2002/09/sip/presence\" deviceId=\"%s\" />"\
	"</m:presentity>")

#define SIPE_SOAP_SEARCH_CONTACT \
	"<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\">" \
	"<SOAP-ENV:Body>" \
	"<m:directorySearch xmlns:m=\"http://schemas.microsoft.com/winrtc/2002/11/sip\">" \
	"<m:filter m:href=\"#searchArray\"/>"\
	"<m:maxResults>%d</m:maxResults>"\
	"</m:directorySearch>"\
	"<m:Array xmlns:m=\"http://schemas.microsoft.com/winrtc/2002/11/sip\" m:id=\"searchArray\">"\
	"%s"\
	"</m:Array>"\
	"</SOAP-ENV:Body>"\
	"</SOAP-ENV:Envelope>"

#define SIPE_SOAP_SEARCH_ROW "<m:row m:attrib=\"%s\" m:value=\"%s\"/>"

#endif /* _PIDGIN_SIPE_H */
