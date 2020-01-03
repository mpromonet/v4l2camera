// app
var app = angular.module('App', ['ngResource']);

// service
app.factory('$api', ['$resource', function($resource){ 
	return {
		capabilities : $resource('/api/capabilities', {}, {'query':{isArray:false}}),
		inputs : $resource('/api/inputs'),
		controlList : $resource('/api/controls'),
		control : $resource('/api/control',{},{'update':{method:'POST'}}),				
		formatList : $resource('/api/formats'), 
		format : $resource('/api/format', {}, {'query':{method:'GET', isArray:false} }, {'update':{method:'POST'} } ) ,
		start : $resource('/api/start'),
		stop : $resource('/api/stop')				
	}
}]);	

// tab controller
app.controller('AppController', function($scope, $api) {
	$api.capabilities.query(function(data) { $scope.capabilities = data; } );
	$api.inputs.query(function(data) { $scope.inputs = data; } );
	
	$scope.tabs = [
	  { title:'video'  , url:'video.html'   },
	  { title:'format' , url:'formats.html' },
	  { title:'control', url:'controls.html', disabled:"1", active:"0"},
	]; 
	
});

// video controller
app.controller('AppVidController', function($scope, $api) {
	$api.format.query(function(data) { $scope.format = data; } );
	
	$scope.captureStart= function() {
		$api.start.query();
	}
	$scope.captureStop= function() {
		$api.stop.query();
		var img = document.getElementById('wsimage');
		img.src = '';
	}
	$scope.mjpegStart= function() {
		var img = document.getElementById('mjpegimage');
		img.src = '/jpeg';
	}
	$scope.mjpegStop= function() {
		var img = document.getElementById('mjpegimage');
		img.src = '';
	}
});

// Control controller
app.controller('AppCtrlController', function($scope, $api) {
	$api.controlList.query(function(data) { $scope.controlList = data; } );
	
	$scope.update = function(controlid,controlvalue,idx) {
		var ctrl = {};
		ctrl.id = parseInt(controlid);
		ctrl.value = parseInt(controlvalue); 
		$api.control.update(ctrl,function(data) { $scope.controlList[idx].value = data.value; } );
	};			
});

// Format controller
app.controller('AppFmtController', function($scope, $api, $log) {			
	$api.formatList.query(function(data) { 
		$scope.formatList = data; 
		$api.format.query(function(data) { 
			$scope.currentformat = data; 
			$scope.updateSelectedFormat();
		} );
	} );	
	
	$scope.update= function(format,width,height,fps) {
		if (format) $scope.currentformat.format = format;
		if (width) $scope.currentformat.width = parseInt(width);
		if (height) $scope.currentformat.height = parseInt(height);
		if (fps) $scope.currentformat.fps = parseInt(fps);
		$scope.currentformat.$save(function(data) {
			$scope.updateSelectedFormat();
		});
	}

	$scope.updateSelectedFormat= function(format) {
		$scope.selectedFormat = $scope.formatList.filter(function(item) { return item.format === $scope.currentformat.format; }) [0];
		$scope.selectedFormat.width = $scope.currentformat.width;
		$scope.selectedFormat.height = $scope.currentformat.height;
		$scope.selectedFormat.fps = $scope.currentformat.fps;
	}			
});		
