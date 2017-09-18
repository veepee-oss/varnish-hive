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
 * Author: David Sebaoun <david.sebaoun@epitech.eu>
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

#include "vapi/vapi_options.h"
#include "vut_options.h"

#define HIVE_OPT_f							\
	VOPT("f:", "[-f]", "Path to configuration file", "")

#define HIVE_OPT_p							\
	VOPT("p:", "[-p]", "Port to listen on",				\
	     "If used, the port in the configuration file will be"	\
	     "overriden."						\
	)

#define HIVE_OPT_l							\
        VOPT("l:", "[-l]", "Path to log file", "")

#define HIVE_OPT_v							\
        VOPT("v:", "[-v]", "Verbosity level (must be above 0)", "")

HIVE_OPT_f
HIVE_OPT_p
HIVE_OPT_l
HIVE_OPT_v
VUT_OPT_D
VUT_OPT_h
