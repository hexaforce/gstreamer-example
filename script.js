const dataChannel = {};

function dataChannelHandler(channel) {
  channel.onopen = ({ target }) => (dataChannel[target.label] = target);
  channel.onmessage = ({ data }) =>
    console.log(channel.label + ".OnMessage", data);
}

function preferredVideoCodecs() {
  const supportedCodecs = RTCRtpReceiver.getCapabilities("video").codecs;
  const preferredOrder = [
    "video/AV1",
    "video/VP9",
    "video/H265",
    "video/H264",
    "video/VP8",
  ];
  return supportedCodecs.sort((a, b) => {
    const indexA = preferredOrder.indexOf(a.mimeType);
    const indexB = preferredOrder.indexOf(b.mimeType);
    const orderA = indexA >= 0 ? indexA : Number.MAX_VALUE;
    const orderB = indexB >= 0 ? indexB : Number.MAX_VALUE;
    return orderA - orderB;
  });
}
