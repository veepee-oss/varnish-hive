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
 *	    David Sebaoun <david.sebaoun@epitech.eu>
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

#include "config.h"

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <pwd.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#include "vapi/vsl.h"
#include "vapi/vsm.h"
#include "vapi/voptget.h"
#include "vas.h"
#include "vdef.h"
#include "vtree.h"
#include "vsb.h"
#include "vut.h"
#include "varnishhive_network.h"
#include "varnishhive_parser.h"
#include "varnishhive_log.h"


#if 0
#define AC(x) assert((x) != ERR)
#else
#define AC(x) x
#endif

#ifndef VARNISHHIVE_USER
#define VARNISHHIVE_USER "varnishhive"
#endif

static const char progname[] = "varnishhive";
static volatile sig_atomic_t quit = 0;
static volatile int keepRunning = 1;
static int discuss_socket = -1;
static hive_data *data = NULL;
varnishhive_log logger = { NULL, NULL, 0 };

#define USER_AGENT "varnish"
#define MAXLEN_URI 2048
#define MAXLEN_HOST 253
#define MAXLEN_ACCEPT 4096
#define MAXLEN_ACCEPT_LANG 4096
#define MAXLEN_HTTP_VERSION 10

struct http_headers {
	char	uri[MAXLEN_URI];
	char	host[MAXLEN_HOST];
	char	accept[MAXLEN_ACCEPT];
	char	accept_lang[MAXLEN_ACCEPT_LANG];
	char	http_version[MAXLEN_HTTP_VERSION];
};

static const char *he[] = {
	"HOST",
	"ACCEPT",
	"ACCEPT-LANGUAGE",
	"USER-AGENT",
	NULL
};

/*
 * Jails the child process and dropes its privileges
 * if user varnishhive doesn't exist, root privileges will be kept
 */
static int
drop_privileges() {
	struct passwd *user;

	if ((user = getpwnam(VARNISHHIVE_USER)) == NULL) {
		syslog(LOG_ERR, "user %s not found", VARNISHHIVE_USER);
                verbose(0, "user %s not found", VARNISHHIVE_USER);
		return (1);
	}
	if (chroot(user->pw_dir) < 0 || chdir("/") < 0) {
		syslog(LOG_ERR, "Error while applying chroot. Please chech that %s's home directory exits", VARNISHHIVE_USER);
                verbose(0, "Error while applying chroot. Please check that %s's home directory exits", VARNISHHIVE_USER);
		return (1);
	}
	if (setgid(user->pw_gid) < 0 || setuid(user->pw_uid) < 0)
		return (1);
	if (setuid(0) == 0 || seteuid(0) == 0) {
		return (1);
	}
	return (0);
}

static int
http_header_handler(struct http_headers *h, const char *data)
{
	char *token;
	char *value;
	char *entry = strdup(data);

	if ((token = strtok(entry, ":")) == NULL)
		return (1);
	for (int i = 0; he[i] != NULL; ++i) {
		if (strcasecmp(he[i], token) == '\0') {
			value = entry + strlen(token) + 1;
			if (strlen(value) > 0 && value[0] == ' ')
				++value;
			switch (i) {
			case 0:
				if (strncpy(h->host, value, MAXLEN_HOST - 1) == NULL)
					return (1);
				break;
			case 1:
				if (strncpy(h->accept, value, MAXLEN_ACCEPT - 1) == NULL)
					return (1);
				break;
			case 2:
				if (strncpy(h->accept_lang, value, MAXLEN_ACCEPT_LANG - 1) == NULL)
					return (1);
				break;
			case 3:
				if (strcmp(value, USER_AGENT) == 0) {
// 					fprintf(stdout, "request from other varnish\n");
					return (1);
				}
				break;
			}
			return (0);
		}
	}
	return (0);
}

