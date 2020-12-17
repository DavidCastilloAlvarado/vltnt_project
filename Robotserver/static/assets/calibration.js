
var sended = 0;
API_KEY = "BGsvAYWtkRmKa507TcThwD93NjeLHdmz";
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
      document.getElementById("status_panel").innerHTML  = "Mover el robot formando un ocho";
      setTimeout(() => { 
            document.getElementById("status_panel").innerHTML  = "Finalizado ...";
            sended = 0; 
        }, 5000);
      
    },
    statusCode:{
        400:function(){
            document.getElementById("status_panel").innerHTML  = "Error al calibrar";
            alert("Error al calibrar")
            sended = 0;
        }
    }
  });
}

var time_ = +new Date();
var speed = 0;
function message(dir, speed) {
  if (+new Date() - time_ > 200 && sended == 0) {
    if(speed == -2){
      console.log(dir);
      var aux_data = { aux: dir, key: API_KEY  };
      send_control(aux_data, "car");
    }
    time_ = +new Date();
    sended = 1;
  }
}

function calibration_buttom(){
    if (sended == 0){
        document.getElementById("status_panel").innerHTML  = "En progreso ...";
        return message(2, -2); // -2 es para enviar el comando auxiliar al chassis
    }
  }