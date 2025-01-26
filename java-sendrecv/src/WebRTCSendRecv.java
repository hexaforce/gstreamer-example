
/*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
 *
 * Copyright 2021 Neil C Smith - Codelerity Ltd.
 * Copyright 2019 Steve Vangasse 
 *
 * Copying and distribution of this file, with or without modification,
 * are permitted in any medium without royalty provided the copyright
 * notice and this notice are preserved. This file is offered as-is,
 * without any warranty.
 *
 */
import java.io.*;
import java.net.URI;
import java.util.List;
import java.util.concurrent.ThreadLocalRandom;
import java.util.logging.*;

import javax.json.*;
import javax.websocket.*;

import org.freedesktop.gstreamer.Bin;
import org.freedesktop.gstreamer.Bus;
import org.freedesktop.gstreamer.Caps;
import org.freedesktop.gstreamer.Element;
import org.freedesktop.gstreamer.Element.PAD_ADDED;
import org.freedesktop.gstreamer.ElementFactory;
import org.freedesktop.gstreamer.Gst;
import org.freedesktop.gstreamer.PadDirection;
import org.freedesktop.gstreamer.Pipeline;
import org.freedesktop.gstreamer.Registry;
import org.freedesktop.gstreamer.SDPMessage;
import org.freedesktop.gstreamer.State;
import org.freedesktop.gstreamer.Version;
import org.freedesktop.gstreamer.elements.DecodeBin;
import org.freedesktop.gstreamer.webrtc.WebRTCBin;
import org.freedesktop.gstreamer.webrtc.WebRTCBin.CREATE_OFFER;
import org.freedesktop.gstreamer.webrtc.WebRTCBin.ON_ICE_CANDIDATE;
import org.freedesktop.gstreamer.webrtc.WebRTCBin.ON_NEGOTIATION_NEEDED;
import org.freedesktop.gstreamer.webrtc.WebRTCBundlePolicy;
import org.freedesktop.gstreamer.webrtc.WebRTCSDPType;
import org.freedesktop.gstreamer.webrtc.WebRTCSessionDescription;

/**
 * Demo GStreamer app for negotiating and streaming a sendrecv webrtc stream
 * with a browser JS app.
 *
 * @author Neil C Smith ( https://www.codelerity.com )
 * @author Steve Vangasse
 */
@ClientEndpoint
public class WebRTCSendRecv {

	private static final Logger LOG = Logger.getLogger(WebRTCSendRecv.class.getName());

	// private static final String REMOTE_SERVER_URL = "wss://webrtc.nirbheek.in:8443";
	private static final String REMOTE_SERVER_URL = "ws://signaling:8443";
	// private static final String REMOTE_SERVER_URL = "ws://localhost:8443";

	public static final String VIDEO_BIN_DESCRIPTION_VP8 = """
			videotestsrc is-live=true !
			videoconvert !
			queue !
			vp8enc deadline=1 !
			rtpvp8pay !
			queue !
			capsfilter caps=application/x-rtp,media=video,encoding-name=VP8,payload=97
			""";

	public static final String VIDEO_BIN_DESCRIPTION_VP9 = """
			videotestsrc is-live=true !
			videoconvert !
			videoscale !
			video/x-raw,format=I420 !
			queue !
			vp9enc deadline=1 !
			rtpvp9pay !
			queue !
			capsfilter caps=application/x-rtp,media=video,encoding-name=VP9,payload=97
			""";

	public static final String VIDEO_BIN_DESCRIPTION_H264 = """
			videotestsrc is-live=true !
			videoconvert !
			queue !
			x264enc bitrate=600 speed-preset=ultrafast tune=zerolatency key-int-max=15 !
			rtph264pay !
			queue !
			capsfilter caps=application/x-rtp,media=video,encoding-name=H264,payload=97
			""";

	static final String AUDIO_BIN_DESCRIPTION = """
			audiotestsrc is-live=true !
			audioconvert !
			audioresample !
			queue !
			opusenc perfect-timestamp=true !
			rtpopuspay !
			queue !
			capsfilter caps=application/x-rtp,media=audio,encoding-name=OPUS,payload=96
			""";

	private final String serverUrl;
	private final String peerId;

	private Session session;
	private WebRTCBin webRTCBin;
	private Pipeline pipe;

