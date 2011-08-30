var io = require('io');
var b = new io.Buffer(11);
print(b.length);
print(b.resize(15));
print(b[14]);
print(b.length);
b[2] = 100;
print(b[2]);
