var locale = require('locale');
locale.setlocale(locale.LC_ALL, '');

var io = require('io');
var b = new io.Buffer([102, 111, 111, 98, 97, 114])

var cd1 = new io.Iconv('ucs-4', 'utf-8');
print(cd1);
var c = cd1.convert(b);

var cd2 = new io.Iconv('utf-8', 'ucs-4');
print(cd2 + ": " + cd2.convert(c));
