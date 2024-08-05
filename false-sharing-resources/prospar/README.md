#### Using gem5-resources for false sharing work
- using tag v20.0.0.3
- Every benchmark setup uses m5 library
- Update the \*.json file under disk-image folder to point to m5(: <path\_to\_gem5>/util/build/x86/out/m5)


#### Cmd options supported in GAPBS
 - `-h` print help message
 - `-f <file>` load the graph form <file>
 - `-s` symmetrize input edge list (false)
 - `-g <scale>` generate a graph with 2^scale node.
 - `-u <scale>` generate a uniform random graph with 2^scale node
 - `-k <degree>` average degree for synthetic graph (16)
 - `-m ` reduce memory usage during graph building (false)
 - `-a` output the analysis of last run (false)
 - `-n num` perform num runs (16)
 - `-r <node>` start from node r(rand)
 -  `-v` verify the output of each run (false)

#### md5sum for different images:
| Image  | Md5sum  |
|--------|---------|
| custom | `415c5a2f64572db356377a0ad8c428d7`|
| spec   | `d079b5d17c49f4b1a883142ac297dfc9`|
| parsec | `7bbdf6ecbb9c9d449392df17a5bf4540`|
| npb    | `6402635796e2040d5d4d651dfb826a0f`|
| gapbs  | `40079949a1ecc0e23b0f6fd11b6a3345`|
