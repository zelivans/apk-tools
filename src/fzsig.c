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

int apk_cache_download_fzsig_local(struct apk_database *db, // struct apk_repository *repo,
		       struct apk_package *pkg, int verify,
		       apk_progress_cb cb, void *cb_ctx, struct apk_bstream *bs)
{
	struct stat st;
	struct apk_istream *is;
	// struct apk_bstream *bs;
	struct apk_sign_ctx sctx;
	// char url[PATH_MAX];
	char tmpcacheitem[128], *cacheitem = &tmpcacheitem[tmpprefix.len];
	apk_blob_t b = APK_BLOB_BUF(tmpcacheitem);
	int r, fd;

	apk_blob_push_blob(&b, tmpprefix);
	// if (pkg != NULL)
	// 	r = apk_pkg_format_cache_pkg(b, pkg);
	// else
	// 	r = apk_repo_format_cache_index(b, repo);
	// if (r < 0) return r;

	// Quick hack to remove the demand of repo
	apk_blob_push_blob(&b, APK_BLOB_STR("APKINDEX.66666666.tar.gz"));
	apk_blob_push_blob(&b, APK_BLOB_PTR_LEN("", 1));
	if (APK_BLOB_IS_NULL(to))
		return -ENOBUFS;

	// r = apk_repo_format_real_url(db, repo, pkg, url, sizeof(url));
	// if (r < 0) return r;

	if ((apk_flags & APK_FORCE) ||
	    fstatat(db->cache_fd, cacheitem, &st, 0) != 0)
		st.st_mtime = 0;

	// apk_message("fetch %s", url);

	// if (apk_flags & APK_SIMULATE) return 0;
	if (cb) cb(cb_ctx, 0);

	if (verify != APK_SIGN_NONE) {
		apk_sign_ctx_init(&sctx, APK_SIGN_VERIFY, NULL, db->keys_fd);
		// bs = apk_bstream_from_url_if_modified(url, st.st_mtime);
		// changed to gettings the bs from the caller
		bs = apk_bstream_tee(bs, db->cache_fd, tmpcacheitem, cb, cb_ctx);
		is = apk_bstream_gunzip_mpart(bs, apk_sign_ctx_mpart_cb, &sctx);
		if (!IS_ERR_OR_NULL(is))
			r = apk_tar_parse(is, apk_sign_ctx_verify_tar, &sctx, FALSE, &db->id_cache);
		else
			r = PTR_ERR(is) ?: -EIO;
		apk_sign_ctx_free(&sctx);
	} else {
		// is = apk_istream_from_url_if_modified(url, st.st_mtime);
		// changed to gettings the bs from the caller
		if (!IS_ERR_OR_NULL(is)) {
			fd = openat(db->cache_fd, tmpcacheitem, O_RDWR | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
			if (fd < 0) r = -errno;
		} else fd = -1, r = PTR_ERR(is) ?: -EIO;

		if (fd >= 0) {
			struct apk_file_meta meta;
			r = apk_istream_splice(is, fd, APK_SPLICE_ALL, cb, cb_ctx);
			is->get_meta(is, &meta);
			apk_file_meta_to_fd(fd, &meta);
			close(fd);
		}
	}
	if (!IS_ERR_OR_NULL(is)) is->close(is);
	if (r == -EALREADY) return 0;
	if (r < 0) {
		unlinkat(db->cache_fd, tmpcacheitem, 0);
		return r;
	}

	if (renameat(db->cache_fd, tmpcacheitem, db->cache_fd, cacheitem) < 0)
		return -errno;
	return 0;
}

static int fzsig_main(void *ctx, struct apk_database *db, struct apk_string_array *args)
{
	int res = 0;
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
	res = apk_cache_download_fzsig_local(db, NULL, APK_SIGN_VERIFY, NULL, NULL, bs);

	bs->close(bs, NULL);
	return res;
}

static struct apk_applet apk_fzsig = {
	.name = "fzsig",
	.help = "Applet for fuzzing the tar.gz cache downloading",
	.open_flags = APK_OPENF_WRITE,
	.main = fzsig_main,
};

APK_DEFINE_APPLET(apk_fzsif);

