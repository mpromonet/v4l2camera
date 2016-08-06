(function (app) {
    app.AppComponent = ng.core
        .Component({
            selector: 'controls',
            templateUrl: 'controls.html'
        })
        .Class({
            constructor: [ng.http.Http, function (http) {
	      var bind = this;
              http.get('/controls').subscribe(function(data) {
                bind.controlList = data.json();
              });
            }],
        });
})(window.app || (window.app = {}));
