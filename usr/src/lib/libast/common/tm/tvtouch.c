/***********************************************************************
*                                                                      *
*               This software is part of the ast package               *
*          Copyright (c) 1985-2009 AT&T Intellectual Property          *
*                      and is licensed under the                       *
*                  Common Public License, Version 1.0                  *
*                    by AT&T Intellectual Property                     *
*                                                                      *
*                A copy of the License is available at                 *
*            http://www.opensource.org/licenses/cpl1.0.txt             *
*         (with md5 checksum 059e8cd6165cb4c31e351f2b69388fd9)         *
*                                                                      *
*              Information and Software Systems Research               *
*                            AT&T Research                             *
*                           Florham Park NJ                            *
*                                                                      *
*                 Glenn Fowler <gsf@research.att.com>                  *
*                  David Korn <dgk@research.att.com>                   *
*                   Phong Vo <kpv@research.att.com>                    *
*                                                                      *
***********************************************************************/
#pragma prototyped
/*
 * Glenn Fowler
 * AT&T Research
 *
 * Tv_t conversion support
 */

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:hide utime
#else
#define utime		______utime
#endif

#ifndef _ATFILE_SOURCE
#define _ATFILE_SOURCE	1
#endif

#include <ast.h>
#include <ls.h>
#include <tv.h>
#include <times.h>
#include <error.h>

#include "FEATURE/tvlib"

#if _hdr_utime && _lib_utime
#include <utime.h>
#endif

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:nohide utime
#else
#undef	utime
#endif

#if _lib_utime
#if _hdr_utime
extern int	utime(const char*, const struct utimbuf*);
#else
extern int	utime(const char*, const time_t*);
#endif
#endif

#define NS(n)		(((uint32_t)(n))<1000000000L?(n):0)

/*
 * touch path <atime,mtime,ctime>
 * Tv_t==0 uses current time
 * Tv_t==TV_TOUCH_RETAIN retains path value if it exists, current time otherwise
 * otherwise it is exact time
 * file created if it doesn't exist and (flags&1)
 * symlink not followed if (flags&2)
 * cv most likely ignored on most implementations
 *
 * NOTE: when *at() calls are integrated we need macros for the flags!
 */

