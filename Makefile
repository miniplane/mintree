simpletest: simpletest.C BpTree.h
	g++ -g -Wall -Wextra -pg -DETYPE=std::string -DCONTAINERTYPE=BpTree simpletest.C -o simpletest

run: simpletest
	./simpletest

debug: simpletest
	gdb ./simpletest

grind: simpletest
	valgrind --track-origins=yes ./simpletest
