/*
 * Copyright (C) 2017 Collabora Inc.
 *    Author: Nicolas Dufresne <nicolas.dufresne@collabora.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include "gstv4l2object.h"
#include "gstv4l2mpeg4enc.h"

#include <string.h>
#include <gst/gst-i18n-plugin.h>

GST_DEBUG_CATEGORY_STATIC (gst_v4l2_mpeg4_enc_debug);
#define GST_CAT_DEFAULT gst_v4l2_mpeg4_enc_debug

static GstStaticCaps src_template_caps =
GST_STATIC_CAPS ("video/mpeg, mpegversion=(int)4, systemstream=(boolean)FALSE");

enum
{
  PROP_0,
  V4L2_STD_OBJECT_PROPS,
  /* TODO */
};

#define gst_v4l2_mpeg4_enc_parent_class parent_class
G_DEFINE_TYPE (GstV4l2Mpeg4Enc, gst_v4l2_mpeg4_enc, GST_TYPE_V4L2_VIDEO_ENC);

static void
gst_v4l2_mpeg4_enc_set_property (GObject * object,
    guint prop_id, const GValue * value, GParamSpec * pspec)
{
  /* TODO */
}

static void
gst_v4l2_mpeg4_enc_get_property (GObject * object,
    guint prop_id, GValue * value, GParamSpec * pspec)
{
  /* TODO */
}

static gint
v4l2_profile_from_string (const gchar * profile)
{
  gint v4l2_profile = -1;

  if (g_str_equal (profile, "simple"))
    v4l2_profile = V4L2_MPEG_VIDEO_MPEG4_PROFILE_SIMPLE;
  else if (g_str_equal (profile, "advanced-simple"))
    v4l2_profile = V4L2_MPEG_VIDEO_MPEG4_PROFILE_ADVANCED_SIMPLE;
  else if (g_str_equal (profile, "core"))
    v4l2_profile = V4L2_MPEG_VIDEO_MPEG4_PROFILE_CORE;
  else if (g_str_equal (profile, "simple-scalable"))
    v4l2_profile = V4L2_MPEG_VIDEO_MPEG4_PROFILE_SIMPLE_SCALABLE;
  else if (g_str_equal (profile, "advanced-coding-efficiency"))
    v4l2_profile = V4L2_MPEG_VIDEO_MPEG4_PROFILE_ADVANCED_CODING_EFFICIENCY;
  else
    GST_WARNING ("Unsupported profile string '%s'", profile);

  return v4l2_profile;
}

static const gchar *
v4l2_profile_to_string (gint v4l2_profile)
{
  switch (v4l2_profile) {
    case V4L2_MPEG_VIDEO_MPEG4_PROFILE_SIMPLE:
      return "simple";
    case V4L2_MPEG_VIDEO_MPEG4_PROFILE_ADVANCED_SIMPLE:
      return "advanced-simple";
    case V4L2_MPEG_VIDEO_MPEG4_PROFILE_CORE:
      return "core";
    case V4L2_MPEG_VIDEO_MPEG4_PROFILE_SIMPLE_SCALABLE:
      return "simple-scalable";
    case V4L2_MPEG_VIDEO_MPEG4_PROFILE_ADVANCED_CODING_EFFICIENCY:
      return "advanced-coding-efficiency";
    default:
      GST_WARNING ("Unsupported V4L2 profile %i", v4l2_profile);
      break;
  }

  return NULL;
}

static gint
v4l2_level_from_string (const gchar * level)
{
  gint v4l2_level = -1;

  if (g_str_equal (level, "0"))
    v4l2_level = V4L2_MPEG_VIDEO_MPEG4_LEVEL_0;
  else if (g_str_equal (level, "0b"))
    v4l2_level = V4L2_MPEG_VIDEO_MPEG4_LEVEL_0B;
  else if (g_str_equal (level, "1"))
    v4l2_level = V4L2_MPEG_VIDEO_MPEG4_LEVEL_1;
  else if (g_str_equal (level, "2"))
    v4l2_level = V4L2_MPEG_VIDEO_MPEG4_LEVEL_2;
  else if (g_str_equal (level, "3"))
    v4l2_level = V4L2_MPEG_VIDEO_MPEG4_LEVEL_3;
  else if (g_str_equal (level, "3b"))
    v4l2_level = V4L2_MPEG_VIDEO_MPEG4_LEVEL_3B;
  else if (g_str_equal (level, "4"))
    v4l2_level = V4L2_MPEG_VIDEO_MPEG4_LEVEL_4;
  else if (g_str_equal (level, "5"))
    v4l2_level = V4L2_MPEG_VIDEO_MPEG4_LEVEL_5;
  else
    GST_WARNING ("Unsupported level '%s'", level);

  return v4l2_level;
}

