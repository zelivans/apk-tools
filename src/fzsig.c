/* update.c - Alpine Package Keeper (APK)
 *
 * Copyright (C) 2005-2008 Natanael Copa <n@tanael.org>
 * Copyright (C) 2008-2011 Timo Teräs <timo.teras@iki.fi>
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation. See http://www.gnu.org/ for details.
 */

#include <stdio.h>
#include "apk_defines.h"
#include "apk_applet.h"
#include "apk_database.h"
#include "apk_version.h"
#include "apk_print.h"

int do_nothing_func (void *sctx, const struct apk_file_info *fi,
			    struct apk_istream *is)
{
	return 0;
}

static int fzsig_main(void *ctx, struct apk_database *db, struct apk_string_array *args)
{
	char * filename = *(&(args)->item[0]);
	// struct apk_bstream *bs = NULL;
	struct apk_istream *is = NULL;

	if (*(&(args)->num) != 1)
	{
		apk_message("Needs one argument of filename");
		return 1;
	}

	apk_message("Working on %s...", filename);

	//bs = apk_bstream_from_file(AT_FDCWD, filename);
	is = apk_istream_from_file(AT_FDCWD, filename);
	if (IS_ERR_OR_NULL(is))
	{
		apk_message("Check your file please");
		return PTR_ERR(is);
	}

	/* The real stuff here */
	// return apk_cache_download_fzsig_local(db, NULL, APK_SIGN_VERIFY, NULL, NULL, bs);
	// return apk_tar_parse(is, do_nothing_func, NULL, FALSE, &db->id_cache); // expects tar (not gunzipped)
	return apk_tar_parse(is, do_nothing_func, NULL, FALSE, 	NULL); // quick hacks - removing idc, db not needed...
}

static struct apk_applet apk_fzsig = {
	.name = "fzsig",
	.help = "Applet for fuzzing the tar.gz cache downloading",
	.open_flags = APK_OPENF_WRITE,
	.main = fzsig_main,
};

APK_DEFINE_APPLET(apk_fzsig);

