#!/usr/bin/env Rscript

library(ggplot2)
library(scales)
library(sitools)

data.densehash <- read.csv("densehash_int.dat", head=TRUE, sep=",")
data.dynamic <- read.csv("dynamic_int.dat", head=TRUE, sep=",")
data.standard <- read.csv("standard_int.dat", head=TRUE, sep=",")
data.tommyds <- read.csv("tommyds_int.dat", head=TRUE, sep=",")
data.ulib <- read.csv("ulib_int.dat", head=TRUE, sep=",")

graph <- ggplot(legend = TRUE) + 
  ggtitle('Integer hash table benchmark') +
  theme(plot.title = element_text(size = 10), 
        axis.title.x = element_text(size = 8), axis.title.y = element_text(size = 8),
        axis.text.x = element_text(size = 8), axis.text.y = element_text(size = 8)) + 
  geom_line(data = data.densehash, aes(x = size, y = rate, colour = "google::dense_hash_map")) +
  geom_line(data = data.dynamic, aes(x = size, y = rate, colour = "libdynamic")) +
  geom_line(data = data.standard, aes(x = size, y = rate, colour = "std::unordered_map")) +
  geom_line(data = data.tommyds, aes(x = size, y = rate, colour = "tommy_hashdyn")) +
  geom_line(data = data.ulib, aes(x = size, y = rate, colour = "ulib")) +
  scale_y_continuous(labels = comma) +
  scale_x_continuous(labels = comma) +
  scale_colour_manual("",
                      values = c("#E69F00", "#56B4E9", "#D55E00", "#009E73", "#0072B2"))
ggsave(graph, file = "hash-table-int-benchmark.pdf", width = 10, height = 5)
