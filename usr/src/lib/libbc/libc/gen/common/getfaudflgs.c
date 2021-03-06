/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License, Version 1.0 only
 * (the "License").  You may not use this file except in compliance
 * with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*
 * Copyright 1992 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#pragma ident	"%Z%%M%	%I%	%E% SMI"

#include <sys/types.h>
#include <sys/label.h>
#include <sys/audit.h>

#define MAXSTRLEN 360

/*	getfaudflgs.c */

/*
 * getfauditflags() - combines system event flag mask with user event
 *                               flag masks.
 *
 * input: usremasks->as_success - always audit on success
 *        usremasks->as_failure - always audit on failure
 *        usrdmasks->as_success - never audit on success
 *        usrdmasks->as_failure - never audit on failure
 *
 * output: lastmasks->as_success - audit on success
 *         lastmasks->as_failure - audit on failure
 *
 * returns:  0 - ok
 *          -1 - error
 */

int
getfauditflags(audit_state_t *usremasks, audit_state_t *usrdmasks,
    audit_state_t *lastmasks)
{	 
	int len = MAXSTRLEN, retstat = 0;
	char s_auditstring[MAXSTRLEN];
	audit_state_t masks;
 
	masks.as_success = 0;
	masks.as_failure = 0;
	/* 
	 * get system audit mask and convert to bit mask 
	 */
	if ((getacflg(s_auditstring, len)) >= 0)  {
		if ((getauditflagsbin(s_auditstring, &masks)) != 0)
	        	retstat = -1;
	} else
		retstat = -1;
 
	/* 
	 * combine system and user event masks 
	 */
	if (retstat == 0) {
		lastmasks->as_success = masks.as_success;
		lastmasks->as_failure = masks.as_failure;
 
		lastmasks->as_success |= usremasks->as_success;
		lastmasks->as_failure |= usremasks->as_failure;
 
		lastmasks->as_success &= ~(usrdmasks->as_success);
		lastmasks->as_failure &= ~(usrdmasks->as_failure);
	}
	return (retstat);
}
