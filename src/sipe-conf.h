/**
 * @file sipe-conf.h
 *
 * pidgin-sipe
 *
 * Copyright (C) 2009 pier11 <pier11@kinozal.tv>
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

/* Forward declaration */
struct sip_session;

/**
 * Processes incoming INVITE with 
 * Content-Type: application/ms-conf-invite+xml
 * i.e. invitation to join conference.
 *
 * Server 2007+ functionality.
 */
void
process_incoming_invite_conf(struct sipe_account_data *sip,
			     struct sipmsg *msg);
			     
/** 
 * Process of conference state
 * Content-Type: application/conference-info+xml
 */
void
sipe_process_conference(struct sipe_account_data *sip,
			struct sipmsg * msg);

/** 
 * Invokes when we leave conversation.
 * Usually by closing chat wingow.
 */
void
conf_session_close(struct sipe_account_data *sip,
		   struct sip_session *session);

/** 
 * Invoked to process message delivery notification
 * in conference.
 */		   
void
sipe_process_imdn(struct sipe_account_data *sip,
		  struct sipmsg *msg);
