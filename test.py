import myext

x = myext.X()
y = myext.Y()

y2 = myext.getY()
assert isinstance(y2, myext.Y)

print("\n==== Xstore ====")

s = myext.Xstore(y)
print("Xstore object:", s)
print(s.store)

print("\n==== echoes ====")

myext.echoX(x)
# should get a different message
myext.echoX(y)
myext.echoX(s.store)

print("\n==== XVec ====")

l = myext.XVec()
l.append(x)
l.append(y)
print(l)
