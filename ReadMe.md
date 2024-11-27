# Process Mapping Analyzer

A tool to evaluate partitions from Process Mapping algorithms.

### Formal Problem Description

- Given a graph $G(V, E)$ with vertex set $V$, edge set $E$, vertex weights $c : V \to \mathbb{R}$ and edge weights $\omega : E \to \mathbb{R}$
  - This graph describes the tasks of an application, where vertex weights represent the cost of a task and edge weights represent the amount of communication between two tasks
- Given a hierarchy $H = a_1 : \ldots : a_\ell$ and distances $D = d_1 : \ldots : d_\ell$
  - This is the hierarchy of a supercomputer. It describes that each processor has $a_1$ PEs (processing elements), each node has $a_2$ processors, each rack has $a_3$ nodes and so forth.
  - The distances denote the communication factor between different cores. Two cores on the same processor have distance $d_1$, two cores in the same node, but on different processors have distance $d_2$, and so forth.
- The graph can be modeled as a communication matrix $C \in \mathbb{R}^{n \times n}$, while the hierarchy and distance can be modeled as the hardware topology matrix $D \in \mathbb{R}^{k \times k}$, with $k = \prod_{i=1}^\ell a_i$.
- The general process mapping problem asks for a mapping $\Pi : [n] \to [k]$ that minimizes
  $$J(C, D, \Pi) = \sum_{i, j} C_{ij}D_{\Pi(i)\Pi(j)}$$
- The mapping additionally has to fulfill the balancing constraint $c(V_i) \leq \lceil (1 + \epsilon) \frac{c(V)}{k} \rceil$ for each of the $k$ partitions.

## This Tool
This tool serves as a basis to evaluate a mapping $\Pi$, while not only calculating $J(C, D, \Pi)$ but also other interesting statistics.

## Usage

Use `build.sh` to build the project, the binary `processmappinganalyzer` will be available in the `build` folder.

Use
``
./processmappinganalyzer [graph_path] [partition_path] [hierachy] [distance] [epsilon] [out_path]
``
to start the tool.
- `[graph_path]` should be the path to a graph in Metis format
- `[partition_path]` should be the path to a file holding the mapping. It should have $n$ lines and each line contains an integer in $k$.
- `[hierarchy]` in the format $a_1:a_2:\ldots:a_\ell$ (no whitespace)
- `[distance]` in the format $d_1:d_2:\ldots:d_\ell$ (no whitespace)
- `[epsilon]` as a double, for example `0.03` for an imbalance of $3\%$
- `[out_path]` should be the file that stores the statistics. The format will be JSON.

## Bugs, Questions, Comments and Ideas

If any bugs arise, questions occur, comments want to be shared, or ideas discussed, please do not hesitate to contact the current repository owner (henning.woydt@informatik.uni-heidelberg.de) or leave a GitHub Issue or Discussion. Thanks!