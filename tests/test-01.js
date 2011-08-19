binary = require('binary');
try {
	s = new binary.ByteString([1, 2, 3]);
	t = s.join([100, 101, 102], 200);
	for (var i = 0; i < t.length; ++i) {
		print(t[i]);
	}
} catch(error) {
	print("error: " + error);
}
