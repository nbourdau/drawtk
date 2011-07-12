#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <drawtk.h>
#include <stdlib.h>
#include <stdio.h>

#include <stdbool.h>

#include <glib.h>
#include <dtk_gstreamer.h>

int main (int   argc, char *argv[])
{
  bool success;

  // Initialisation 
  dtk_gst_hpipeline pipe = dtk_gst_create_empty_pipeline("dtk_pipeline");

  success = (pipe!=NULL);

  // Add elements 
  success = success && dtk_gst_add_element_full(pipe, "tcpclientsrc",	"tcp-src", "port", 38501, NULL);
  success = success && dtk_gst_add_element(pipe, "multipartdemux",	"demuxer");
  success = success && dtk_gst_add_element(pipe, "jpegdec",		"decoder");
  success = success && dtk_gst_add_element(pipe, "ffmpegcolorspace",	"converter");
  success = success && dtk_gst_add_element(pipe, "autovideosink",	"autovideosink");
  
  if (!success) {
          g_printerr ("One element could not be created. Exiting.\n");
          return -1;
  }

  success = dtk_gst_run_pipeline(pipe);

  if (!success) {
        g_printerr ("The pipeline could not be started. Exiting.\n");
        return -1;
  }
  else
  {
        g_print("The pipeline is executing.\n");
  }

  while(dtk_gst_is_pipeline_alive(pipe));

  // terminate pipeline thread
  dtk_gst_stop_pipeline(pipe);

  return 0;
}

