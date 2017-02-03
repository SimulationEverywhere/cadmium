#This is a proxy to call boostbuild from travis-ci

all: cadmium test


cadmium: 
	

test: cadmium
	bjam

clean:
	bjam clean
