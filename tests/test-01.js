var assert = require('assert');
var binary = require('binary');
var a = new binary.ByteString([0, 0, 0, 102, 0, 0, 0, 111, 0, 0, 0, 111,
		0, 0, 0, 98, 0, 0, 0, 97, 0, 0, 0, 114]);
print(a.decodeToString('UCS-4'));
