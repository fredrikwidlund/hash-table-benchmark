VARIANTS	= standard dynamic densehash
DATA		= $(VARIANTS:=.dat)
CXXFLAGS	+=-Wall -O3 -std=c++11 -flto -D_GNU_SOURCE
LDLIBS  	+=-ldynamic -L/usr/local/lib -lfarmhash

.PHONY: clean

hash-table-benchmark.pdf: $(DATA)
	./graph.R

%.dat: %
	echo "\"size\",\"rate\"" > $@
	for i in $$(seq 10000 10000 1000000); do \
		echo $$i,$$(./$^ $$i $$((1000000/$$i)));\
	done >> $@

$(VARIANTS): template.cc
	$(CXX) $(CXXFLAGS) -o $@ template.cc $(LDLIBS) -D$$(echo $@ | tr '[:lower:]' '[:upper:]')

clean:
	rm -f $(VARIANTS) $(DATA)
