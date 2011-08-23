var assert = require('assert');
assert.error(function() {
		assert.ok(false, "foo");
	}, assert.AssertionError, "bar");
