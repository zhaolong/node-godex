var http = require('http');
var godex = require('./build/Release/godex');

function getTime() {
  function intFormat(num, length) {
	var r = "" + num;
    while (r.length < length) {
      r = "0" + r;
    }
    return r;
  }
  var date = new Date();
  return (intFormat(date.getFullYear(), 4) + '-' + intFormat(date.getMonth() + 1, 2) + '-' + intFormat(date.getDate(), 2) + ' '
  	+ intFormat(date.getHours(), 2) + ':' + intFormat(date.getMinutes(), 2) + ':' + intFormat(date.getSeconds(), 2));
}

function godexCmd(code, text) {
	var code128 = '^Q50,2\n^W79\n^H10\n^P1\n^S4\n^AD\n^C1\n^R0\n~Q+0\n^O0\n^D0\n^E30\n~R200\n^L\nDy2-me-dd\nTh:m:s\nBQ,100,162,2,7,100,0,1,';
	code128 += code;
	var time = 'AZ,104,130,1,1,0,0,' + getTime();
	var cmd = code128 + '\n' + time;
	return cmd;
}

function godexPrint(code, text) {
	godex.openport();
	godex.sendcommand(godexCmd(code));
	godex.ectextout(104, 70, 24, '黑体', text[0]);
	godex.ectextout(104, 100, 24, '黑体', text[1]);
	godex.sendcommand("E");
	godex.closeport();
}

function trytoPrint() {
	var postData = 'key=barcodeserverkey';
	var postOptions = {
	  hostname: 'www.barcodeserver.com',
	  port: 80,
	  path: '/barcodequeue/pop',
	  method: 'POST',
	  headers: {
          'Content-Type': 'application/x-www-form-urlencoded',
          'Content-Length': postData.length
      }
	};
	console.log('Try to print from http://' + postOptions.hostname + postOptions.path);
	var req = http.request(postOptions, function(res) {
	  	console.log('Got status code: ' + res.statusCode);
	  	res.setEncoding('utf8');
	    res.on('data', function (chunk) {
	    	console.log('Got data ' + chunk);
	    	try {
		    	var result = JSON.parse(chunk)['result'];
		    	if (result) {
		    		var barcode = result['code'];
		    		var bartext = result['text'];
		    		console.log('Start to print barcode: ' + barcode + ' bartext: ' + bartext);
		    		godexPrint(barcode, bartext);
		    	}
		    } catch (e) {
			  console.log(e);
			}
  		});
	}).on('error', function(e) {
	  	console.log('Got error: ' + e.message);
	});
	req.write(postData);
	req.end();
}

godex.setup(50, 10, 4, 0, 2, 0);
setInterval(trytoPrint, 1000);