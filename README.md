# ELF

This is a C++ repository for research.
-----------------

Requirements:

   - Visual Studio 2019
   
   - Vmware Workstation 16.0.0
   
   - Ubuntu server 20.04.2 LTS
   
For the usage of visual studio cross-platform, please refer to:

    - https://devblogs.microsoft.com/cppblog/linux-development-with-c-in-visual-studio/
    
-----------------
    
This repository is consisted of four projects which are from:
    
    - https://github.com/BoleynSu/bfl
    
    - https://github.com/steps/Ferrari
    
    - https://github.com/zakimjz/grail
    
    - https://github.com/oeljeklaus-you/MGTag
    
    
They are transformed into visual studio projects and should be loaded and excuted by *.sln (in the root directory).

Modifications have been done on these four methods for applying ELF.

----------------------------

For dataset and query, you need to change their format and put them into /data/{DATASET}_dag_uniq.gra and /{DATASET}/query.txt, respectively. You can also download datasets in our experiments from:

  - https://github.com/ecml-pkdd-paper-596/ELF-datasets
  
Graph format:

*_dag_uniq.gra

The first line must be "graph_for_greach".

For u and all u's successors v, it should be described as:    u: v_1 v_2 ... v_deg+(u)#

Query format:

query.txt

For query Reach(u,v), it should be described as:

u v -1

-1 can be changed into 1 and 0 for reachable and unreachable queries, respectively.
