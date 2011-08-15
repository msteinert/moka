exports.foo = 'foo';
print(exports);
try {
	foo = require('foo');
} catch(error) {
	print(error, ": ", exports);
}
