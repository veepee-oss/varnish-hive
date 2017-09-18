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
 * Author: Louis Giesen <louis.giesen@epitech.eu>
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


#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdarg.h>
#include <time.h>
#include <varnishhive_log.h>

extern varnishhive_log logger;

#define MAXLEN_ACCEPT 4096

void
openlogfile(int append)
{
  if( access( logger.lf, F_OK ) != -1 ) {
    append = 1;
  }

  logger.fo = fopen(logger.lf, append ? "a" : "w");
  if (logger.fo == NULL)
    fprintf(stderr, "Can't open output file (%s)", strerror(errno));
}

int
flushlogfile(void)
{
  if ( fflush( logger.fo ) ) {
    return 0;
  }
  return errno;
}

int
closelogfile(void)
{
  fclose( logger.fo );

  return(0);
}

int
rotatelogfile(void)
{
  verbose(-1,"SIGHUP receive - close and reopen logfile.");
  closelogfile();
  openlogfile(1);

  return(0);
}

void
setVerbosityLevel(int verbosity_level)
{
  logger.verbosity_level = verbosity_level;
}

int
verbose(int verbosity_level, const char * format, ...)
{
  bool log2file = false;
  if ( verbosity_level <= 0  ) {
    log2file = true;
    verbosity_level = abs( verbosity_level );
  }
  if ( verbosity_level > logger.verbosity_level ) return 0;

  time_t current_time;
  char* c_time_string;

  /* Obtain current time. */
  current_time = time(NULL);

  /* Convert to local time format. */
  c_time_string = ctime(&current_time);
  c_time_string[strlen(c_time_string) -1] = 0;

  va_list args;
  va_start (args, format);
  char formatted_string[MAXLEN_ACCEPT];
  int ret = vsprintf (formatted_string, format, args);
  va_end (args);

  // Remove \r\n
  char *formatted_string2;
  if ( ( formatted_string2 = malloc( sizeof(char) * (strlen(formatted_string) + 1)) ) != NULL) {
    formatted_string2 = strip_http_header_newline(formatted_string);
    // Remove trailing carriage return
    formatted_string2 = strip_copy(formatted_string2);
    // log to stdout
    if ( verbosity_level <= logger.verbosity_level ) {
      printf("%s - %s\n", c_time_string, formatted_string2);
    }
    // log to file
    if (log2file && logger.fo != NULL ) {
      fprintf(logger.fo, "%s - %s\n", c_time_string, formatted_string2);
      flushlogfile();
    }

    free(formatted_string2);
  }

  return ret;
}

char *
strip_copy(const char *s)
{
  char *p = malloc(strlen(s) + 1);
  if(p) {
    char *p2 = p;
    while(*s != '\0') {
      if(*s != '\r' && *s != '\n') {
        *p2++ = *s++;
      } else {
        ++s;
      }
    }
    *p2 = '\0';
  }
  return p;
}

char *
strip_http_header_newline(const char *s)
{
  char *p = malloc(strlen(s) + 1);
  char *p2 = p;
  if(p) {
    for ( size_t i = 0; i < strlen(s) - 2; i++) {
      if ( s[i] != '\r' && s[i+1] != '\n') {
        *p2++ = s[i];
        if ( i == strlen(s) - 3 ) {
          *p2++ = s[i+1];
        }
      } else {
        *p2++ = ' ';
        i++;
      }
    }
    *p2++ = s[strlen(s)-1];
    *p2 = '\0';
  }
  return p;
}
