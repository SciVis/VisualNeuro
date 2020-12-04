function plotErrorBars(data) {
	
	var width = 0.1*document.body.clientWidth;
	var height = 0.93*document.body.clientHeight;

	//delete data.Parameter['Subject ID'];
	//console.log(JSON.stringify(data))
    var spec = {
        "$schema": "https://vega.github.io/schema/vega-lite/v3.json",
        "title": "Spearman correlation between the selected brain regions and all clinical parameters",
        "width": width,
        "height": height,
      "data": {
        "values": data
      },
	  "config": {
	  "axis": {
			"labelFontSize": 14,
			"titleFontSize": 14
		}
	  },
	  "transform": [
	    {
	      "calculate": "0",
	      "as": "zero"
	    }
	  ],
	  "facet": {
	    "field": "Parameter",
	    "type": "nominal",
	    "sort": {
	      "field": "Parameter"
	    },
	    "spacing": 1,
	    "columns": 2
	  },
	  "spec": {
	    "layer": [
	      {
	        "mark": "rule",
	        "encoding": {
	          "y": {
	            "field": "zero",
	            "type": "quantitative"
	          }
	        }
	      },
	      {
	        "mark": {
	          "type": "errorbar",
	          "ticks": false
	        },
	        "encoding": {
	          "y": {
	            "field": "Max_correlation",
	            "type": "quantitative",
	            "scale": {
	              "zero": false,
	              "domain": [
	                -1.0,
	                1.0
	              ]
	            },
	            "title": "Spearman correlation"
	          },
	          "y2": {
	            "field": "Min_correlation"
	          },
	          "x": {
	            "field": "Processor",
	            "type": "nominal",
	            "title": "",
	            "axis": {
	              "labelAngle": 0,
	              "labels": true
	            }
	          }
	        }
	      },
	      {
	        "mark": {
	          "type": "bar"
	        },
	        "encoding": {
	          "y": {
	            "field": "First_quartile",
	            "type": "quantitative"
	          },
	          "y2": {
	            "field": "Third_quartile"
	          },
	          "x": {
	            "field": "Processor",
	            "type": "nominal"
	          },
	          "color": {
	            "field": "Processor",
	            "type": "nominal",
	            "title": "Group",
	            "legend": { "orient": "left" }
	          }
	        }
	      },
	      {
	        "mark": {
	          "type": "tick",
	          "thickness": 1,
	          "color": "#edb100",
	          "size": 20,
	          "opacity": 1
	        },
	        "encoding": {
	          "y": {
	            "field": "Median_correlation",
	            "type": "quantitative"
	          },
	          "x": {
	            "field": "Processor",
	            "type": "nominal"
	          }
	        }
	      }
	    ]
	  }
  }

  vegaEmbed('#view', spec, {'actions':false}).then(function(result) {
    // Access the Vega view instance (https://vega.github.io/vega/docs/api/view/) as result.view
  }).catch(console.error);

}