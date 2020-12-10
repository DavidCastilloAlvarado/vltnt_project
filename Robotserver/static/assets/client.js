ip_global = location.hostname; // Ip del servidor (raspi)
port_webrtc = 9000; // Port del servidor webRTC(raspi)
API_KEY = "BGsvAYWtkRmKa507TcThwD93NjeLHdmz";
const video = document.getElementById("remote-video");
const canvas = document.getElementById("c");
const ctx = canvas.getContext("2d");

var signalling_server_hostname = location.hostname || ip_global;
var ws = null;
var pc;
var datachannel;
var recorder = null;
var recordedBlobs;
var pcConfig = {
  iceServers: [
    { urls: ["stun:" + signalling_server_hostname + ":3478"] }, //"stun:stun.l.google.com:19302",
  ],
};
var pcOptions = {
  optional: [],
};
var mediaConstraints = {
  optional: [],
  mandatory: {
    OfferToReceiveAudio: true,
    OfferToReceiveVideo: true,
  },
};
var keys = [];
var trickle_ice = true;
var remoteDesc = false;
var iceCandidates = [];

RTCPeerConnection =
  window.RTCPeerConnection ||
  /*window.mozRTCPeerConnection ||*/ window.webkitRTCPeerConnection;
RTCSessionDescription =
  /*window.mozRTCSessionDescription ||*/ window.RTCSessionDescription;
RTCIceCandidate = /*window.mozRTCIceCandidate ||*/ window.RTCIceCandidate;
navigator.getUserMedia =
  navigator.getUserMedia ||
  navigator.mozGetUserMedia ||
  navigator.webkitGetUserMedia ||
  navigator.msGetUserMedia;
var URL = window.URL || window.webkitURL;

function createPeerConnection() {
  try {
    var pcConfig_ = pcConfig;
    console.log(JSON.stringify(pcConfig_));
    pc = new RTCPeerConnection(pcConfig_, pcOptions);
    pc.onicecandidate = onIceCandidate;

    if ("ontrack" in pc) {
      pc.ontrack = onTrack;
    } else {
      pc.onaddstream = onRemoteStreamAdded; // deprecated
    }
    pc.onremovestream = onRemoteStreamRemoved;
    //pc.ondatachannel = onDataChannel;
    console.log("peer connection successfully created!");
  } catch (e) {
    console.error("createPeerConnection() failed");
  }
}

function onIceCandidate(event) {
  if (event.candidate) {
    var candidate = {
      sdpMLineIndex: event.candidate.sdpMLineIndex,
      sdpMid: event.candidate.sdpMid,
      candidate: event.candidate.candidate,
    };
    var request = {
      what: "addIceCandidate",
      data: JSON.stringify(candidate),
    };
    ws.send(JSON.stringify(request));
  } else {
    console.log("End of candidates.");
    //send_message() //Modification1
  }
}

function addIceCandidates() {
  iceCandidates.forEach(function (candidate) {
    pc.addIceCandidate(
      candidate,
      function () {
        console.log("IceCandidate added: " + JSON.stringify(candidate));
      },
      function (error) {
        console.error("addIceCandidate error: " + error);
      }
    );
  });
  iceCandidates = [];
}

function onRemoteStreamAdded(event) {
  console.log("Remote stream added:", URL.createObjectURL(event.stream));
  var remoteVideoElement = document.getElementById("remote-video");
  remoteVideoElement.src = URL.createObjectURL(event.stream);
  remoteVideoElement.play();
}

function onTrack(event) {
  console.log("Remote track!");
  var remoteVideoElement = document.getElementById("remote-video");
  remoteVideoElement.srcObject = event.streams[0];
  var playPromise = remoteVideoElement.play();
  if (playPromise !== undefined) {
    playPromise
      .then((_) => {
        console.log("Reproduciendo");
        document.getElementById("datachannels").disabled = false;
      })
      .catch((error) => {
        // Auto-play was prevented
        console.log("No se puede reproducir");
      });
  }
}

function onRemoteStreamRemoved(event) {
  var remoteVideoElement = document.getElementById("remote-video");
  remoteVideoElement.srcObject = null;
  remoteVideoElement.src = ""; // TODO: remove
}

