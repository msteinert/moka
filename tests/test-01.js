var locale = require('locale');
locale.setlocale(locale.LC_ALL, '');

var io = require('io');
var b = new io.Buffer([102, 111, 111, 98, 97, 114])
print(b);

var cd1 = new io.Iconv('UCS-4');
var c = cd1.convert(b);
print(cd1 + ": " + c);

var cd2 = new io.Iconv('UTF-8', 'UCS-4');
var d = cd2.convert(c);
print(cd2 + ": " + d);

print(new io.Buffer('\u00bd + \u00bc = \u00be'));
