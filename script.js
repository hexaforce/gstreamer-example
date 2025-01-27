const $ = (id) => document.getElementById(id)
const d = document.documentElement

const messageChannel = (channel) => {
  channel.onopen = (event) => console.log(channel.label + '.OnOpen', event)
  channel.onmessage = (event) => console.log(channel.label + '.OnMessage', event)
  channel.onerror = (error) => console.log(channel.label + '.OnError:', error)
  channel.onclose = (event) => console.log(channel.label + '.OnClose', event)
}

document.addEventListener('fullscreenchange', () => {
  console.log('Fullscreen mode changed')
})

const getIpVersion = (ip) => (/^(\d{1,3}\.){3}\d{1,3}$/.test(ip) ? 'IPv4' : 'IPv6')

const resolveStats = (stats, obj, keys) => {
  keys.forEach((key) => {
    if (obj[key + 'Id']) obj[key] = stats.get(obj[key + 'Id'])
  })
}

const transport = (stats, transport) => {
  resolveStats(stats, transport, ['selectedCandidatePair', 'localCertificate', 'remoteCertificate'])
  const pair = transport.selectedCandidatePair
  if (pair) {
    resolveStats(stats, pair, ['localCandidate', 'remoteCandidate'])
    if (pair.localCandidate) pair.localCandidate.ipVersion = getIpVersion(pair.localCandidate.ip)
    if (pair.remoteCandidate) pair.remoteCandidate.ipVersion = getIpVersion(pair.remoteCandidate.ip)
  }
  return transport
}

const rtp = (stats, rtp) => {
  resolveStats(stats, rtp, ['codec', 'transport', 'playout', 'mediaSource', 'remote'])
  if (rtp.remote && rtp.remote.codecId) rtp.remote.codec = stats.get(rtp.remote.codecId)
  if (rtp.transport) rtp.transport = transport(stats, stats.get(rtp.transportId))
  return JSON.stringify(rtp, null, 2).replaceAll('"', '')
}

const prev = {}
var bitrates = {}

const measureStreamingMetrics = (report) => {
  let totalBytes = 0,
    totalFrames = 0,
    latestTimestamp = 0
  const { type, kind } = report
  let key = `${type}-${kind}`
  if (kind !== 'audio') totalFrames = 0
  if (type === 'inbound-rtp') {
    const { bytesReceived, framesDecoded, timestamp } = report
    totalBytes += bytesReceived
    if (kind !== 'audio') totalFrames += framesDecoded
    latestTimestamp = Math.max(latestTimestamp, timestamp || 0)
  } else if (type === 'outbound-rtp') {
    const { bytesSent, framesEncoded, timestamp } = report
    totalBytes += bytesSent
    if (kind !== 'audio') totalFrames += framesEncoded
    latestTimestamp = Math.max(latestTimestamp, timestamp || 0)
  }
  const prevEntry = prev[key] || { totalBytes: 0, totalFrames: 0, latestTimestamp: 0 }
  const timeDiff = latestTimestamp - prevEntry.latestTimestamp
  if (timeDiff <= 0) return prev[key]
  const bytesDiff = totalBytes - prevEntry.totalBytes
  const framesDiff = totalFrames - prevEntry.totalFrames
  const bitrate = (bytesDiff * 8) / (timeDiff / 1000)
  const fps = kind !== 'audio' ? framesDiff / (timeDiff / 1000) : 0
  prev[key] = { fps, bitrate, totalBytes, totalFrames, latestTimestamp }
  var reproductionTime = '0:00'
  if (startTimestamp) {
    const elapsedTime = latestTimestamp - startTimestamp
    if (elapsedTime > 0) {
      const totalSeconds = Math.floor(elapsedTime / 1000)
      const minutes = Math.floor(totalSeconds / 60)
      const seconds = totalSeconds % 60
      reproductionTime = `${minutes}:${seconds.toString().padStart(2, '0')}`
    }
  }
  return {
    fps: fps.toFixed(2),
    bitrate: (bitrate / 1000000).toFixed(2), // bps -> Mbps
    totalBytes: (totalBytes / (1024 * 1024)).toFixed(2), // byte -> Mbyte
    totalFrames: totalFrames.toLocaleString(),
    reproductionTime: reproductionTime,
  }
}