function start() {
  if ("WebSocket" in window) {
    document.getElementById("stop").disabled = false;
    document.getElementById("start").disabled = true;
    document.documentElement.style.cursor = "wait";

    var protocol = location.protocol === "https:" ? "wss:" : "ws:";
    ws = new WebSocket(
      protocol + "//" + ip_global + ":" + port_webrtc + "/stream/webrtc"
    );

    function call(stream) {
      iceCandidates = [];
      remoteDesc = false;
      createPeerConnection();
      var request = {
        what: "call",
        options: {
          force_hw_vcodec: document.getElementById("remote_hw_vcodec").checked,
          vformat: document.getElementById("remote_vformat").value,
          trickle_ice: true,
        },
      };
      ws.send(JSON.stringify(request));
      console.log("call(), request=" + JSON.stringify(request));
    }

    ws.onopen = function () {
      console.log("onopen()");
      call();
    };

    ws.onmessage = function (evt) {
      var msg = JSON.parse(evt.data);
      if (msg.what !== "undefined") {
        var what = msg.what;
        var data = msg.data;
      }
      //console.log("message=" + msg);
      console.log("message =" + what);

      switch (what) {
        case "offer":
          pc.setRemoteDescription(
            new RTCSessionDescription(JSON.parse(data)),
            function onRemoteSdpSuccess() {
              remoteDesc = true;
              addIceCandidates();
              console.log("onRemoteSdpSucces()");
              pc.createAnswer(
                function (sessionDescription) {
                  pc.setLocalDescription(sessionDescription);
                  var request = {
                    what: "answer",
                    data: JSON.stringify(sessionDescription),
                  };
                  ws.send(JSON.stringify(request));
                  console.log(request);
                },
                function (error) {
                  alert("Failed to createAnswer: " + error);
                },
                mediaConstraints
              );
            },
            function onRemoteSdpError(event) {
              alert(
                "Failed to set remote description (unsupported codec on this browser?): " +
                  event
              );
              stop();
            }
          );

          /*
                               * No longer needed, it's implicit in "call"
                              var request = {
                                  what: "generateIceCandidates"
                              };
                              console.log(request);
                              ws.send(JSON.stringify(request));
                              */
          break;

        case "answer":
          break;

        case "message":
          alert(msg.data);
          break;

        case "iceCandidate": // when trickle is enabled
          if (!msg.data) {
            console.log("Ice Gathering Complete");
            break;
          }
          var elt = JSON.parse(msg.data);
          let candidate = new RTCIceCandidate({
            sdpMLineIndex: elt.sdpMLineIndex,
            candidate: elt.candidate,
          });
          iceCandidates.push(candidate);
          if (remoteDesc) addIceCandidates();
          document.documentElement.style.cursor = "default";
          break;

        case "iceCandidates": // when trickle ice is not enabled
          var candidates = JSON.parse(msg.data);
          for (var i = 0; candidates && i < candidates.length; i++) {
            var elt = candidates[i];
            let candidate = new RTCIceCandidate({
              sdpMLineIndex: elt.sdpMLineIndex,
              candidate: elt.candidate,
            });
            iceCandidates.push(candidate);
          }
          if (remoteDesc) addIceCandidates();
          document.documentElement.style.cursor = "default";
          break;
      }
    };

    ws.onclose = function (evt) {
      if (pc) {
        pc.close();
        pc = null;
      }
      document.getElementById("stop").disabled = true;
      document.getElementById("start").disabled = false;
      document.documentElement.style.cursor = "default";
    };

    ws.onerror = function (evt) {
      alert("An error has occurred!");
      ws.close();
    };
  } else {
    alert("Sorry, this browser does not support WebSockets.");
  }
}

function stop() {
  if (!document.getElementById("datachannels").disabled) {
    document.getElementById("datachannels").disabled = true;
    console.log("closing data channels");
    document.getElementById("keypresssend").checked = false;
    keypresssend_selection();
  }

  stop_record();
  document.getElementById("remote-video").srcObject = null;
  document.getElementById("remote-video").src = ""; // TODO; remove
  if (pc) {
    pc.close();
    pc = null;
  }
  if (ws) {
    ws.close();
    ws = null;
  }
  document.getElementById("stop").disabled = true;
  document.getElementById("start").disabled = false;
  document.documentElement.style.cursor = "default";
}

function mute() {
  var remoteVideo = document.getElementById("remote-video");
  remoteVideo.muted = !remoteVideo.muted;
}

function pause() {
  var remoteVideo = document.getElementById("remote-video");
  if (remoteVideo.paused) remoteVideo.play();
  else remoteVideo.pause();
}

