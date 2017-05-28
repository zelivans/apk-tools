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

static int fz_main(void *ctx, struct apk_database *db, struct apk_string_array *args)
{
	char * filename = *(&(args)->item[0]);
	struct apk_bstream *bs = NULL;

	if (*(&(args)->num) != 1)
	{
		apk_message("Needs one argument of filename");
		return 1;
	}

	apk_message("Working on %s...", filename);

	bs = apk_bstream_from_file(AT_FDCWD, filename);
	if (IS_ERR_OR_NULL(bs))
	{
		apk_message("Check your file please");
		return PTR_ERR(bs);
	}

	/* The real stuff here */
	apk_db_index_read(db, bs, 1);

	bs->close(bs, NULL);
	return 0;
}

static struct apk_applet apk_fz = {
	.name = "fz",
	.help = "Applet for simple fuzzing",
	.open_flags = APK_OPENF_WRITE,
	.main = fz_main,
};

APK_DEFINE_APPLET(apk_fz);

