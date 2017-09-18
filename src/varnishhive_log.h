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

#ifndef VARNISHHIVELOG_H_
#define VARNISHHIVELOG_H_

typedef struct varnishhive_log varnishhive_log;
struct varnishhive_log {
  FILE *fo;
  char* lf;
  int verbosity_level;
};

void openlogfile(int append);
int flushlogfile(void);
int closelogfile(void);
int rotatelogfile(void);

void setVerbosityLevel(int verbosity_level);
int verbose(int verbosity_level, const char * format, ...);

char *strip_copy(const char *s);
char *strip_http_header_newline(const char *s);
#endif