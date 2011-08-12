print(require);
print(require.path);
try {
	foo = require('foo');
} catch(error) {
	print(error);
}
