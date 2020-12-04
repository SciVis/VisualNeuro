var dialogWindow;

$(document).ready(function() {
  document.getElementById("correlationsDialog").onclick = function() {
    if(dialogWindow)
      dialogWindow.close();
    dialogWindow = window.open("correlationsDialog.html", "", "width=" + 0.8*document.body.clientWidth + ", height=" + 550 +"px" );
  };
});
