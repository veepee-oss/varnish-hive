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
 * Authors: Sami Farhane <sami.farhane@epitech.eu>
 *          Sabri Abdellatif <sabri.abdellatif@epitech.eu>
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

#include "varnishhive_parser.h"

static int
init_list(hive_list **list)
{
	(*list) = malloc(sizeof(hive_list));
	(*list)->last = malloc(sizeof(node));

	if ((*list) == NULL || (*list)->last == NULL)
		exit(1);

	(*list)->last->port = 0;
	(*list)->last->host = NULL;
	(*list)->last->type = NONE;
	(*list)->last->next = NULL;
	return (0);
}

static int
init_hive(hive **hiv)
{
	if (((*hiv) = malloc(sizeof(hive))) == NULL)
		exit(1);
	(*hiv)->port = 0;
	(*hiv)->host = NULL;
	(*hiv)->type = NONE;
	(*hiv)->varnish_port = 0;
	return (0);
}

static void
init_hive_data(hive_data **data)
{
	if (((*data) = malloc(sizeof(hive_data))) == NULL)
		exit(1);
	init_hive(&(*data)->config_hive);
	init_list(&(*data)->config_node);
}

void
insert_in_list(hive_list **list, short new_port, char *new_content, format new_type)
{
	node *new_elem = malloc(sizeof(node));

	if (list == NULL || new_elem == NULL)
		exit(1);

	new_elem->port = new_port;
	if ((new_elem->host = strdup(new_content)) == NULL)
		exit(1);
	new_elem->type = new_type;
	new_elem->next = (*list)->last;
	(*list)->last = new_elem;
}

static req_ex
req_ex_init(char *expression)
{
	req_ex	new;

	new.regex = expression;
	new.err	= 0;
	new.match = WAIT_MATCH;
	return (new);
}

static int
check_hive(dictionary *dico)
{
	int	port = 0;
	int	varnish = 0;
	int	host = 0;
	int	listener = 0;
	int	i = 0;

	if ((strcmp(STR_HIVE, dico->key[0]) == 0) && dico->val[0] == NULL) {
		for (; i < NB_HIVE_DATA; i++) {
			if (strstr(dico->key[i], STR_PORT) != NULL)
				port = 1;
			else if (strstr(dico->key[i], STR_HOST) != NULL)
				host = 1;
			else if (strstr(dico->key[i], STR_VARNISH_PORT) != NULL)
				varnish = 1;
			else if (strstr(dico->key[i], STR_DISCUSS))
				listener = 1;
		}
		if (port == 1 && host == 1 && varnish == 1 && listener == 1)
			return (0);
		else {
			fprintf(stderr, "No port or host or varnish port or listener in [hive] section\n");
			return (-1);
		}
	}
	fprintf(stderr, "No [hive] in first line of the file\n");
	return (-1);
}

static int
check_nodes(dictionary *dico)
{
	int	j;
	int	port;
	int	host;
	int	i = NB_HIVE_DATA;

	for (; dico->key[i] != NULL; i += 3) {
		if (dico->val[i] == NULL) {
			port = 0;
			host = 0;
			for (j = 1; j < 3; j++) {
				if (strstr(dico->key[i + j], STR_PORT) != NULL)
					port = 1;
				else if (strstr(dico->key[i + j], STR_HOST) != NULL)
					host = 1;
			}
			if (port != 1 || host != 1) {
				fprintf(stderr, "One node is not set correctly, check .ini\n");
				return (-1);
			}
		}
		else {
			fprintf(stderr, "Node name has a value, check .ini\n");
			return (-1);
		}
	}
	return (0);
}

int
hive_parser(hive_data **data, char *file_name)
{
	dictionary	*ini;
	int		i = 0;
	int		data_node = 0;
	short		port = 0;
	char		*host = NULL;
	format		format = NONE;
	req_ex		ip = req_ex_init("^[0-9]{1,3}[.][0-9]{1,3}[.][0-9]{1,3}[.][0-9]{1,3}+\0");
	req_ex		domain = req_ex_init("^[a-zA-Z0-9-]{2,63}([.][a-zA-Z0-9-]{2,63})+\0");

	ip.err = regcomp(&ip.preg, ip.regex, REG_NOSUB | REG_EXTENDED);
	domain.err = regcomp(&domain.preg, domain.regex, REG_NOSUB | REG_EXTENDED);
	if ((ip.err != 0) || (domain.err != 0)) {
		printf("ERROR : Bad Regex expression.\n\n");
		return (-1);
	}
	if ((ini = iniparser_load(file_name)) == NULL)
		return (-1);
	init_hive_data(data);
	if (check_hive(ini) != 0 || check_nodes(ini) != 0)
		return (-1);
	for (; ini->key[i] != NULL; i++) {
		ip.match = WAIT_MATCH;
		domain.match = WAIT_MATCH;
		if (i < NB_HIVE_DATA && i > 0) {
			if (strstr(ini->key[i], STR_PORT) != NULL)
				(*data)->config_hive->port = atoi(ini->val[i]);
			else if (strstr(ini->key[i], STR_HOST) != NULL) {
				ip.match = regexec(&ip.preg, ini->val[i], 0, NULL, 0);
				domain.match = regexec(&domain.preg, ini->val[i], 0, NULL, 0);
				if (ip.match == MATCH)
					(*data)->config_hive->type = IP_ADRESS;
				else if (domain.match == MATCH)
					(*data)->config_hive->type = DOMAIN_NAME;
				(*data)->config_hive->host = ini->val[i];
			}
			else if (strstr(ini->key[i], STR_VARNISH_PORT) != NULL)
				(*data)->config_hive->varnish_port = atoi(ini->val[i]);
			else if (strstr(ini->key[i], STR_DISCUSS) != NULL)
				(*data)->config_hive->listener = atoi(ini->val[i]);
		}
		else if (i >= NB_HIVE_DATA) {
			if (strstr(ini->key[i], STR_PORT) != NULL) {
				port = atoi(ini->val[i]);
				data_node += 1;
			}
			else if (strstr(ini->key[i], STR_HOST) != NULL) {
				ip.match = regexec(&ip.preg, ini->val[i], 0, NULL, 0);
				domain.match = regexec(&domain.preg, ini->val[i], 0, NULL, 0);
				if (ip.match == MATCH)
					format = IP_ADRESS;
				else if (domain.match == MATCH)
					format = DOMAIN_NAME;
				host = ini->val[i];
				data_node += 1;
			}
			if (data_node == 2) {
				insert_in_list(&(*data)->config_node, port, host, format);
				data_node = 0;
				port = 0;
				format = NONE;
				host = NULL;
			}
		}
	}
	return (PARSER_SUCCESS);
}
