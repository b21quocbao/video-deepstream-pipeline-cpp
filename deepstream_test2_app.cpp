#include <gst/gst.h>
#include <stdio.h>
#include <torch/torch.h>
#include <iostream>
#include "nvbufsurface.h"

gint frames_processed = 0;
gint64 start_time;
torch::Device device = torch::kCPU;

void buffer_to_image_tensor(GstBuffer * buf, GstCaps * caps) {
    const GstStructure *caps_structure = gst_caps_get_structure (caps, 0);
    auto height = g_value_get_int(gst_structure_get_value(caps_structure, "height"));
    auto width = g_value_get_int(gst_structure_get_value(caps_structure, "width"));
    GstMapInfo map_info;
    memset (&map_info, 0, sizeof (map_info));
    gboolean is_mapped = gst_buffer_map(buf, &map_info, GST_MAP_READ);
    NvBufSurface *info_data = (NvBufSurface *) map_info.data;
    assert(info_data->numFilled == 1);
    assert(info_data->surfaceList[0].colorFormat == 19);
    std::cout << "width is: " << info_data->surfaceList[0].width << "\n";
    std::cout << "height is: " << info_data->surfaceList[0].height << "\n";
    std::cout << "pitch is: " << info_data->surfaceList[0].pitch << "\n";
    std::cout << "colorFormat is: " << info_data->surfaceList[0].colorFormat << "\n";
    std::cout << "layout is: " << info_data->surfaceList[0].layout << "\n";
    std::cout << "bufferDesc is: " << info_data->surfaceList[0].bufferDesc << "\n";
    std::cout << "dataSize is: " << info_data->surfaceList[0].dataSize << "\n";
    std::cout << "dataPtr is: " << info_data->surfaceList[0].dataPtr << "\n";
    if (is_mapped) {
        auto max_val = at::zeros( {height, width, 4}, torch::CUDA(torch::kUInt8));
        gst_buffer_unmap(buf, &map_info);
    }
}

static GstPadProbeReturn
on_frame_probe(GstPad * pad, GstPadProbeInfo * info, gpointer u_data) {
    GstBuffer *buf = gst_pad_probe_info_get_buffer(info);
    buffer_to_image_tensor(buf, gst_pad_get_current_caps(pad));
    
    if (frames_processed == 0) {
        start_time = g_get_monotonic_time();
    }
    frames_processed ++;
    std::cout << "Number frames: " << frames_processed << "\n";
    return GST_PAD_PROBE_OK;
}

static gboolean
bus_call (GstBus * bus, GstMessage * msg, gpointer data)
{
  GMainLoop *loop = (GMainLoop *) data;
  switch (GST_MESSAGE_TYPE (msg)) {
    case GST_MESSAGE_EOS:
      g_print ("End of stream\n");
      g_main_loop_quit (loop);
      break;
    case GST_MESSAGE_ERROR:{
      gchar *debug;
      GError *error;
      gst_message_parse_error (msg, &error, &debug);
      g_printerr ("ERROR from element %s: %s\n",
          GST_OBJECT_NAME (msg->src), error->message);
      if (debug)
        g_printerr ("Error details: %s\n", debug);
      g_free (debug);
      g_error_free (error);
      g_main_loop_quit (loop);
      break;
    }
    default:
      break;
  }
  return TRUE;
}

int
main (int argc, char *argv[])
{
  if (torch::cuda::is_available()) {
      std::cout << "CUDA is available! Training on GPU." << std::endl;
      device = torch::kCUDA;
  }
  GstElement *pipeline;
  GstMessage *msg;
  GstPad *sink_pad = NULL;
  GstBus *bus = NULL;
  guint bus_watch_id;
  GMainLoop *loop = NULL;
  /* Check input arguments */
  if (argc != 2) {
    g_printerr ("Usage: %s <MP4 filename>\n", argv[0]);
    return -1;
  }

  /* Initialize GStreamer */
  gst_init (&argc, &argv);
  loop = g_main_loop_new (NULL, FALSE);

  /* Build the pipeline */
  std::string file_location(argv[1]);
  pipeline =
      gst_parse_launch
      (("filesrc location=" + file_location + " num-buffers=256 ! decodebin ! nvvideoconvert ! video/x-raw(memory:NVMM),format=RGBA ! fakesink name=s").c_str(),
      NULL);
  sink_pad = gst_element_get_static_pad(gst_bin_get_by_name(GST_BIN(pipeline), "s"), "sink");
  
  if (!sink_pad)
    g_print ("Unable to get sink pad\n");
  else
    gst_pad_add_probe (sink_pad, GST_PAD_PROBE_TYPE_BUFFER,
        on_frame_probe, NULL, NULL);
  gst_object_unref (sink_pad);

  /* we add a message handler */
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  bus_watch_id = gst_bus_add_watch (bus, bus_call, loop);
  gst_object_unref (bus);
  /* Set the pipeline to "playing" state */
  g_print ("Now playing: %s\n", argv[1]);
  gst_element_set_state (pipeline, GST_STATE_PLAYING);

  /* Wait till pipeline encounters an error or EOS */
  g_print ("Running...\n");
  g_main_loop_run (loop);

  /* Out of the main loop, clean up nicely */
  g_print ("Returned, stopping playback\n");
  gst_element_set_state (pipeline, GST_STATE_NULL);
  g_print ("Deleting pipeline\n");
  gst_object_unref (GST_OBJECT (pipeline));
  g_source_remove (bus_watch_id);
  g_main_loop_unref (loop);
  return 0;
}