	public static void main(String[] args) throws Exception {
		if (args.length == 0) {
			LOG.log(Level.SEVERE,
					"Please pass at least the peer-id from the signaling server e.g java -jar target/webrtc-sendrecv-java-1.5.0-SNAPSHOT-jar-with-dependencies.jar --peer-id=1234 --server=wss://webrtc.gstreamer.net:8443");
			return;
		}

		Registry registry = Registry.get();
		for (String element : List.of("opusenc", "nicesink", "webrtcbin", "dtlssrtpenc", "srtpenc", "rtpbin",
				"rtpopuspay")) {
			if (registry.lookupFeature(element) == null) {
				LOG.log(Level.SEVERE, String.format("Error: not found %s", element));
				// System.exit(1);
			}
		}

		String serverUrl = REMOTE_SERVER_URL;
		String peerId = null;
		for (String arg : args) {
			if (arg.startsWith("--server=")) {
				serverUrl = arg.substring("--server=".length());
			} else if (arg.startsWith("--peer-id=")) {
				peerId = arg.substring("--peer-id=".length());
			}
		}

		if (peerId == null) {
			LOG.log(Level.SEVERE,
					"Please pass at least the peer-id from the signaling server e.g docker exec -it gst-java-sendrecv-1 java -jar app --peer-id=1 --peer-id=1234 --server=wss://webrtc.gstreamer.net:8443");
			System.exit(1);
		}

		// Uncomment to output GStreamer debug information
		// GLib.setEnv("GST_DEBUG", "4", true);

		// Initialize GStreamer with minimum version of 1.16.
		Gst.init(Version.of(1, 24));

		// Initialize call - make sure webpage is set to allow audio in browser
		WebRTCSendRecv webrtcSendRecv = new WebRTCSendRecv(peerId, serverUrl);
		webrtcSendRecv.startCall(webrtcSendRecv);
	}

	private WebRTCSendRecv(String peerId, String serverUrl) {
		this.peerId = peerId;
		this.serverUrl = serverUrl;
		Bin video = Gst.parseBinFromDescription(VIDEO_BIN_DESCRIPTION_H264, true);
		Bin audio = Gst.parseBinFromDescription(AUDIO_BIN_DESCRIPTION, true);

		pipe = new Pipeline();
		webRTCBin = new WebRTCBin("webrtcbin");
		webRTCBin.setBundlePolicy(WebRTCBundlePolicy.MAX_BUNDLE);
		webRTCBin.setStunServer("stun://stun.l.google.com:19302");
		webRTCBin.setTurnServer("turn://gstreamer:IsGreatWhenYouCanGetItToWork@webrtc.nirbheek.in:3478");
		pipe.addMany(webRTCBin, video, audio);
		video.link(webRTCBin);
		audio.link(webRTCBin);

		setupPipeLogging(pipe);

		// When the pipeline goes to PLAYING, the on_negotiation_needed() callback
		// will be called, and we will ask webrtcbin to create an offer which will
		// match the pipeline above.
		webRTCBin.connect(onNegotiationNeeded);
		webRTCBin.connect(onIceCandidate);
		webRTCBin.connect(onIncomingStream);
	}

	private void startCall(WebRTCSendRecv webRTCSendRecv) throws Exception {
		WebSocketContainer container = ContainerProvider.getWebSocketContainer();
		container.connectToServer(webRTCSendRecv, URI.create(serverUrl));
		Gst.main();
	}

	@OnOpen
	public void onOpen(Session session) {
		this.session = session;
		LOG.info("websocket onOpen");
		int id = ThreadLocalRandom.current().nextInt(10, 10000);
		session.getAsyncRemote().sendText("HELLO " + id);
	}

	@OnClose
	public void onClose(Session session, CloseReason closeReason) {
		LOG.info(() -> "WebSocket onClose : " + closeReason.getCloseCode() + " : " + closeReason.getReasonPhrase());
		endCall();
	}

	@OnMessage
	public void onMessage(String payload) {
		if (payload.equals("HELLO")) {
			session.getAsyncRemote().sendText("SESSION " + peerId);
		} else if (payload.equals("SESSION_OK")) {
			pipe.play();
		} else if (payload.startsWith("ERROR")) {
			LOG.severe(payload);
			endCall();
		} else {
			handleSdp(payload);
		}
	}

	@OnError
	public void onError(Session session, Throwable throwable) {
		LOG.log(Level.SEVERE, "onError", throwable);
	}

	private void handleSdp(String payload) {
		JsonObject json = Json.createReader(new StringReader(payload)).readObject();
		if (json.containsKey("sdp")) {
			String sdpStr = json.getJsonObject("sdp").getString("sdp");
			LOG.info(() -> "Answer SDP:\n" + sdpStr);
			SDPMessage sdpMessage = new SDPMessage();
			sdpMessage.parseBuffer(sdpStr);
			webRTCBin.setRemoteDescription(new WebRTCSessionDescription(WebRTCSDPType.ANSWER, sdpMessage));
		} else if (json.containsKey("ice")) {
			JsonObject iceJson = json.getJsonObject("ice");
			String candidate = iceJson.getString("candidate");
			int sdpMLineIndex = iceJson.getInt("sdpMLineIndex");
			LOG.info(() -> "Adding ICE candidate : " + candidate);
			webRTCBin.addIceCandidate(sdpMLineIndex, candidate);
		}
	}

