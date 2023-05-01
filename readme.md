# pysv-numpy: Interface Python Array with System-Verilog

![env](https://img.shields.io/badge/env-python3-brightgreen)  ![version](https://img.shields.io/badge/version-0.1-blue)  

**pysv-numpy** is a bridge framework that allows running Python functions in System-Verilog with *numpy-svOpenArrayHandle* interplay. Yes! It supports <u>multi-dimension data with arbitrary bit depths</u>. We also optimize the runtime speed by adopting *import-and-cache* instead of *parse-when-call* strategy: The Python scripts are parsed from the plain text only once, and are running through bytecodes until the environment is destructed.

> **Note:** If you do not need to handle the array data between Python and System-Verilog, kindly refer to another amazing project [pysv](https://github.com/Kuree/pysv) developed by [@Kuree](https://github.com/Kuree). But **pysv-numpy** has the totally different design philosophy from **pysv**, and thus should follow a distinctive configuration way (@see also [Advanced Topics](#advanced-topics)).

**Any Suggestions are welcomed! [Contact me via Email.](mailto:yinyi.liu@connect.ust.hk)**

## Change Logs
- [x] May 1, 2023: Extracted as *pysv-numpy* library from FIONA project and ready to release.
- [x] Apr 7, 2023: Reorganized project hierarchy and add docker supports.
- [x] Apr 5, 2023: Completed and tested SystemVerilog-Python arbitrary dimension data communication and co-simulation.


## Quickstarts
1. Install the build environment and [Verilator](https://github.com/verilator/verilator) of which version should be greater than **5.006**;
2. Compile this project through the instructions as follows:

```bash
# clone the repository and build env through docker
git clone https://github.com/LIU-Yinyi/pysv-numpy
cd pysv-numpy/
docker build .

# build the executable in docker container
mkdir build && cd build
cmake .. && make

# run the example program 
./pysv-numpy-example

# shortcuts (building and running, clean build)
cd pysv-numpy/src
make run
make clean
```


## Project Organization
To improve the readability and scalability of the project, the files are intended to organize as follows:

```
pysv-numpy
    |-- readme.md                   
    |-- Dockerfile                  ...... use for building the env for running the project
    |-- verilator/
        |-- Makefile                ...... shortcut to build (args: @run @clean)
        |-- CMakeLists.txt          ...... for project compilation
        |-- main.cpp                ...... main C++ file required by Verilator
        |-- top.sv                  ...... to define the top module in system-verilog (SV)
        |-- requirements.txt        ...... to collect all the python dependencies
        |-- register.hh             ...... to register python functions called by SV-DPI
        |-- mod/                    ...... where to put modules (built-in: @bridge)
        |-- pyfunc/                 ...... where to put python files containing your function calls
            |-- utils/              ...... provide built-in utilities such as BridgeParser

```


## Advanced Topics

### 1. Add customized python functions to the project
Once you want to insert your customized python function into system-verilog DPI, remember to modify the files: 

1. **top.sv**: add `import array_handle("python_file", "python_func", input_array, output_array)`;
2. **register.hh**: append {"python_file", "python_func"} to `pyfilefunc_reg`;
3. **pyfunc/*.py**: put your python files at this directory with `*.py` extension.


### 2. Keep the modification of py functions updated when running simulation
The directory of **pyfunc** will be copied under the **build** when you do `cmake ..`. The program you run will also seek the callable files under `build/pyfunc`.   
Therefore, once you modify the python scripts in **pyfunc**, remember to re-execute `cmake ..` before you run the simulation. 


### 3. How pysv-numpy handles arbitrary-bit data
The data are decomposed into sets of *uint8* in a little-end manner. At the Python-end, our provided **BridgeParser** under `utils.parser` will pack and retrieve the data. For example, in System-Verilog:

```verilog
// supposed it is filled with zeros
bit [17:0] example_reg[2:0][1:0];
import "DPI-C" function void array_handle(..., ..., input bit[17:0] example_reg[2:0][1:0][], ...);
```

In python, 18-bit data are decomposed into 3 sets of *uint8*. The rest of the 6-bit data is used as padding.

```python
# raw data
example_reg = [[[0, 0, 0], [0, 0, 0]], [[0, 0, 0], [0, 0, 0]], [[0, 0, 0], [0, 0, 0]]]

# built-in parser (bitnum_in is auto deduced, not required to explicitly set)
from .utils.parser import BridgePaser
parser = BridgeParser(example_reg, bitnum_in=18, ..., ...) 
parsed_reg = parser.get_in(sign_flag=False) # otherwise sign flag should define manually 
```


## Troubleshoots

### 1. svOpenArrayHandle requires appended square bracket
Due to the definition from SV-DPI docs, the **appended square bracket** pair is required, 
in despite of the zero dimension data, e.g. `bit[N:0] val` should be `bit[N:0] val[]`,
or the compiler will parse it as `svBitVecVal` instead of `svOpenArrayHandle`.

```verilog
// supposed data type:
bit[15:0] val_zero_dim;
// which responds to:
import "DPI-C" function void array_handle(..., ..., input bit[15:0] val_zero_dim[], output ...);

// supposed data type:
bit[15:0] val_multi_dim[3:0];
// which responds to:
import "DPI-C" function void array_handle(..., ..., input bit[15:0] val_multi_dim[3:0][], output ...);
```

**Note:** this rule is applied to both `input` and `output`.


### 2. array_handle() simultaneously handle input and output arrays
The form of `array_handle` is as follows:

```verilog
import "DPI-C" function void array_handle(
        string filename, string funcname, 
        input bit[M:0] array_in[A:0][...][], output bit[N:0] array_out[B:0][...][]);
```

Suppose that you develop a python script of which name is **payload.py**, with two functions called **func1** and **func2** respectively inside.    
It concludes that the arguments:

|**Arguments**|**filename**|**funcname**|
|---|---|---|
|Pair #1|"payload"|"func1"|
|Pair #2|"payload"|"func2"|

Don't forget to register them in `register.hh` before you call them in `top.sv`:

```cpp
const PyFileFuncVec pyfilefunc_reg {
    {"payload", "func1"},
    {"payload", "func2"}
};
```

### 3. account information within docker container
Use the following command to create and access the dev env:

```bash
cd pysv-numpy/
docker build .
docker run --name pysv -it <IMAGE_NAME>
```

In case you encounter the scenarios that require password:

|**username**|**password**|
|---|---|
|root|root|
|dev|dev|
