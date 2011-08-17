print("require: " + require);
print("require.paths: " + require.paths);
print("require.main: " + require.main);
print("require.main.id: " + require.main.id);
print("require.main.uri: " + require.main.uri);
print("module: " + module);
print("module.id: " + module.id);
print("module.uri: " + module.uri);
print("exports: " + exports);
try {
	foo = require('foo');
	foo.sayHi();
} catch(error) {
	print(error);
}
