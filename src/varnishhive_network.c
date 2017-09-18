/*-
 *   VARNISH HIVE
 *      ______
 *     /      \
 *     \______/
 *   /\  ____  /\
 *   \ \ \  / / /
 *    \ \ \/ / /
 *     \_\  /_/
 *
 * Copyright (c) 2017 vente-privee
 * All rights reserved.
 *
 * Authors: Louis Giesen <louis.giesen@epitech.eu>
 *	    Sami Farhane <sami.fahrane@epitech.eu>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Replicate varnish cache between many Varnish
 */

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#include <netdb.h>
#include <stdio.h>
#include <syslog.h>
#include "varnishhive_network.h"
#include "varnishhive_parser.h"
#include "varnishhive_log.h"

static hive_list	*list = NULL;

extern varnishhive_log logger;

static int
send_message(int sock, char *msg)
{
	ssize_t		len_send = 0;
	ssize_t		len = 0;
	int		size = strlen(msg);

        verbose(4, "msg: %s\n", msg);
	for (; len != size; len += len_send) {
		if ((len_send = write(sock, msg + len, strlen(msg) - len)) == -1) {
			syslog(LOG_ERR, "Failed to send message\n");
			return (-1);
		}
		if (len_send == 0)
			break;
	}
	return (0);
}

void
add_to_varnish_list(char *ip, char *port)
{
	insert_in_list(&list, (short)atoi(port), strdup(ip), IP_ADRESS);
}

static int
hive_socket()
{
	int sock;

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		fprintf(stderr, "Creation socket Failed\n");
	return (sock);
}

static int
hive_connect_to_varnish(int sock, node *data, int port)
{
	int			serv;
	struct sockaddr_in	addr;

	if (data->type == DOMAIN_NAME) {
		struct hostent *host = NULL;

		if ((host = gethostbyname(data->host)) == NULL) {
			fprintf(stderr, "Failed host by name = %s\n", data->host);
                        verbose(0, "Failed host by name = %s\n", data->host );
			return (-1);
		}
		addr.sin_addr = *(struct in_addr *)host->h_addr;
	}
	else if (data->type == IP_ADRESS)
		addr.sin_addr.s_addr = inet_addr(data->host);
	if (port > 0) {
                verbose(4,"port: %d", port);
		addr.sin_port = htons(port);
        } else {
                verbose(4,"data->port: %d", data->port);
		addr.sin_port = htons(data->port);
        }
	addr.sin_family = AF_INET;

        verbose(-4, "Connect to Varnish %s:%d", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

        if ((serv = connect(sock, (struct sockaddr *)&addr, sizeof(addr))) == -1) {
          syslog(LOG_ERR, "Failed connect to server %s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port) );
          verbose(0, "Failed connect to server %s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port) );
          return (-1);

	}
	return (sock);
}

int
connect_all_hive(hive_list *nodes, hive *hive)
{
// 	int sock;
// 	int send;
// 	node *tmp;
// 	char port[10];

	list = nodes;
// 	tmp = list->last;
// 	for ( ; tmp != NULL && tmp->host != NULL; tmp = tmp->next) {
//                 verbose(3,"tmp->host: %s - tmp->port: %d\n", tmp->host, tmp->port);
//
// 		if ((sock = hive_socket()) == -1)
// 			return (-1);
//
//                 verbose(3,"tmp->host: %s - hive->listener: %d\n", tmp->host, hive->listener);
// 		sock = hive_connect_to_varnish(sock, tmp, hive->listener);
// 		if (sock == -1)
// 			continue ;
// 		snprintf(port, 9, "%d", hive->listener);
// 		send = send_message(sock, port);
// 		(void)send;
// 		close(sock);
// 	}

	return (0);
}

int
hive_send_message_to_varnish(char *msg)
{
	int	send;
	int	sock;
	node	*tmp = list->last;

	for ( ; tmp != NULL && tmp->host != NULL; tmp = tmp->next) {
		if ((sock = hive_socket()) == -1)
			return (-1);
		sock = hive_connect_to_varnish(sock, tmp, -1);
		if (sock == -1)
			continue ;
                verbose(-4,"target: %s:%d - msg: %s\n", tmp->host, tmp->port, msg);
		/* syslog(LOG_NOTICE, "target: %s:%d - msg: %s\n", tmp->host, tmp->port, msg); */

		send = send_message(sock, msg);
		(void)send;
		close(sock);
	}
	syslog(LOG_NOTICE, "leave send_message\n");
	return (0);
}
