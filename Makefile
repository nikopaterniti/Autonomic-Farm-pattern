# The one optimezed one
prod:
	g++ -O3 -std=c++17 main.cpp -lpthread -o farm.out

# With the debug flag 
build:
	g++ -g --debug -std=c++17 main.cpp -lpthread -o farm.out

ff:
	g++ -g --debug -std=c++17 -I./fastflow -std=c++17 main-ff.cpp -lpthread -o farm-ff.out

prodff:
	g++ -O3 -std=c++17 -I./fastflow main-ff.cpp -lpthread -o farm-ff.out