# Dominance frontiers
### About
This program can help you to generate such graphes:
1) CFG graph
2) Dominance tree graph
3) Dominance join graph
4) Dominance frontier graph
## Requirements
**cmake** version must be `3.15` or higher  
**gcc** version must be `13.1` or higher  
**graphviz** tool should be installed  
**display** command should be installed  
## How to build
```bash
git clone git@github.com:VictorBerbenets/dominance-frontiers.git
cd dominance-frontiers/
cmake -S ./ -B build/ -DCMAKE_BUILD_TYPE=Release .
cd build/
cmake --build .
```
## To Run the program do
```bash
./dom-frontiers [command] [options] ...
```
The program will be waiting for command using `-g=` flag:
```bash
--GFG generation:
-g=cfg-txt - generate txt graph format (Vertex1 --> Vertex2)
-g=cfg-dot
-g=cfg-png
-g=cfg     - generate all formats above
--Dominance tree graph generation:
-g=dom-tree-dot
-g=dom-tree-png
-g=dom-tree     - generate all formats above
--Dominance join graph generation:
-g=join-graph-dot
-g=join-graph-png
-g=join-graph     - generate all formats above
--Dominance frontier graph generation:
-g=dom-frontier-dot
-g=dom-frontier-png
-g=dom-frontier     - generate all formats above
```
## Available options:
```bash
--path=<>       - path to create files.  
--graph-name=<> - set graph name.  
--num-nodes=<>  - set number of nodes.  
--num-edges=<>  - set the limit on the number of node edges.  
--node-color=<> - set node color (lightblue is default).  
--edge-color=<> - set edge color (red is default).  
--node-shape=<> - set node shape (square is default).  
  Check `graphviz.org`for more.  
--edge-shape=<> - set edge shape (vee is default).  
  Check `graphviz.org` for more.  
--file-name=<>  - set name for generated file(s) (name graph setted as default).  
--node-name=<>  - set name for nodes. BB setted as default.  
Note: you can use RGB format for color option (e.g. --node-color=#ffffff).
```
### Help option (run with -h, -help):
```bash
   ./dom-frontiers -h 
```
