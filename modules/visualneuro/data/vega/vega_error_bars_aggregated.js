function onInviwoDataChanged(data, axes) {
	var width = 0.1*document.body.clientWidth;
	var height = 0.93*document.body.clientHeight;

    var spec = {
        "$schema": "https://vega.github.io/schema/vega-lite/v3.json",
        "title": "Correlations between the selected brain regions and all clinical parameters",
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
	    }
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
	            "title": "Correlation"
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
	            "title": "Group"
	          }
	        }
	      },
	      {
	        "mark": {
	          "type": "tick",
	          "thickness": 1,
	          "color": "#FFFFFF",
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