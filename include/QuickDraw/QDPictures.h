/*
 * QDPictures.h - QuickDraw Picture Recording
 *
 * Provides functions for recording QuickDraw drawing operations
 * into Picture resources for later playback.
 *
 * Copyright (c) 2025 - System 7.1 Portable Project
 */

#ifndef __QDPICTURES_H__
#define __QDPICTURES_H__

#include "SystemTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Picture recording hooks - called by QuickDraw primitives */
void PictureRecordFrameRect(const Rect *r);
void PictureRecordPaintRect(const Rect *r);
void PictureRecordEraseRect(const Rect *r);
void PictureRecordInvertRect(const Rect *r);
void PictureRecordFrameOval(const Rect *r);
void PictureRecordPaintOval(const Rect *r);
void PictureRecordEraseOval(const Rect *r);
void PictureRecordInvertOval(const Rect *r);

#ifdef __cplusplus
}
#endif

#endif /* __QDPICTURES_H__ */