int
tvtouch(const char* path, register const Tv_t* av, register const Tv_t* mv, const Tv_t* cv, int flags)
{
	int		fd;
	int		mode;
	int		oerrno;
	struct stat	st;

#if _lib_utimensat
	struct timespec	ts[2];

	errno = oerrno;
	if (!av)
	{
		ts[0].tv_sec = 0;
		ts[0].tv_nsec = UTIME_NOW;
	}
	else if (av == TV_TOUCH_RETAIN)
	{
		ts[0].tv_sec = 0;
		ts[0].tv_nsec = UTIME_OMIT;
	}
	else
	{
		ts[0].tv_sec = av->tv_sec;
		ts[0].tv_nsec = NS(av->tv_nsec);
	}
	if (!mv)
	{
		ts[1].tv_sec = 0;
		ts[1].tv_nsec = UTIME_NOW;
	}
	else if (mv == TV_TOUCH_RETAIN)
	{
		ts[1].tv_sec = 0;
		ts[1].tv_nsec = UTIME_OMIT;
	}
	else
	{
		ts[1].tv_sec = mv->tv_sec;
		ts[1].tv_nsec = NS(mv->tv_nsec);
	}
	if (!cv && av == TV_TOUCH_RETAIN && mv == TV_TOUCH_RETAIN && !stat(path, &st) && !chmod(path, st.st_mode & S_IPERM))
		return 0;
	if (!utimensat(AT_FDCWD, path, ts[0].tv_nsec == UTIME_NOW && ts[1].tv_nsec == UTIME_NOW ? (struct timespec*)0 : ts, (flags & 2) ? AT_SYMLINK_NOFOLLOW : 0))
		return 0;
	if (errno != ENOENT || !(flags & 1))
		return -1;
	umask(mode = umask(0));
	mode = (~mode) & (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
	if ((fd = open(path, O_WRONLY|O_CREAT|O_TRUNC, mode)) < 0)
		return -1;
	close(fd);
	errno = oerrno;
	if ((ts[0].tv_nsec != UTIME_NOW || ts[1].tv_nsec != UTIME_NOW) && utimensat(AT_FDCWD, path, ts, (flags & 2) ? AT_SYMLINK_NOFOLLOW : 0))
		return -1;
	return 0;
#else
	Tv_t		now;
#if _lib_utimets
	struct timespec	am[2];
#else
#if _lib_utimes
	struct timeval	am[2];
#else
#if _hdr_utime
	struct utimbuf	am;
#else
	time_t		am[2];
#endif
#endif
#endif

	oerrno = errno;
	if ((av == TV_TOUCH_RETAIN || mv == TV_TOUCH_RETAIN) && stat(path, &st))
	{
		errno = oerrno;
		if (av == TV_TOUCH_RETAIN)
			av = 0;
		if (mv == TV_TOUCH_RETAIN)
			mv = 0;
	}
	if (!av || !mv)
	{
		tvgettime(&now);
		if (!av)
			av = (const Tv_t*)&now;
		if (!mv)
			mv = (const Tv_t*)&now;
	}
#if _lib_utimets
	if (av == TV_TOUCH_RETAIN)
	{
		am[0].tv_sec = st.st_atime;
		am[0].tv_nsec = ST_ATIME_NSEC_GET(&st);
	}
	else
	{
		am[0].tv_sec = av->tv_sec;
		am[0].tv_nsec = NS(av->tv_nsec);
	}
	if (mv == TV_TOUCH_RETAIN)
	{
		am[1].tv_sec = st.st_mtime;
		am[1].tv_nsec = ST_MTIME_NSEC_GET(&st);
	}
	else
	{
		am[1].tv_sec = mv->tv_sec;
		am[1].tv_nsec = NS(mv->tv_nsec);
	}
	if (!utimets(path, am))
		return 0;
	if (errno != ENOENT && av == (const Tv_t*)&now && mv == (const Tv_t*)&now && !utimets(path, NiL))
	{
		errno = oerrno;
		return 0;
	}
#else
#if _lib_utimes
	if (av == TV_TOUCH_RETAIN)
	{
		am[0].tv_sec = st.st_atime;
		am[0].tv_usec = ST_ATIME_NSEC_GET(&st) / 1000;
	}
	else
	{
		am[0].tv_sec = av->tv_sec;
		am[0].tv_usec = NS(av->tv_nsec) / 1000;
	}
	if (mv == TV_TOUCH_RETAIN)
	{
		am[1].tv_sec = st.st_mtime;
		am[1].tv_usec = ST_MTIME_NSEC_GET(&st) / 1000;
	}
	else
	{
		am[1].tv_sec = mv->tv_sec;
		am[1].tv_usec = NS(mv->tv_nsec) / 1000;
	}
	if (!utimes(path, am))
		return 0;
	if (errno != ENOENT && av == (const Tv_t*)&now && mv == (const Tv_t*)&now && !utimes(path, NiL))
	{
		errno = oerrno;
		return 0;
	}
#else
#if _lib_utime
	am.actime = (av == TV_TOUCH_RETAIN) ? st.st_atime : av->tv_sec;
	am.modtime = (mv == TV_TOUCH_RETAIN) ? st.st_mtime : mv->tv_sec;
	if (!utime(path, &am))
		return 0;
#if _lib_utime_now
	if (errno != ENOENT && av == (const Tv_t*)&now && mv == (const Tv_t*)&now && !utime(path, NiL))
	{
		errno = oerrno;
		return 0;
	}
#endif
#endif
#endif
	if (!access(path, F_OK))
	{
		if (av != (const Tv_t*)&now || mv != (const Tv_t*)&now)
		{
			errno = EINVAL;
			return -1;
		}
		if ((fd = open(path, O_RDWR)) >= 0)
		{
			char	c;

			if (read(fd, &c, 1) == 1)
			{
				if (c = (lseek(fd, 0L, 0) == 0L && write(fd, &c, 1) == 1))
					errno = oerrno;
				close(fd);
				if (c)
					return 0;
			}
			close(fd);
		}
	}
#endif
	if (errno != ENOENT || !(flags & 1))
		return -1;
	umask(mode = umask(0));
	mode = (~mode) & (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
	if ((fd = open(path, O_WRONLY|O_CREAT|O_TRUNC, mode)) < 0)
		return -1;
	close(fd);
	errno = oerrno;
	if (av == (const Tv_t*)&now && mv == (const Tv_t*)&now)
		return 0;
#if _lib_utimets
	return utimets(path, am);
#else
#if _lib_utimes
	return utimes(path, am);
#else
#if _lib_utime
	return utime(path, &am);
#else
	errno = EINVAL;
	return -1;
#endif
#endif
#endif
#endif

}
