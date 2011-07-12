#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <drawtk.h>
#include <stdlib.h>
#include <stdio.h>

#include <stdbool.h>

#include <glib.h>
#include <dtk_gstreamer.h>

int main (int argc, char *argv[])
{
  bool success;

  // Initialization 
  dtk_gst_hpipeline pipe = dtk_gst_create_file_pipeline("MyFilePipeline","/media/sf_master/test.ogg");

  success = (pipe!=NULL);
  if (!success) {
          g_printerr ("Pipeline could not be created. Exiting.\n");
          return -1;
  }

  // Run
  success = dtk_gst_run_pipeline(pipe);
  if (!success) {
        g_printerr ("The pipeline could not be started. Exiting.\n");
        return -1;
  }
  else
  {
        g_print("The pipeline is executing.\n");
  }

  // Wait till pipeline dies
  while(dtk_gst_is_pipeline_alive(pipe));

  // terminate pipeline thread, useless in this configuration as we're waiting till the pipeline is alive anyways...
  dtk_gst_stop_pipeline(pipe);

  return 0;
}

