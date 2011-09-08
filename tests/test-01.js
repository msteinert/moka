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

var x = new ArrayBuffer(16);

var y = new FloatArray(x);
print('y.length: ' + y.length);
for (var i = 0; i < y.length; ++i) {
	y[i] = i * Math.PI;
	print(y[i]);
}
print();

var z = new Uint32Array(x);
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
