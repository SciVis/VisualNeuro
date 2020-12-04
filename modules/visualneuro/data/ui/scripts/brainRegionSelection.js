// Helper functions
function isLetter(c) {
  return c.toLowerCase() != c.toUpperCase();
}
function isSpecialCharacter(c) {
  return /^[a-zA-Z0-9]*$/.test(c) == false;
}

// Format a string to create an id in the same way Inviwo does
function formatIdString(id) {
  var formattedId = "";
  // Skip all special characters
  for(var i = 0; i < id.length; i++) {
    if (!isSpecialCharacter(id[i]))
      formattedId = formattedId + id[i];
  }
  return formattedId;
}


function setupBrainRegionTable() {
	

    //define some sample data
 var tabledata = [];
  
  var atlasRegions = JSON.parse(localStorage.getItem("atlasRegions"));
  if (atlasRegions === null || atlasRegions.length == 0) {
    return;
  }
  for (var i = 0; i < atlasRegions.length - 1; i++) {
    var checkboxId = String(i+1) + "_" + formatIdString(atlasRegions[i].Region);
    tabledata.push({propId : checkboxId, id: i+1, enabled: false, region: atlasRegions[i].Region});
    (function(id, i) {  // get a local copy of the current value
      inviwo.getProperty('VolumeAtlas.selectedVolumeRegion.' + id, function(prop) {
        tabledata[i].enabled = prop.value;
      });
    })(checkboxId, i);
  }

 //create Tabulator on DOM element with id "region-table"
var table = new Tabulator("#region-table", {
  height:300, // set height of table (in CSS or here), this enables the Virtual DOM and improves render speed dramatically (can be any valid css height value)
  data:tabledata, //assign data to table
  selectable:true, //make rows selectable
  layout:"fitDataFill",
  columns:[ //Define Table Columns
    {title:"Id", field:"id", headerFilter:true, headerFilterPlaceholder:"Search ID"},
    {title:"Id", field:"propId", visible:false},
    {title:"Region", field:"region", headerFilter:true, headerFilterPlaceholder:"Search region...", align:"left"}
  ],
  rowSelected:function(row){
    inviwo.setProperty('VolumeAtlas.selectedVolumeRegion.' + row.getData().propId, {value: true});
  },
  rowDeselected:function(row){
    inviwo.setProperty('VolumeAtlas.selectedVolumeRegion.' + row.getData().propId, {value: false});
  }
});
// Synchronize with Inviwo
var rows = table.getRows();
for (var i = 0; i < rows.length; i++) {
  (function(id, i) {  // get a local copy of the current value
    inviwo.getProperty('VolumeAtlas.selectedVolumeRegion.' + id, function(prop) {
      if (prop.value === true) {
        table.selectRow(i+1);
      }
    });
  })(rows[i].getData().propId, i);
}

//deselect row on "deselect" button click
$("#deselectAtlasRegions").click(function(){
  var rows = table.getRows();
  for (var i = 0; i < rows.length; i++) {
    rows[i].deselect();
  }
});

//select row on "select all" button click
$("#selectAtlasRegions").click(function(){
    var rows = table.getRows();
    for (var i = 0; i < rows.length; i++) {
      rows[i].select();
    }
});

}

