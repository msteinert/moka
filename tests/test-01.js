var assert = require('assert');
var test = require('test');
try {
	//assert.deepEqual([1, 2, 3], [1, 2, 3]);
	/*
	assert.notDeepEqual([1, 2, 3], [3, 2, 1]);
	assert.deepEqual({one: 1, two: 2}, {two: 2, one: 1});
	assert.deepEqual({one: 1, two: 2}, {two: 2});
	*/
	assert.throws(function() {
		throw { message: 'foo' };
	}, Error);
} catch(err) {
	print(err.expected + " != " + err.actual);
}
