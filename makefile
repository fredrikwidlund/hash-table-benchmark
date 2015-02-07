VARIANTS	= standard dynamic densehash ulib
DATA		= $(VARIANTS:=.dat)
OUT		= $(VARIANTS:=.out)
CXXFLAGS	+=-Wall -O3 -std=gnu++11 -flto -D_GNU_SOURCE -Isupport
LDLIBS  	+=-ldynamic -L/usr/local/lib -lfarmhash

.PHONY: clean

.SECONDARY: $(OUT)

hash-table-benchmark.pdf: $(DATA)
	./graph.R

%.out: %
	for i in $$(seq 10000 10000 1000000); do \
		echo $$i $$(./$^ $$i $$((1000000/$$i)));\
	done >> $@

%.dat: %.out
	echo "\"size\",\"rate\"" > $@
	awk '{print $$1 "," $$3}' < $< >> $@

$(VARIANTS): template.cc
	$(CXX) $(CXXFLAGS) support/ulib/hash_func.c -o $@ template.cc $(LDLIBS) -D$$(echo $@ | tr '[:lower:]' '[:upper:]')

clean:
	rm -f $(VARIANTS) $(DATA) $(OUT)
