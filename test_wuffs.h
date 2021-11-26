#ifndef TEST_WUFFS_H
#define TEST_WUFFS_H

#define WUFFS_IMPLEMENTATION

#define WUFFS_CONFIG__MODULES
#define WUFFS_CONFIG__MODULE__ADLER32
#define WUFFS_CONFIG__MODULE__BASE
#define WUFFS_CONFIG__MODULE__CRC32
#define WUFFS_CONFIG__MODULE__DEFLATE
#define WUFFS_CONFIG__MODULE__PNG
#define WUFFS_CONFIG__MODULE__ZLIB

#include "wuffs-v0.3.c"

#include <stdio.h>

unsigned char *getimage_wuffs(unsigned char *buf, size_t size, size_t *out_size)
{
	wuffs_base__io_buffer bufwrap =
		wuffs_base__ptr_u8__reader(buf, size, 1 /* true */);
	wuffs_base__image_decoder *decoder =
		wuffs_png__decoder__alloc_as__wuffs_base__image_decoder();
	if(!decoder)
	{
		printf("wuffs init error\n");
		return NULL;
	}

	wuffs_base__image_config cfg;
	wuffs_base__status status =
		wuffs_base__image_decoder__decode_image_config(decoder, &cfg, &bufwrap);
	if(!wuffs_base__status__is_ok(&status))
	{
		printf("wuffs: %s\n", wuffs_base__status__message(&status));
		goto fail_config;
	}
	if(!wuffs_base__image_config__is_valid(&cfg))
	{
		printf("wuffs: %s\n", "invalid Wuffs image configuration");
		goto fail_config;
	}

	uint32_t width = wuffs_base__pixel_config__width(&cfg.pixcfg);
	uint32_t height = wuffs_base__pixel_config__height(&cfg.pixcfg);
	wuffs_base__pixel_config__set(&cfg.pixcfg,
		WUFFS_BASE__PIXEL_FORMAT__RGBA_NONPREMUL,
		WUFFS_BASE__PIXEL_SUBSAMPLING__NONE, width, height);

	wuffs_base__slice_u8 workbuf = {0};
	uint64_t workbuf_len_max_incl =
		wuffs_base__image_decoder__workbuf_len(decoder).max_incl;
	if(workbuf_len_max_incl)
	{
		workbuf = wuffs_base__malloc_slice_u8(malloc, workbuf_len_max_incl);
		if(!workbuf.ptr)
		{
			printf("wuffs: %s\n", "failed to allocate a work buffer");
			goto fail_config;
		}
	}

	*out_size = width * height * 4 /* RGBA */;
	unsigned char *data = calloc(1, *out_size);
	if(data == NULL)
	{
		printf("wuffs: %s\n", "failed to allocate target buffer");
		goto fail_image_buffer;
	}

	wuffs_base__pixel_buffer pb = {0};
	status = wuffs_base__pixel_buffer__set_from_slice(&pb,
		&cfg.pixcfg, wuffs_base__make_slice_u8(data, *out_size));
	if(!wuffs_base__status__is_ok(&status))
	{
		printf("wuffs: %s\n", wuffs_base__status__message(&status));
		goto fail_decode;
	}

	status = wuffs_base__image_decoder__decode_frame(decoder,
		&pb, &bufwrap, WUFFS_BASE__PIXEL_BLEND__SRC, workbuf, NULL);
	if(!wuffs_base__status__is_ok(&status))
	{
		printf("wuffs: %s\n", wuffs_base__status__message(&status));
		goto fail_decode;
	}

	free(workbuf.ptr);
	free(decoder);
	return data;

fail_decode:
	free(data);
fail_image_buffer:
	free(workbuf.ptr);
fail_config:
	free(decoder);
	return NULL;
}

#endif /* TEST_WUFFS_H */