function fullscreen() {
  var remoteVideo = document.getElementById("c"); // Llamando al canvas
  if (remoteVideo.requestFullScreen) {
    remoteVideo.requestFullScreen();
  } else if (remoteVideo.webkitRequestFullScreen) {
    remoteVideo.webkitRequestFullScreen();
  } else if (remoteVideo.mozRequestFullScreen) {
    remoteVideo.mozRequestFullScreen();
  }
}

function handleDataAvailable(event) {
  //console.log(event);
  if (event.data && event.data.size > 0) {
    recordedBlobs.push(event.data);
  }
}

function handleStop(event) {
  console.log("Recorder stopped: ", event);
  document.getElementById("record").innerHTML = "Start Recording";
  recorder = null;
  var superBuffer = new Blob(recordedBlobs, { type: "video/webm" });
  var recordedVideoElement = document.getElementById("recorded-video");
  recordedVideoElement.src = URL.createObjectURL(superBuffer);
}

function discard_recording() {
  var recordedVideoElement = document.getElementById("recorded-video");
  recordedVideoElement.srcObject = null;
  recordedVideoElement.src = "";
}

function stop_record() {
  if (recorder) {
    recorder.stop();
    console.log("recording stopped");
    document.getElementById("record-detail").open = true;
  }
}

function startRecording(stream) {
  recordedBlobs = [];
  var options = { mimeType: "video/webm;codecs=h264" };
  if (!MediaRecorder.isTypeSupported(options.mimeType)) {
    console.log(options.mimeType + " is not Supported");
    options = { mimeType: "video/webm;codecs=vp8" };
    if (!MediaRecorder.isTypeSupported(options.mimeType)) {
      console.log(options.mimeType + " is not Supported");
      options = { mimeType: "video/webm;codecs=vp9" };
      if (!MediaRecorder.isTypeSupported(options.mimeType)) {
        console.log(options.mimeType + " is not Supported");
        options = { mimeType: "video/webm" };
        if (!MediaRecorder.isTypeSupported(options.mimeType)) {
          console.log(options.mimeType + " is not Supported");
          options = { mimeType: "" };
        }
      }
    }
  }
  try {
    recorder = new MediaRecorder(stream, options);
  } catch (e) {
    console.error("Exception while creating MediaRecorder: " + e);
    alert(
      "Exception while creating MediaRecorder: " +
        e +
        ". mimeType: " +
        options.mimeType
    );
    return;
  }
  console.log("Created MediaRecorder", recorder, "with options", options);
  //recorder.ignoreMutedMedia = true;
  recorder.onstop = handleStop;
  recorder.ondataavailable = handleDataAvailable;
  recorder.onwarning = function (e) {
    console.log("Warning: " + e);
  };
  recorder.start();
  console.log("MediaRecorder started", recorder);
}

function start_stop_record() {
  if (pc && !recorder) {
    var streams = pc.getRemoteStreams();
    if (streams.length) {
      console.log("starting recording");
      startRecording(streams[0]);
      document.getElementById("record").innerHTML = "Stop Recording";
    }
  } else {
    stop_record();
  }
}

function download() {
  if (recordedBlobs !== undefined) {
    var blob = new Blob(recordedBlobs, { type: "video/webm" });
    var url = window.URL.createObjectURL(blob);
    var a = document.createElement("a");
    a.style.display = "none";
    a.href = url;
    a.download = "video.webm";
    document.body.appendChild(a);
    a.click();
    setTimeout(function () {
      document.body.removeChild(a);
      window.URL.revokeObjectURL(url);
    }, 100);
  }
}

function remote_hw_vcodec_selection() {
  if (!document.getElementById("remote_hw_vcodec").checked)
    unselect_remote_hw_vcodec();
  else select_remote_hw_vcodec();
}

function remote_hw_vcodec_format_selection() {
  if (document.getElementById("remote_hw_vcodec").checked)
    remote_hw_vcodec_selection();
}

