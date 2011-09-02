var io = require('io');

var a = new ArrayBuffer(10);
print(a.byteLength);

var b = a.slice(9, 10);
print(b.byteLength);

//var c = new Int8Array(10);
