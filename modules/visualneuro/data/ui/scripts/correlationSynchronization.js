

function groupAProgressChanged(progress) {
    let percentageProgress = Math.trunc(progress*100);
    $('#groupAProgress').css("width", percentageProgress + "%")
        .attr("aria-valuenow", percentageProgress)
        .text(percentageProgress + "% Complete");
}

function groupAProgressChangedVisibleChanged(visible) {
    if (visible) {  
      $('#groupAProgressDiv').show();
    } else  {
      $('#groupAProgressDiv').hide();
    }
}
function groupBProgressChanged(progress) {
    let percentageProgress = Math.trunc(progress*100);
    $('#groupBProgress').css("width", percentageProgress + "%")
        .attr("aria-valuenow", percentageProgress)
        .text(percentageProgress + "% Complete");
}

function groupBProgressChangedVisibleChanged(visible) {
    if (visible) {  
      $('#groupBProgressDiv').show();
    } else  {
      $('#groupBProgressDiv').hide();
    }
}



$(document).ready(function() {
	
// Update region correlation plot
if (localStorage.getItem("regionCorrelations")) {
    plotErrorBars(JSON.parse(localStorage.getItem("regionCorrelations")));
}

window.addEventListener('storage', function(e) {  
  if (e.key === 'regionCorrelations') {
     plotErrorBars(JSON.parse(e.newValue));
  }
});

  $('#groupAProgressDiv').hide();
  $('#groupBProgressDiv').hide();
  inviwo.subscribeProcessorProgress("A", groupAProgressChanged, groupAProgressChangedVisibleChanged);
  inviwo.subscribeProcessorProgress("B", groupBProgressChanged, groupBProgressChangedVisibleChanged);


});
window.addEventListener('unload', function(event) {
  inviwo.unsubscribeProcessorProgress("A");
  inviwo.unsubscribeProcessorProgress("B");
});
