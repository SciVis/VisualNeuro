var GuidedTour = function (steps, options) {
  $(document)
    .on('click', '[data-toggle=popover]', function () {
      var $context = $($(this).data('target')).popover('show');
      var scrollTop = $context.data('bs.popover').tip.offsetTop - $(window).height() / 2;
    $('html, body').clearQueue().animate({scrollTop: Math.max(scrollTop, 0)}, 'fast');
      return false;
    })
    .on('click', '[data-dismiss="popover"]', function () {
      $(this).closest('.popover').data('bs.popover').hide();
      return false;
    });

  return {
    start: function () {
      var toursteps = [];
      var defaults = {
        html: true,
        placement: 'auto top',
        container: 'body',
        trigger: 'manual'
      };
      var opts = $.extend(defaults, options);
      $(steps).each(function (i, step) {
        if (step.target) {
          var $target = $(step.target);
          if (!$target.length) {
            console.warn('Target not found', $target);
            return;
          }
          if (step.content instanceof $) step.content = step.content.html();
          var content = step.content;
          step.content = function () {
            var out = content;
            out += '<div class="mm_actions d-flex flex-row-reverse">';
            if (i + 1 < steps.length) {
              out += '<button type="button" class="btn btn-primary pull-right ml-2" autofocus data-dismiss="popover" data-toggle="popover" data-target="'+steps[i + 1].target+'">Next</button>';
            }
            out += '<button type="button" class="btn btn-default pull-right" data-dismiss="popover">Close</button>';
            out += '</div>';
            return out;
          }
          toursteps.push($target.popover($.extend(opts, step)));
        }
      });
      
      if (toursteps[0]) toursteps[0].popover('show');
    }
  }
};

function startDataImportTour() {
  $('#dataImportTourModal').modal();
  

  $('#dataImportTourModal #start').click(function () {
    var tour = GuidedTour([
      {
        target: '#tour-step1',
        title: 'Data import',
        placement: 'bottom',
        content: $('#tour-step1-info'),
      },
      {
        target: '#tour-step2',
        title: 'Type of data',
        placement: 'bottom',
        content: $('#tour-step2-info')
      },
      {
        target: '#tour-step3',
        title: 'Clinical data',
        placement: 'bottom',
        content: $('#tour-step3-info')
      },
      {
        target: '.container',
        placement: 'bottom',
        title: 'Great work',
        content: 'That should be enough to get your data into Visual Neuro. Press this button when you are ready to analyze your data.'
      },
    ]);

    tour.start();
  });
}

function startAnalyzeTour() {
  $('#analyzeTourModal').modal();
  

  $('#analyzeTourModal #start').click(function () {
    var tour = GuidedTour([
      {
        target: '#tour-pcp-1',
        title: 'Select subject group',
        placement: 'top',
        content: $('#tour-pcp-info'),
      },
      {
        target: '#tour-group-selection',
        title: 'Group selection',
        placement: 'top',
        content: $('#tour-group-selection-info')
      },
      {
        target: '#activeVolume',
        title: 'Compare groups',
        placement: 'top',
        content: $('#tour-ttest-info')
      },
      {
        target: '#tour-pcp-2',
        title: 'Within group parameter-brain correlation',
        placement: 'top',
        content: $('#tour-param-to-brain-info')
      },
      {
        target: '#region-table',
        title: 'Brain region-parameter correlation',
        placement: 'top',
        content: $('#tour-brain-to-param-info')
      },
      {
        target: '#tour-pcp-3',
        placement: 'top',
        title: 'Great work',
        content: '<p>You have selected subject groups, compared groups, and analyzed correlations between parameters and the brain.</p> <p>You have mastered the basics and are ready to go play on your own!</p>'
      },
    ]);

    tour.start();
  });
}
//