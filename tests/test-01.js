'use strict';

function toArray(object) {
	if (object.length == undefined) {
		object.length = Object.keys(object).length;
	}
	return Array.prototype.slice.call(object, 0);
}

var x = new ArrayBuffer(16);

var y = new Float32Array(x);
print('y.length: ' + y.length);
for (var i = 0; i < y.length; ++i) {
	y[i] = i * Math.PI;
	print(y[i]);
}
var u = toArray(JSON.parse(JSON.stringify(y)));
var w = new Float32Array(u);
print();

var z = new Uint32Array(w);
print('z.length: ' + z.length);
for (var i = 0; i < z.length; ++i) {
	print(z[i]);
}
print();

var a = z.subarray(-2, 4);
print('a.length: ' + a.length);
for (var i = 0; i < a.length; ++i) {
	print(a[i]);
}
print();
