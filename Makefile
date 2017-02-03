#This is a proxy to call boostbuild from travis-ci

all: cadmium test


cadmium: 
	

test: cadmium
	b2

clean:
	b2 clean