	private void setupPipeLogging(Pipeline pipe) {
		Bus bus = pipe.getBus();
		bus.connect((Bus.EOS) source -> {
			LOG.info(() -> "Reached end of stream : " + source.toString());
			endCall();
		});

		bus.connect((Bus.ERROR) (source, code, message) -> {
			LOG.severe(() -> "Error from source : " + source + ", with code : " + code + ", and message : " + message);
			endCall();
		});

		bus.connect((source, old, current, pending) -> {
			if (source instanceof Pipeline) {
				LOG.info(() -> "Pipe state changed from " + old + " to " + current);
			}
		});
	}

	private void endCall() {
		try {
			pipe.setState(State.NULL);
			session.close();
		} catch (IOException e) {
			LOG.log(Level.SEVERE, "Error while ending the call", e);
		} finally {
			Gst.quit();
		}
	}

	private CREATE_OFFER onOfferCreated = offer -> {
		webRTCBin.setLocalDescription(offer);
		String sdp = offer.getSDPMessage().toString();
		JsonObject sdpNode = Json.createObjectBuilder().add("type", "offer").add("sdp", sdp).build();
		JsonObject rootNode = Json.createObjectBuilder().add("sdp", sdpNode).build();
		try (StringWriter writer = new StringWriter(); JsonWriter jsonWriter = Json.createWriter(writer)) {
			jsonWriter.writeObject(rootNode);
			String json = writer.toString();
			LOG.info(() -> "Sending offer:\n" + json);
			session.getAsyncRemote().sendText(json);
		} catch (Exception e) {
			LOG.log(Level.SEVERE, "Couldn't write JSON", e);
		}
	};

	private final ON_NEGOTIATION_NEEDED onNegotiationNeeded = elem -> {
		LOG.info(() -> "onNegotiationNeeded: " + elem.getName());

		// When webrtcbin has created the offer, it will hit our callback and we
		// send SDP offer over the websocket to signaling server
		webRTCBin.createOffer(onOfferCreated);
	};

	private final ON_ICE_CANDIDATE onIceCandidate = (sdpMLineIndex, candidate) -> {
		JsonObject iceNode = Json.createObjectBuilder().add("sdpMLineIndex", sdpMLineIndex).add("candidate", candidate)
				.build();
		JsonObject rootNode = Json.createObjectBuilder().add("ice", iceNode).build();
		try (StringWriter writer = new StringWriter(); JsonWriter jsonWriter = Json.createWriter(writer)) {
			jsonWriter.writeObject(rootNode);
			String json = writer.toString();
			LOG.info(() -> "ON_ICE_CANDIDATE: " + json);
			session.getAsyncRemote().sendText(json);
		} catch (Exception e) {
			LOG.log(Level.SEVERE, "Couldn't write JSON", e);
		}
	};

	private final PAD_ADDED onDecodedStream = (element, pad) -> {
		if (!pad.hasCurrentCaps()) {
			LOG.info("Pad has no current Caps - ignoring");
			return;
		}
		Caps caps = pad.getCurrentCaps();
		LOG.info(() -> "Received decoded stream with caps : " + caps.toString());
		if (caps.isAlwaysCompatible(Caps.fromString("video/x-raw"))) {
			Element q = ElementFactory.make("queue", "videoqueue");
			Element conv = ElementFactory.make("videoconvert", "videoconvert");
			Element sink = ElementFactory.make("autovideosink", "videosink");
			pipe.addMany(q, conv, sink);
			q.syncStateWithParent();
			conv.syncStateWithParent();
			sink.syncStateWithParent();
			pad.link(q.getStaticPad("sink"));
			q.link(conv);
			conv.link(sink);
		} else if (caps.isAlwaysCompatible(Caps.fromString("audio/x-raw"))) {
			Element q = ElementFactory.make("queue", "audioqueue");
			Element conv = ElementFactory.make("audioconvert", "audioconvert");
			Element resample = ElementFactory.make("audioresample", "audioresample");
			Element sink = ElementFactory.make("autoaudiosink", "audiosink");
			pipe.addMany(q, conv, resample, sink);
			q.syncStateWithParent();
			conv.syncStateWithParent();
			resample.syncStateWithParent();
			sink.syncStateWithParent();
			pad.link(q.getStaticPad("sink"));
			q.link(conv);
			conv.link(resample);
			resample.link(sink);
		}
	};

	private final PAD_ADDED onIncomingStream = (element, pad) -> {
		LOG.info(() -> "Receiving stream! Element : " + element.getName() + " Pad : " + pad.getName());
		if (pad.getDirection() != PadDirection.SRC) {
			return;
		}
		DecodeBin decodeBin = new DecodeBin("decodebin_" + pad.getName());
		decodeBin.connect(onDecodedStream);
		pipe.add(decodeBin);
		decodeBin.syncStateWithParent();
		pad.link(decodeBin.getStaticPad("sink"));
	};

}
