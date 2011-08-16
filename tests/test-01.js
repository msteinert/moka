print("require: " + require);
print("require.paths: " + require.paths);
print("require.main: " + require.main);
print("module: " + module);
print("module.id: " + module.id);
print("module.uri: " + module.uri);
print("exports: " + exports);
try {
	foo = require('foo');
} catch(error) {
	print(error);
}
