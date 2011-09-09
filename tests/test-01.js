'use strict';

/*
var c = new DoubleArray(10);
for (var i = 0; i < c.length; ++i) {
	c[i] = i * Math.PI;
	print(c[i]);
	c.set(i, 2 * c[i]);
	print(c.get(i));
}

var d = new Int32Array([1, 2, 3, 4, 5]);
for (var i = 0; i < d.length; ++i) {
	print(d[i]);
}

var e = new Int16Array(d);
for (var i = 0; i < e.length; ++i) {
	print(e[i]);
}
*/

function toArray(object) {
	if (!object.length) {
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
print('foo: ' + Float32Array.BYTES_PER_ELEMENT);
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