function select_remote_hw_vcodec() {
  document.getElementById("remote_hw_vcodec").checked = true;
  var vformat = document.getElementById("remote_vformat").value;
  switch (vformat) {
    case "5":
      document.getElementById("remote-video").style.width = "320px";
      document.getElementById("remote-video").style.height = "240px";
      break;
    case "10":
      document.getElementById("remote-video").style.width = "320px";
      document.getElementById("remote-video").style.height = "240px";
      break;
    case "20":
      document.getElementById("remote-video").style.width = "352px";
      document.getElementById("remote-video").style.height = "288px";
      break;
    case "25":
      document.getElementById("remote-video").style.width = "640px";
      document.getElementById("remote-video").style.height = "480px";
      break;
    case "30":
      document.getElementById("remote-video").style.width = "640px";
      document.getElementById("remote-video").style.height = "480px";
      break;
    case "35":
      document.getElementById("remote-video").style.width = "800px";
      document.getElementById("remote-video").style.height = "480px";
      break;
    case "40":
      document.getElementById("remote-video").style.width = "960px";
      document.getElementById("remote-video").style.height = "720px";
      break;
    case "50":
      document.getElementById("remote-video").style.width = "1024px";
      document.getElementById("remote-video").style.height = "768px";
      break;
    case "55":
      document.getElementById("remote-video").style.width = "1280px";
      document.getElementById("remote-video").style.height = "720px";
      break;
    case "60":
      document.getElementById("remote-video").style.width = "1280px";
      document.getElementById("remote-video").style.height = "720px";
      break;
    case "63":
      document.getElementById("remote-video").style.width = "1280px";
      document.getElementById("remote-video").style.height = "720px";
      break;
    case "65":
      document.getElementById("remote-video").style.width = "1280px";
      document.getElementById("remote-video").style.height = "768px";
      break;
    case "70":
      document.getElementById("remote-video").style.width = "1280px";
      document.getElementById("remote-video").style.height = "768px";
      break;
    case "80":
      document.getElementById("remote-video").style.width = "1280px";
      document.getElementById("remote-video").style.height = "960px";
      break;
    case "90":
      document.getElementById("remote-video").style.width = "1600px";
      document.getElementById("remote-video").style.height = "768px";
      break;
    case "95":
      document.getElementById("remote-video").style.width = "1640px";
      document.getElementById("remote-video").style.height = "1232px";
      break;
    case "97":
      document.getElementById("remote-video").style.width = "1640px";
      document.getElementById("remote-video").style.height = "1232px";
      break;
    case "100":
      document.getElementById("remote-video").style.width = "1920px";
      document.getElementById("remote-video").style.height = "1080px";
      break;
    case "105":
      document.getElementById("remote-video").style.width = "1920px";
      document.getElementById("remote-video").style.height = "1080px";
      break;
    default:
      document.getElementById("remote-video").style.width = "1280px";
      document.getElementById("remote-video").style.height = "720px";
  }
  /*
       // Disable video casting. Not supported at the moment with hw codecs.
       var elements = document.getElementsByName('video_cast');
       for(var i = 0; i < elements.length; i++) {
       elements[i].checked = false;
       }
       */
}

function unselect_remote_hw_vcodec() {
  document.getElementById("remote_hw_vcodec").checked = false;
  document.getElementById("remote-video").style.width = "640px";
  document.getElementById("remote-video").style.height = "480px";
}

function requestAnimationFrame2(callback) {
  return setTimeout(callback, 1000);
}
var sended = 0;
function send_control(data_send, dest) {
  // url_ = 'http://' + ip_global + ':8010/control/' + dest
  url_ = location.origin + "/control/" + dest;
  $.ajax({
    type: "POST",
    url: url_,
    // The key needs to match your method's input parameter (case-sensitive).
    data: JSON.stringify(data_send),
    contentType: "application/json; charset=utf-8",
    dataType: "json",
    success: function (data) {
      document.getElementById("control_resp").value = data["message"];
      sended = 0;
    },
    failure: function (errMsg) {
      sended = 0;
      alert(errMsg);
    },
  });
}

var time_ = +new Date();
var speed = 0;
function message(dir, speed) {
  if (+new Date() - time_ > 200 && sended == 0) {
    if (speed == -1) {
      console.log(dir);
      var move_data = { order: dir };
      send_control(move_data, "arm");
    } else {
      console.log(dir);
      var move_data = { D: dir, S: speed, key: API_KEY };
      send_control(move_data, "car");
    }
    time_ = +new Date();
    sended = 1;
  }
}

