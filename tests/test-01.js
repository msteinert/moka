var io = require('io');
var f = io.FileStream('bar.txt', 'r+');
var a = new io.Buffer(256);
f.read(a);
print(a);
f.seek(0);
var b = new Array(4);
f.read(b);
print(b);
f.close();
