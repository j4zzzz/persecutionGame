test: main.o
	g++ -o test Controller2.o -Lsrc/lib -lsfml-graphics -lsfml-window -lsfml-system
main.o: main.cpp
	g++ -c Controller2.cpp -Isrc/include