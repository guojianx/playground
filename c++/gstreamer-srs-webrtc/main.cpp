#include <iostream>

#include <curl/curl.h>
#include <json-glib/json-glib.h>
#include <gst/gst.h>

#define GST_USE_UNSTABLE_API
#include <gst/webrtc/webrtc.h>

static GstElement *rtc = nullptr;

static gchar* GetStringFromJsonObject(JsonObject *object)
{
    JsonNode *root = nullptr;
    JsonGenerator *gen = nullptr;
    gchar *text = nullptr;

    root = json_node_init_object(json_node_alloc(), object);
    gen = json_generator_new();
    json_generator_set_root(gen, root);
    text = json_generator_to_data(gen, NULL);

    return text;
}

static size_t curl_callback(char *buf, size_t size, size_t nb, void *data)
{
    gst_print("curl_callback(%p, %ld, %ld, %p)\n", buf, size, nb, data);
    std::string *data_ptr = static_cast<std::string*>(data);
    g_assert_nonnull(data_ptr);

    data_ptr->append(buf, size * nb);

    return size * nb;
}

static void HttpPost4Sdp(const gchar *msg, std::string &remote_sdp_str)
{
    std::string peer_msg{};
    CURL *curl = nullptr;
    CURLcode res{};

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    g_assert_nonnull(curl);

    curl_easy_setopt(curl, CURLOPT_URL, "http://192.168.1.248:1985/rtc/v1/publish/");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, msg);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &peer_msg);

    gst_print("Http Post to %s Perform...\n", "http://192.168.1.248:1985/rtc/v1/publish/");
    res = curl_easy_perform(curl);
    JsonParser *parser = json_parser_new();
    json_parser_load_from_data(parser, peer_msg.c_str(), -1, nullptr);
    JsonReader *reader = json_reader_new(json_parser_get_root(parser));
    json_reader_read_member(reader, "sdp");

    gst_print("[Peer SDP]\n%s\n", json_reader_get_string_value(reader));
    remote_sdp_str = json_reader_get_string_value(reader);

    curl_easy_cleanup(curl);
    curl_global_cleanup();
}

static void ExchangeSdp(const GstWebRTCSessionDescription &desc, GstWebRTCSessionDescription **answer)
{
    std::string remote_sdp_str{};
    GstSDPMessage *remote_sdp;
    JsonObject *msg = nullptr;
    gchar *message = nullptr;
    msg = json_object_new();
    json_object_set_string_member(msg, "api", "http://192.168.1.248:1985/rtc/v1/publish/");
    json_object_set_string_member(msg, "tid", "1419821");
    json_object_set_string_member(msg, "streamurl", "webrtc://192.168.1.248/live/livestreamy");
    json_object_set_string_member(msg, "clientip", "null");
    json_object_set_string_member(msg, "sdp", gst_sdp_message_as_text(desc.sdp));

    message = GetStringFromJsonObject(msg);
    gst_print("[Offer]\n%s\n", message);
    HttpPost4Sdp(message, remote_sdp_str);
    gst_sdp_message_new_from_text(remote_sdp_str.c_str(), &remote_sdp);
    *answer = gst_webrtc_session_description_new(GST_WEBRTC_SDP_TYPE_ANSWER, remote_sdp);

    return;
}

static void OnOfferCreated(GstPromise *p, gpointer user_data)
{
    gst_print("OnOfferCreated(promise = %p, data = %p)\n", p, user_data);

    gchar *text = nullptr;
    GstWebRTCSessionDescription *offer  = nullptr;
    GstWebRTCSessionDescription *answer = nullptr;
    const GstStructure *reply = nullptr;

    reply = gst_promise_get_reply(p);
    gst_structure_get(reply, "offer", GST_TYPE_WEBRTC_SESSION_DESCRIPTION, &offer, NULL);
    gst_promise_unref(p);

    p = gst_promise_new();
    g_signal_emit_by_name(rtc, "set-local-description", offer, p);
    gst_promise_interrupt(p);
    gst_promise_unref(p);

    text = gst_sdp_message_as_text(offer->sdp);
    gst_print("[SDP]\n%s\n", text);

    ExchangeSdp(*offer, &answer);
    gst_webrtc_session_description_free(offer);

    p = gst_promise_new();
    g_signal_emit_by_name(rtc, "set-remote-description", answer, p);
    gst_promise_interrupt(p);
    gst_promise_unref(p);
    
    return;
}

static void OnNegotiationNeeded(GstElement *rtc, gpointer user_data)
{
    gst_print("OnNegotiationNeeded(%p)\n", rtc);

    GstPromise *p = nullptr;
    p = gst_promise_new_with_change_func(OnOfferCreated, user_data, NULL);
    g_signal_emit_by_name(rtc, "create-offer", NULL, p);

    return;
}

