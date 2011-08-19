binary = require('binary');
try {
	s1 = new binary.ByteString([1, 2, 3]);
	s2 = new binary.ByteString(s1);
	if (s1 instanceof binary.ByteString) {
		print('foo');
	}
	if (s2 instanceof binary.Binary) {
		print('bar');
	}
} catch(error) {
	print(error);
}
