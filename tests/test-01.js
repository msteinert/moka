print(module.__path__[0]);
print(module.__path__[1]);
print(module.__path__[2]);
module.__path__[10] = 'foobar';
print(module.__path__[9]);
print(module.__path__[10]);
var a = new Array(1, 2, 3);
print(a);