const metricsDisplay = (stats, report) => {
  const { type, kind } = report
  const key = `${type}-${kind}`
  if (kind === 'video') {
    $(key).innerText = rtp(stats, report)
    const metrics = measureStreamingMetrics(report)
    bitrates[key] = metrics.bitrate
    $(`${key}-chart`).firstChild.data = `${key}    ReproductionTime:${metrics.reproductionTime}  Speed:${metrics.bitrate}Mbps  TotalBytes:${metrics.totalBytes}MB  FPS:${metrics.fps}  TotalFrames:${metrics.totalFrames}`
  }
  if (kind === 'audio') {
    $(key).innerText = rtp(stats, report)
    const metrics = measureStreamingMetrics(report)
    bitrates[key] = metrics.bitrate
    $(`${key}-chart`).firstChild.data = `${key}    ReproductionTime:${metrics.reproductionTime}  Speed:${metrics.bitrate}Mbps  TotalBytes:${metrics.totalBytes}MB`
  }
}

var n = 40,
  random = d3.random.normal(0, 0)

function chart(id, xdomain, ydomain, interpolation, tick) {
  var data = d3.range(n).map(random)
  var margin = { top: 6, right: 0, bottom: 6, left: 40 },
    width = 960 - margin.right,
    height = 120 - margin.top - margin.bottom
  var x = d3.scale.linear().domain(xdomain).range([0, width])
  var y = d3.scale.linear().domain(ydomain).range([height, 0])
  var line = d3.svg
    .line()
    .interpolate(interpolation)
    .x((d, i) => {
      return x(i)
    })
    .y((d, i) => {
      return y(d)
    })
  var svg = d3.select(id).append('p').append('svg')
  svg.attr('width', width + margin.left + margin.right).attr('height', height + margin.top + margin.bottom)
  svg.append('g').attr('transform', 'translate(' + margin.left + ',' + margin.top + ')')
  svg.append('defs').append('clipPath').attr('id', 'clip').append('rect').attr('width', width).attr('height', height)
  svg.append('g').attr('class', 'y axis').call(d3.svg.axis().scale(y).ticks(5).orient('left'))
  var path = svg.append('g').attr('clip-path', 'url(#clip)').append('path').datum(data).attr('class', 'line').attr('d', line)
  tick(path, line, data, x)
}

var conn
var startTimestamp

const load = () => {
  var ws = new WebSocket(`${window.location.protocol}ws`)
  ws.onmessage = async (event) => {
    try {
      const { type, data } = JSON.parse(event.data)
      if (!conn) {
        conn = new RTCPeerConnection({ iceServers: [{ urls: 'stun:stun.l.google.com:19302' }] })
        messageChannel(conn.createDataChannel('control-command-channel', null))
        conn.ondatachannel = ({ channel }) => messageChannel(channel)
        conn.ontrack = ({ streams }) => (document.getElementById('stream').srcObject = streams[0])
        conn.onicecandidate = ({ candidate }) => candidate && ws.send(JSON.stringify({ type: 'ice', data: candidate }))
        conn.oniceconnectionstatechange = ({ target }) => {
          const { connectionState, iceConnectionState, remoteDescription, localDescription } = target
          if (iceConnectionState == 'connected') {
            startTimestamp = Date.now()
            console.log('Local SDP:\n', localDescription.sdp)
            console.log('Remote SDP:\n', remoteDescription.sdp)
          }
        }
      }
      if (type == 'sdp') {
        await conn.setRemoteDescription(data)
        const desc = await conn.createAnswer()
        await conn.setLocalDescription(desc)
        ws.send(JSON.stringify({ type: 'sdp', data: conn.localDescription }))
      } else if (type == 'ice') {
        await conn.addIceCandidate(new RTCIceCandidate(data))
      }
    } catch (err) {
      console.error(err)
    }
  }
  $('fullscreen-btn').addEventListener('click', () => {
    if (d.requestFullscreen) {
      d.requestFullscreen()
    } else if (d.webkitRequestFullscreen) {
      d.webkitRequestFullscreen()
    }
  })
}

const interval = async () => {
  if (!conn || conn.connectionState != 'connected' || conn.iceConnectionState != 'connected') return

  let peer = null
  const dataChannels = []
  const stats = await conn.getStats(null)

  stats.forEach((report) => {
    switch (report.type) {
      case 'inbound-rtp':
      case 'outbound-rtp':
        metricsDisplay(stats, report)
        break
      case 'peer-connection':
        peer = report
        break
      case 'data-channel':
        dataChannels.push(report)
        break
      default:
        if (!['candidate-pair', 'local-candidate', 'remote-candidate', 'remote-inbound-rtp', 'remote-outbound-rtp', 'transport', 'media-source', 'media-playout', 'certificate', 'codec'].includes(report.type)) {
          console.log('unknown type:', report.type)
        }
    }
  })

  if (peer) {
    peer.dataChannels = dataChannels
    $('peer-connection').innerText = JSON.stringify(peer, null, 2).replaceAll('"', '')
  }
}
