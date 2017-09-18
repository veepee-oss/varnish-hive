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

#ifndef		HIVE_PARSER_H
# define	HIVE_PARSER_H

# define	BUFF_MAX	100
# define	MATCH		0
# define	NON_MATCH	1
# define	WAIT_MATCH	2
# define	PARSER_SUCCESS	0

# define	NB_HIVE_DATA	5

# define	STR_PORT		":port"
# define	STR_HOST		":host"
# define	STR_VARNISH_PORT	":varnish_port"
# define	STR_DISCUSS		":hive_listener"
# define	STR_HIVE		"hive"

# include	<regex.h>
# include	<stdio.h>
# include	<stdlib.h>
# include	<string.h>
# include	<time.h>
# include	<unistd.h>

# include	"dictionary.h"
# include	"iniparser.h"

typedef enum	format format;
enum		format
{
	IP_ADRESS,
	DOMAIN_NAME,
	NONE
};

typedef struct  hive hive;
struct		hive
{
	short		port;
	char		*host;
	format		type;
	short		varnish_port;
	short		listener;
};

typedef struct  node node;
struct		node
{
	short		port;
	char		*host;
	format		type;
	node		*next;
};

typedef struct  req_ex req_ex;
struct		req_ex
{
	const char	*regex;
	regex_t		preg;
	int		err;
	int		match;
};

typedef struct  hive_list hive_list;
struct		hive_list
{
	node		*last;
};

typedef struct  hive_data hive_data;
struct		hive_data
{
	hive		*config_hive;
	hive_list	*config_node;
};


int		hive_parser(hive_data **data, char *file_name);
void		insert_in_list(hive_list **list, short new_port, char *new_content, format new_type);

#endif		/* HIVE_PARSER_H */