function keydown(e) {
  if (e.keyCode == 0 || e.keyCode == 229) {
    // on mobile
    return;
  }
  e.preventDefault();
  e.stopPropagation();
  e.stopImmediatePropagation();
  //keys[e.keyCode] = e.keyCode;
  //clearTimeout(time_out);
  // Teclas para controlar el movimiento del carro
  if (event.keyCode == 38) {
    // fecha hacia arriva
    return message(1, speed);
  } else if (event.keyCode == 40) {
    // flecha hacia abajo
    return message(2, speed);
  } else if (event.keyCode == 39) {
    // flecha hacia la derecha
    return message(3, speed);
  } else if (event.keyCode == 37) {
    // flecha hacia la izquierda
    return message(4, speed);
  } else if (event.keyCode == 187) {
    // tecla de suma, en el  cuerpo del teclado
    speed = speed + 1;
    if (speed > 9) {
      speed = 9;
    }
  } else if (event.keyCode == 189) {
    // tecla de resta, en el cuerpo del teclado
    speed = speed - 1;
    if (speed < 0) {
      speed = 0;
    }
  } // Keys para controlar el robot/////////////////////
  else if (event.keyCode == 87) {
    //w -- avance en L del brazo
    return message("w", -1);
  } else if (event.keyCode == 83) {
    //s
    return message("s", -1);
  } else if (event.keyCode == 81) {
    //q -- avance en Z con el brazo
    return message("q", -1);
  } else if (event.keyCode == 69) {
    //e
    return message("e", -1);
  } else if (event.keyCode == 77) {
    //m -- inclinacion de la camara
    return message("m", -1);
  } else if (event.keyCode == 78) {
    //n
    return message("n", -1);
  } else if (event.keyCode == 65) {
    //a -- rotar el brazo theta1
    return message("a", -1);
  } else if (event.keyCode == 68) {
    //d
    return message("d", -1);
  }
  // comandos para la camara
  else if (event.keyCode == 73) {
    //i -- camara apunta hacia arriva
    return message("i", -1);
  } else if (event.keyCode == 75) {
    //k
    return message("k", -1);
  } else if (event.keyCode == 74) {
    //j --camara apunta hacia la izquierda
    return message("j", -1);
  } else if (event.keyCode == 76) {
    //l
    return message("l", -1);
  }
  // FIn de comandos para la camara
  else if (event.keyCode == 188) {
    //coma(,) aumenta dp, diferencial de movimiento del robot.
    return message("sm", -1);
  } else if (event.keyCode == 190) {
    //punto(.) -- disminuye el dp
    return message("rt", -1);
  } else if (event.keyCode == 79) {
    // o  abre gripper
    return message("op", -1);
  } else if (event.keyCode == 80) {
    // p cierra griper
    return message("cl", -1);
  }
}

function keypresssend_selection() {
  if (document.getElementById("keypresssend").checked) {
    window.addEventListener("keydown", keydown, true);
  } else {
    keys = [];
    window.removeEventListener("keydown", keydown, true);
  }
}

window.onload = function () {
  // En caso el navegador no pueda grabar el streaming
  if (window.MediaRecorder === undefined) {
    document.getElementById("record").disabled = true;
  }
  if (false) {
    start();
  }
};

window.onbeforeunload = function () {
  // Detiene la comunicación antes de cerrar la pagina
  if (ws) {
    ws.onclose = function () {}; // disable onclose handler first
    stop();
  }
};

function read_data() {
  document.getElementById("readdata").disabled = true;
  try {
    $.ajax({
      // url: 'http://' + ip_global + ':8010/datos', // Es la dirección en donde se encuentra ejecutando el servidor, el cual es distinta a la dirección del WEBRTC (En caso de alojado en el raspi, el IP debe corresponder al IP del raspi)
      url: location.origin + "/datos/" + "car",
      type: "GET",
      dataType: "json",
      success: displayAll,
    });
    function displayAll(data) {
      document.getElementById("telem_robot").value =
        data.ang_x + " " + data.ang_y; //+ "    " + data.dist + " " + data.st1 + " " + data.st2;
    }
  } catch (e) {
    console.error(e);
  }

  requestAnimationFrame2(read_data);
}
////////////////////////////////////////////////////////////////////////

video.addEventListener(
  "play",
  function () {
    // Every 33 milliseconds copy the video image to the canvas
    setInterval(function () {
      if (video.paused || video.ended) {
        return;
      }
      var w = video.getAttribute("width");
      var h = video.getAttribute("height");
      ctx.fillRect(0, 0, w, h);
      ctx.drawImage(video, 0, 0, w, h);
      ctx.fillStyle = "black";
      ctx.font = "20px Arial";
      var text = document.getElementById("telem_robot").value;
      var text2 = document.getElementById("control_resp").value;
      ctx.strokeText(text, 20, 20);
      ctx.strokeText(text2, 20, h - 20);
    }, 33);
  },
  false
);