static int __match_proto__(VSLQ_dispatch_f)
	recv_element(struct VSL_data *vsl, struct VSL_transaction * const pt[],
		     void *priv)
{
	struct VSL_transaction *tr;
	struct http_headers h;
	char is_backend_fetch = 0;

	h.host[0] = '\0';
	for (tr = pt[0]; tr != NULL; tr = *++pt) {
		while (VSL_Next(tr->c) == 1) {
			if (!VSL_Match(vsl, tr->c))
				continue;
			switch(VSL_TAG(tr->c->rec.ptr)) {
			case 32: /* Save the URI */
				if (strncpy(h.uri, VSL_CDATA(tr->c->rec.ptr), MAXLEN_URI - 1) == NULL)
					return (0);
				break;
			case 33: /* Save the HTTP protocol header */
				if (strncpy(h.http_version, VSL_CDATA(tr->c->rec.ptr), MAXLEN_HTTP_VERSION - 1) == NULL)
					return (0);
				break;
			case 36: /* Save the Host */
				if (http_header_handler(&h, VSL_CDATA(tr->c->rec.ptr)) == 1)
					return (0);
				break;
			case 50: /* Drop request if fetch from backend not successful */
				if (atoi(VSL_CDATA(tr->c->rec.ptr)) >= 400)
					return (0);
				break;
			case 60: /* Drop request if not a BACKEND_FETCH */
				if (strncmp(VSL_CDATA(tr->c->rec.ptr), "BACKEND_", strlen("BACKEND_")) != 0)
					return (0);
				if (strcmp(VSL_CDATA(tr->c->rec.ptr), "BACKEND_FETCH") == 0)
					is_backend_fetch = 1;
				break;
			case 76: /* Drop request if not a fetch ex: if rxreq */
				if (strstr(VSL_CDATA(tr->c->rec.ptr), "fetch") == NULL)
					return (0);
				break;
			}
		}
	}

	int len = sizeof(struct http_headers);
	unsigned char *msg;

	if ((msg = malloc(len)) == NULL)
		exit(1);
	memcpy(msg, &h, len);
	if (is_backend_fetch == 1) {
		if (write(discuss_socket, msg, len) <= 0)
			return (0);
	}
	free(msg);
	return (0);
}

/*
 * Interprets the hive structure
 */
#define LEN_MSG 80 /* LEN OF GET request without params */
static void
send_and_recv(struct http_headers h)
{
	char *msg;
	unsigned int len = strlen(h.uri) + strlen(h.http_version) + strlen(h.host) + strlen(h.accept) +
	  strlen(h.accept_lang) + LEN_MSG;

        if ((msg = malloc(sizeof(char) * (len + 1))) == NULL)
                exit(1);
        snprintf(msg, len, "GET %s %s\r\nHost: %s\r\nUser-Agent: varnish\r\nAccept: %s\r\nAccept-Language: %s\r\n\r\n",
		 h.uri, h.http_version, h.host, h.accept, h.accept_lang);

        verbose(-3, "send msg: %s\n", msg);

	hive_send_message_to_varnish(msg);
	free(msg);
}

/*
 * Concatenate the different part of the varnish read from the socketpair
 */
static unsigned char *
concat_hive(unsigned char *dest, const unsigned char *src, unsigned int start, unsigned int len)
{
	unsigned int i = 0;

	for (i = 0; i < len; ++i) {
		dest[start + i] = src[i];
	}
	return (dest);
}

/*
 * Get server socket for dynamic binding
 */
static int
get_socket(int port)
{
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in sin = {0};

	if (sock == -1) {
		fprintf(stderr, "Can't create socket\n");
		return (-1);
	}

	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0) {
		fprintf(stderr, "setsockopt(SO_REUSEADDR) failed");
		return (-1);
	}

	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	if (bind (sock, (struct sockaddr *)&sin, sizeof(sin)) == -1) {
		fprintf(stderr, "Can't bind to port %d\n", port);
		return (-1);
	}
	if (listen(sock, 20) == -1) {
		fprintf(stderr, "Can't listen\n");
		return (-1);
	}
	return (sock);
}

/*
 * Whenever hive_socket has something to read
 */
static int
add_to_list(int fd)
{
	int sock;
        struct sockaddr_in sin = { 0 };
	unsigned int sinsize = sizeof(sin);
	int lenRecv = 0;
	char buffer[20];

	sock = accept(fd, (struct sockaddr *)&sin, &sinsize);
	if (sock == -1) {
		fprintf(stderr, "Can't accept socket\n");
		return (-1);
	}

	lenRecv = recv(sock, buffer, sizeof(buffer), 0);
	if (lenRecv > 19)
		lenRecv = 19;
	buffer[lenRecv] = '\0';

	char *ip = inet_ntoa(sin.sin_addr);
        verbose(-1, "Dynamic binding for %s:%s\n", ip, buffer);
	syslog(LOG_NOTICE, "Dynamic binding for %s:%s\n", ip, buffer);

	/* add_to_varnish_list(ip, buffer); */
	return (0);
}

/*
 * Main loop, read in the fifo and transforms it in struct hive
 */
