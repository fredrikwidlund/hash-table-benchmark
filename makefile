VARIANTS_STR	= standard_str dynamic_str densehash_str ulib_str
VARIANTS_INT	= standard_int dynamic_int densehash_int ulib_int
DATA_STR	= $(VARIANTS_STR:=.dat)
OUT_STR		= $(VARIANTS_STR:=.out)
DATA_INT	= $(VARIANTS_INT:=.dat)
OUT_INT		= $(VARIANTS_INT:=.out)
CXXFLAGS	+=-Wall -O3 -std=gnu++11 -flto -D_GNU_SOURCE -Isupport
LDLIBS  	+=-ldynamic -L/usr/local/lib -lfarmhash

.PHONY: all clean

.SECONDARY: $(OUT_STR) $(OUT_INT)

all: hash-table-str-benchmark.pdf hash-table-int-benchmark.pdf

hash-table-str-benchmark.pdf: $(DATA_STR)
	./graph_str.R

hash-table-int-benchmark.pdf: $(DATA_INT)
	./graph_int.R

%_str.out: %_str
	for i in $$(seq 10000 10000 1000000); do \
		echo $$i $$(./$^ $$i $$((1000000/$$i)));\
	done >> $@

%_int.out: %_int
	for i in $$(seq 10000 10000 1000000); do \
		echo $$i $$(./$^ $$i $$((1000000/$$i)));\
	done >> $@

%.dat: %.out
	echo "\"size\",\"rate\"" > $@
	awk '{print $$1 "," $$3}' < $< >> $@

$(VARIANTS_STR): template_str.cc
	$(CXX) $(CXXFLAGS) support/ulib/hash_func.c -o $@ template_str.cc $(LDLIBS) -D$$(echo $@ | tr '[:lower:]' '[:upper:]')

$(VARIANTS_INT): template_int.cc
	$(CXX) $(CXXFLAGS) support/ulib/hash_func.c -o $@ template_int.cc $(LDLIBS) -D$$(echo $@ | tr '[:lower:]' '[:upper:]')

clean:
	rm -f $(VARIANTS_STR) $(VARIANTS_INT) $(DATA_STR) $(DATA_INT) $(OUT_STR) $(OUT_INT)
