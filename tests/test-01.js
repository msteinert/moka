var assert = require('assert');
try {
	//assert.deepEqual([1, 2, 3], [1, 2, 3]);
	/*
	assert.notDeepEqual([1, 2, 3], [3, 2, 1]);
	assert.deepEqual({one: 1, two: 2}, {two: 2, one: 1});
	assert.deepEqual({one: 1, two: 2}, {two: 2});
	*/
	assert.throws(function() {
		throw RangeError;
	}, Error);
} catch(err) {
	print(err + ": " + err.expected + " != " + err.actual);
}