static void
turn_forever(int fd_socket, int disc_socket)
{
	const int len = sizeof(struct http_headers);
	unsigned char *read_msg = malloc(len);
	unsigned char *msg = malloc(len);
	struct http_headers h;
	int len_read = 0;
	unsigned int check_read = 0;
	int nb_sockets_ready;
	struct pollfd fds[2];

	if (read_msg == NULL || msg == NULL)
		exit(1);

	if (drop_privileges() == 1) {
		syslog(LOG_ERR,"fatal error while dropping privileges");
		exit(1);
	}

	fds[0].fd = fd_socket;
	fds[0].events = POLLIN;
	fds[1].fd = disc_socket;
	fds[1].events = POLLIN;
	syslog(LOG_NOTICE, "ready");
	while (keepRunning && !quit) {
		check_read = 0;
		msg[0] = '\0';
		fds[0].revents = 0;
		fds[1].revents = 0;
		nb_sockets_ready = poll(fds, 2, -1); /* infinite timeout */
		if (nb_sockets_ready <= 0) {
			syslog(LOG_ERR, "No sockets available\n");
			exit(-1);
		}
		if (fds[0].revents == POLLIN) {
			add_to_list(fds[0].fd);
		} else if (fds[1].revents == POLLIN) {
			while (check_read != len) {
				if ((len_read = read(fds[1].fd, read_msg, len - check_read)) <= 0) {
					return ;
				}
				msg = concat_hive(msg, read_msg, check_read, len_read);
				check_read += len_read;
			}
			memcpy(&h, msg, len);
			/* TODO: thread this shit so it replicates soooooooo fast*/
			send_and_recv(h);
		}
	}
	free(read_msg);
	free(msg);
}

static int __match_proto__(VUT_cb_f)
     sighup(void)
{
	quit = 1;
	return (1);
}

static void
usage(int status)
{
	const char **opt;
	fprintf(stderr, "Usage: %s <options>\n\n", progname);
	fprintf(stderr, "Options:\n");
	for (opt = vopt_usage; *opt != NULL; opt += 2)
		fprintf(stderr, "  %-25s %s\n", *opt, *(opt + 1));
	exit(status);
}

static void
chldHandler(int sig)
{
	(void)sig;
	syslog(LOG_ERR,"stopping: child process died");
	exit(1);
}

static void
intHandler(int sig)
{
	(void)sig;
	keepRunning = 0;
	exit(0);
}

int
main(int argc, char **argv)
{
	int opt;
	int p_opt = 0;
	char *f_opt = NULL;
        int v_opt = 0;
        char *l_opt = NULL;
	int sock_fd;
	int sock_disc;
	int fd[2];
	pid_t pid;

	signal(SIGINT, intHandler);
	signal(SIGCHLD, chldHandler);
	openlog ("varnishhive", LOG_PID, LOG_DAEMON);
	syslog(LOG_NOTICE,"launched");

	VUT_Init(progname);

	while ((opt = getopt(argc, argv, vopt_optstring)) != -1) {
		switch (opt) {
		case 'f':
			/* Path to configuration file */
			f_opt = strdup(optarg);
			break;
		case 'p':
			/* Port on which varnish runs */
			if ((p_opt = atoi(optarg)) == 0)
				usage(0);
			break;
                case 'l':
                        /* Log file */
                        l_opt = strdup(optarg);
                        break;
                case 'v':
                        /* Verbosity */
                        if ((v_opt = atoi(optarg)) == 0)
                          usage(0);
                        setVerbosityLevel(v_opt);
                        break;
		case 'h':
			/* Usage help */
			usage(0);
			break;
		default:
			if (!VUT_Arg(opt, optarg))
				usage(1);
		}
	}

	if (optind != argc)
		usage(1);

	if (hive_parser(&data, f_opt != NULL ? f_opt : "/etc/varnish/varnishhive.ini") == -1)
		exit(1);

        if ( l_opt != NULL ) {
          logger.lf = l_opt;
          openlogfile(0);
        }

	sock_fd = get_socket(p_opt != 0 ? p_opt : data->config_hive->listener);

	if (socketpair(PF_LOCAL, SOCK_STREAM, 0, fd) == -1) {
		syslog(LOG_ERR, "socketpair failed");
		exit(1);
	}
	sock_disc = fd[0];
	discuss_socket = fd[1];

	if (connect_all_hive(data->config_node, data->config_hive) == -1)
		exit(1);

	VUT_Setup();

	pid = fork();
	if (pid < 0) {
		syslog(LOG_ERR, "could not create child");
		syslog(LOG_NOTICE, "stopped");
		exit(1);
	}
	if (pid == 0) {
		close(discuss_socket);
		syslog(LOG_NOTICE, "child process started");
		turn_forever(sock_fd, sock_disc);
	} else {
		close(sock_disc);
		syslog(LOG_NOTICE, "dad process started");
		VUT.dispatch_f = recv_element;
		VUT.dispatch_priv = NULL;
		VUT.sighup_f = sighup;
                if ( l_opt != NULL ) {
                  VUT.sighup_f = rotatelogfile;
                }
		VUT_Main();
		VUT_Fini();
		syslog(LOG_NOTICE, "stopped");
	}

	if ( l_opt != NULL ) {
          flushlogfile();
          closelogfile();
        }

	exit(0);
}