static const gchar *
v4l2_level_to_string (gint v4l2_level)
{
  switch (v4l2_level) {
    case V4L2_MPEG_VIDEO_MPEG4_LEVEL_0:
      return "0";
    case V4L2_MPEG_VIDEO_MPEG4_LEVEL_0B:
      return "0b";
    case V4L2_MPEG_VIDEO_MPEG4_LEVEL_1:
      return "1";
    case V4L2_MPEG_VIDEO_MPEG4_LEVEL_2:
      return "2";
    case V4L2_MPEG_VIDEO_MPEG4_LEVEL_3:
      return "3";
    case V4L2_MPEG_VIDEO_MPEG4_LEVEL_3B:
      return "3b";
    case V4L2_MPEG_VIDEO_MPEG4_LEVEL_4:
      return "4";
    case V4L2_MPEG_VIDEO_MPEG4_LEVEL_5:
      return "5";
    default:
      GST_WARNING ("Unsupported V4L2 level %i", v4l2_level);
      break;
  }

  return NULL;
}

static void
gst_v4l2_mpeg4_enc_init (GstV4l2Mpeg4Enc * self)
{
}

static void
gst_v4l2_mpeg4_enc_class_init (GstV4l2Mpeg4EncClass * klass)
{
  GstElementClass *element_class;
  GObjectClass *gobject_class;
  GstV4l2VideoEncClass *baseclass;

  parent_class = g_type_class_peek_parent (klass);

  element_class = (GstElementClass *) klass;
  gobject_class = (GObjectClass *) klass;
  baseclass = (GstV4l2VideoEncClass *) (klass);

  GST_DEBUG_CATEGORY_INIT (gst_v4l2_mpeg4_enc_debug, "v4l2mpeg4enc", 0,
      "V4L2 MPEG4 Encoder");

  gst_element_class_set_static_metadata (element_class,
      "V4L2 MPEG4 Encoder",
      "Codec/Encoder/Video/Hardware",
      "Encode MPEG4 video streams via V4L2 API",
      "Nicolas Dufresne <nicolas.dufresne@collabora.com");

  gobject_class->set_property =
      GST_DEBUG_FUNCPTR (gst_v4l2_mpeg4_enc_set_property);
  gobject_class->get_property =
      GST_DEBUG_FUNCPTR (gst_v4l2_mpeg4_enc_get_property);

  baseclass->codec_name = "MPEG4";
  baseclass->profile_cid = V4L2_CID_MPEG_VIDEO_MPEG4_PROFILE;
  baseclass->profile_to_string = v4l2_profile_to_string;
  baseclass->profile_from_string = v4l2_profile_from_string;
  baseclass->level_cid = V4L2_CID_MPEG_VIDEO_MPEG4_LEVEL;
  baseclass->level_to_string = v4l2_level_to_string;
  baseclass->level_from_string = v4l2_level_from_string;
}

/* Probing functions */
gboolean
gst_v4l2_is_mpeg4_enc (GstCaps * sink_caps, GstCaps * src_caps)
{
  return gst_v4l2_is_video_enc (sink_caps, src_caps,
      gst_static_caps_get (&src_template_caps));
}

void
gst_v4l2_mpeg4_enc_register (GstPlugin * plugin, const gchar * basename,
    const gchar * device_path, GstCaps * sink_caps, GstCaps * src_caps)
{
  gst_v4l2_video_enc_register (plugin, GST_TYPE_V4L2_MPEG4_ENC,
      "mpeg4", basename, device_path, sink_caps,
      gst_static_caps_get (&src_template_caps), src_caps);
}
