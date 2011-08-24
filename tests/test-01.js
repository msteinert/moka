var assert = require('assert');
var binary = require('binary');
try {
	var s = new binary.ByteString(
			[0, 0, 0, 102,
			 0, 0, 0, 111,
			 0, 0, 0, 111,
			 0, 0, 0, 98,
			 0, 0, 0, 97,
			 0, 0, 0, 114]);
	assert.strictEqual(s.decodeToString('UCS-4'), 'foobar',
			'decode UCS-4 ByteString');
	assert.equal(s.indexOf(102), 3,
			'check index of byte 102');
	assert.equal(s.indexOf(111, 8), 11,
			'check index of byte 111 at offset 8');
	assert.equal(s.indexOf(new binary.ByteString([0, 0, 111])), 5,
			'check index of ByteString([0, 0, 111])');
	assert.equal(s.indexOf(new binary.ByteString([0, 0, 120])), -1,
			'check index of ByteString([0, 0, 120])');
	assert.equal(s.lastIndexOf(111), 11,
			'check last index of byte 111');
	assert.equal(s.lastIndexOf(new binary.ByteString([0, 0, 111])), 9,
			'check last index of ByteString([0, 0, 111])');
	assert.equal(s.codeAt(3), 102,
			'check value of code at index 3 using codeAt');
	assert.equal(s.get(3).codeAt(0), 102,
			'check value of code at index 3 using get');
	var a = new binary.ByteArray(
			[0, 0, 0, 102,
			 0, 0, 0, 111,
			 0, 0, 0, 111,
			 0, 0, 0, 98,
			 0, 0, 0, 97,
			 0, 0, 0, 114]);
	assert.strictEqual(a.decodeToString('UCS-4'), 'foobar',
			'decode UCS-4 ByteArray');
	assert.equal(a.indexOf(111), 7,
			'check index of byte 111');
	assert.equal(a.indexOf(98, 0, 14), -1,
			'check index of byte 98 between offsets 8 & 14');
} catch (error) {
	if (error instanceof assert.AssertionError) {
		print(error.message ? "FAIL: " + error.message : "FAIL");
		if (error.expected) {
			print('\texpected: ' + error.expected);
		}
		if (error.actual) {
			print('\t  actual: ' + error.actual);
		}
	} else {
		print("FAIL: " + error);
	}
}
