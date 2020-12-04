function sendCefQuery(elementId, elementValue) {
  window.cefQuery({
    request: JSON.stringify({"id":elementId, "value": elementValue}),
    //request: '{"id":"'+ elementId + value + '"}',
    onSuccess: function(response) {},
    onFailure: function(error_code, error_message) {}
  });
}

function setupSliderSynchronization(elementId, path, delayedSynchronization = false, callback) {
  if(delayedSynchronization) {
	inviwo.subscribe(path, callback);
  	$("#" + elementId).data("ionRangeSlider").update({onFinish: function(data) {
		  inviwo.setProperty(path, {value: Number(data.from)});
		}
    });
  } else {
	$("#" + elementId).data("ionRangeSlider").update({
		onChange: function(data) {
		  inviwo.setProperty(path, {value: Number(data.from)});
		}
	});
  }
}

function setupCheckboxSynchronization(elementId, path, callback) {
  var element = document.getElementById(elementId);
  inviwo.subscribe(path, callback);
  element.onchange = function() {
    inviwo.setProperty(path, {value: this.checked});
  }
}

function setupButtonSynchronization(elementId, path) {
  var element = document.getElementById(elementId);
  element.onclick = function() {
    inviwo.setProperty(path, {pressButton: true});
  }
}

function setupTextfieldSynchronization(elementId, path, callback) {
  var element = document.getElementById(elementId);
  inviwo.subscribe(path, callback);
  element.onchange = function() {
    inviwo.setProperty(path, {value: this.value});
  }
}

function setupSelectionSynchronization(elementId, path, callback) {
  var element = document.getElementById(elementId);
  inviwo.subscribe(path, callback);
  element.onchange = function() {
	var selectedIndex = $(this).children("option:selected").index();
    inviwo.setProperty(path, {selectedIndex: Number(selectedIndex)});
  }
}

function setupSelectDatatypeSynchronization(elementId, listPath, boolPath) {
  var element = document.getElementById(elementId);
  var sliderId = elementId + "Slider";
  var boolId = elementId + "Bool";
  var newElementSliderHTML = '<input type="range" min="0" max="99999" value="0" step="1" class="noShow" id="' + sliderId + '">';
  var newElementBoolHTML = '<input type="checkbox" value="0" class="noShow" id="' + boolId + '">';
  $("body").append(newElementSliderHTML);
  $("body").append(newElementBoolHTML);
  var selectionSlider = document.getElementById(sliderId);
  var selectionBool = document.getElementById(boolId);
  
  inviwo.subscribe(selectionSlider.id, listPath);
  inviwo.subscribe(selectionBool.id, boolPath);
  element.onclick = function() {
    for (var i = 0; i < element.children.length; i++) {
      for (var j = 0; j < element.children[i].classList.length; j++) {
        if (element.children[i].classList[j] == "selectedElement") {
          selectionSlider.value = Math.max(0, Math.min(i, 1));
          selectionBool.checked = selectionSlider.value == 1;
          //console.log(selectionBool.checked);
          console.log(selectionSlider.value);
		  
		  inviwo.setProperty(listPath, {value: Number(selectionSlider.value)});
		  inviwo.setProperty(boolPath, {value: selectionBool.checked});
          //sendCefQuery(selectionSlider.id, Number(selectionSlider.value));
          //sendCefQuery(selectionBool.id, selectionBool.checked);
          return;
        }
      }
    }
  };
}

// Updates which resetFiltering-button that is visible. Also updates the buttons describing label.
function updateResetButton() {
  var group = $("#selectGroup").serializeArray()[0]["value"].substring(5,6);
  if(group == "A") {
    $("#resetFilteringA").removeClass("noShow");
    $("#resetFilteringB").addClass("noShow");
  } else {
    $("#resetFilteringA").addClass("noShow");
    $("#resetFilteringB").removeClass("noShow");
  }
  $("#patientGroupLabel").html(group);
}
