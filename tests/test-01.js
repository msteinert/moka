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

var e = new io.Buffer(256);
var f = io.FileStream('bar.txt', 'r+');
print(f.read(e));
print(e);
f.seek(0);
print(f.read(e));
print(e);
f.close();

try {
	var foo = new io.FileStream(100);
} catch (error) {
	print(error);
}

var stdout = new io.FileStream(1);
stdout.write('Hello, World!\n');

var stdin = new io.FileStream(0);
while (true) {
	stdout.write('>>> ');
	var msg = new String;
	while ('\n' != (c = stdin.read(1)))
		msg += c;
	if (msg.length) {
		stdout.write(msg + '\n');
	}
}
