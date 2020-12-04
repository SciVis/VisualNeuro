var filePath = window.location.href;
filePath = filePath.replace("file:///", "");
filePath = filePath.replace("saveDialog.html", "");
filePath += "SavedFiles/";
$("#saveLocation").html(filePath);

function savePatients() {

  if($('input[type=checkbox]:checked').length == 0) {

    $("#saveFeedback").html("Please select at least one patient group to save!").css("color", "#f44242");

  } else if($("#fileName").val() == "") {

      $("#saveFeedback").html("Please enter a filename!").css("color", "#f44242");

  } else {
    $("#saveFile").prop("disabled", true);
    $("#fileName").prop("disabled", true);



    if($('#checkGroupA:checked').length == 1) {
      var fileNameA = filePath + $("#fileName").val() + "_groupA.csv";
      console.log(fileNameA)
      localStorage.setItem("patientFileNameA", fileNameA);
    }
    if($('#checkGroupB:checked').length == 1) {
      var fileNameB = filePath + $("#fileName").val() + "_groupB.csv";
      console.log(fileNameB)
      localStorage.setItem("patientFileNameB", fileNameB);
    }

    $("#saveFeedback").html("Your file(s) has now been saved!").css("color", "#87ff93");

      //window.location.href ger:
      //file:///C:/Users/albin/Documents/Github/research-dev/modules/visualneuro/data/workspaces/html/saveDialog.html
  }
}