static void SendIceCandidateMessage(GstElement *webrtc G_GNUC_UNUSED, guint mlineindex,
        gchar* candidate, gpointer user_data G_GNUC_UNUSED)
{
    gst_print("[ICE]\nmlineindex=%u\n%s\n", mlineindex, candidate);
    return;
}

static void IceStateNotify(GstElement *rtc, GParamSpec *psec, gpointer user_data)
{
    GstWebRTCICEGatheringState ice_state;
    const char *state = "unknown";
    g_object_get(rtc, "ice-gathering-state", &ice_state, NULL);
    switch (ice_state) {
    case GST_WEBRTC_ICE_GATHERING_STATE_NEW:
        state = "new";
        break;
    case GST_WEBRTC_ICE_GATHERING_STATE_GATHERING:
        state = "gathering";
        break;
    case GST_WEBRTC_ICE_GATHERING_STATE_COMPLETE:
        state = "complete";
        break;
    }

    gst_print("\n#####################################################################\n");
    gst_print("ICE gathering state changed to %s\n", state);
    gst_print("#####################################################################\n");
    return;
}

static void OnGetStats(GstElement *rtc)
{
    gst_print("OnGetStats(%d)\n", rtc);
    return;
}

static gboolean GetStats(GstElement *rtc)
{
    GstPromise *promise;
    gst_print("\n#####################################################################\n");
    gst_print("GetStats(%p)\n", rtc);
    gst_print("#####################################################################\n");
    promise = gst_promise_new_with_change_func((GstPromiseChangeFunc)OnGetStats, rtc, NULL);
    g_signal_emit_by_name(rtc, "get-stats", NULL, promise);
    gst_promise_unref(promise);
    return G_SOURCE_REMOVE;
}

int main(int argc, char **argv)
{
    int ret;
    /* This must always be first GStreamer command.
     * Initializes all internal structures
     * Checks what plug-ins available
     * Executes any command-line option intended for GStreamer
     */
    gst_init(&argc, &argv);

    /* Media travels from the 'source' element, down to the 'sink' element,
     * passing through a series of intermediate elements.
     * The set of all the interconnected elements is called a 'pipe'.
     * "playin uri=https://moviesite.com/abc.webm"
     */
    auto pipe = gst_parse_launch("webrtcbin bundle-policy=max-bundle name=sendrecv "
            " videotestsrc is-live=true pattern=ball ! x264enc ! rtph264pay !"
            " application/x-rtp,media=video,encoding-name=H264,payload=96 ! sendrecv. "
            " audiotestsrc is-live=true wave=red-noise ! audioconvert ! audioresample ! opusenc !  rtpopuspay name=audiopay ! "
            " application/x-rtp,media=audio,encoding-name=OPUS,payload=97 ! sendrecv. ", NULL);

    /* Get a reference to webrtcbin and attach callbacks to it. */
    rtc = gst_bin_get_by_name(GST_BIN(pipe), "sendrecv");

    /* The entry point where we create the offer.            *
     * It will be called when the pipeline goes to PLAYING.  */
    g_signal_connect(rtc, "on-negotiation-needed", G_CALLBACK(OnNegotiationNeeded), NULL);

    /* transmit ICE candidate to srs signalling server */
    g_signal_connect(rtc, "on-ice-candidate", G_CALLBACK(SendIceCandidateMessage), NULL);
    g_signal_connect(rtc, "notify::ice-gethering-state", G_CALLBACK(IceStateNotify), NULL);

    ret = gst_element_set_state(pipe, GST_STATE_READY);
    gst_print("Pipeline READY: %d %s GST_STATE_CHANGE_FAILURE(%d)\n", ret,
            (ret == GST_STATE_CHANGE_FAILURE) ? "==" : "!=", GST_STATE_CHANGE_FAILURE);


    g_timeout_add(100, (GSourceFunc)GetStats, rtc);
    gst_object_unref(rtc);

    /* Start the process */
    ret = gst_element_set_state(pipe, GST_STATE_PLAYING);
    gst_print("Pipeline PLAYING: %d %s GST_STATE_CHANGE_FAILURE(%d)\n", ret,
            (ret == GST_STATE_CHANGE_FAILURE) ? "==" : "!=", GST_STATE_CHANGE_FAILURE);

    /* Wait until error or EOS */
    auto bus = gst_element_get_bus(pipe);
    GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS(GST_BIN(pipe), GST_DEBUG_GRAPH_SHOW_ALL, "guojx.observe");
    auto msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, (GstMessageType)(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

    if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_ERROR)
        g_error("Got an ERROR!!");

    gst_message_unref(msg);
    gst_object_unref(bus);
    gst_element_set_state(pipe, GST_STATE_NULL);
    gst_object_unref(pipe);

    return 0;
}
