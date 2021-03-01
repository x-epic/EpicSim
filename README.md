
# Getting Started with EpicSim

EpicSim is a Verilog compiler. It is suitable for use as a simulator and recommended to run under Red Hat and CentOS. Instructions in this document are generally applicable to all environments.  

The project is developed based on icarus iverilog under LGPL. Special thanks to Stephen Williams (steve@icarus.com).  

## 1. Install From Source

If you are starting from the source, the build process is designed to be as simple as practical. Someone basically familiar with the target system and C/C++ compilation should be able to build the source distribution with little effort. Some actual programming skills are not required, but helpful in case of problems.  

### 1.1 Compile Time Prerequisites

You need the following softwares to compile EpicSim from source on a UNIX-like system:

- CMake
- GNU Make
- ISO C++ Compiler
- bison and flex
- gperf 3.0 or later
- readline 4.2 or later
- termcap
- bash

### 1.2 Compilation

Unpack the tar-ball and compile the source with the commands:
```bash
  cd EpicSim
  mkdir build
  cd build
  cmake ..
  make install
```
After the installtion, set the environment variables with the commands:
```bash
  cd EpicSim
  #bash
  export PATH="target_installation_path/EpicSim/install/bin:$PATH"
  #cshell
  setenv PATH target_installation_path/EpicSim/install/bin:$PATH"
```

## 2. Hello, World!

As a user, the first thing you want to do is probably learning how to compile and execute even the most trivial design. For simulation, we use the following design as an example:    
```verilog
  module main;
  initial
    begin
      $display("Hello, World");
      $finish ;
    end
  endmodule
```
1. Arrange for the above program to be in a text file, `hello.vl` using a text editor or copy the `hello.vl` file from the EpicSim examples directory.
2. Compile the program with the `epicsim` command:
    ```bash
    % epicsim hello.vl
    ```
3. The results of this compile are placed into the `epicsim-run` file, and it is excuted:
    ```bash
    Hello, World
    ```
The compiled program can also be executed by two steps:  
```bash
% epicsim-driver hello.vl -o epicsim-run 
% epicsim-vvp ./epicsim-run  
```
```bash
Hello, World
```
And there it is, the program has been executed. So what happened? The first step, the `epicsim-driver` command, read and interpreted the source file, then generated a compiled result. The compiled form may be selected by command line switches, but the default form is `vvp`, which is actually run by the `epicsim-vvp` command.   

The `epicsim-driver` and `epicsim-vvp` commands are the only commands that users use to invoke EpicSim. What the compiler actually does is controlled by command line switches. In our little example, we asked the compiler to compile the source program to the default `vvp` form, which is in turn executed by the `epicsim-vvp` program.    

## 3. How EpicSim Works

This tool includes a parser which reads in Verilog (plus extensions) and generates an internal netlist. The netlist is passed to various processing steps that transform the design to more optimal or practical forms, then is passed to a code generator for final output. The processing steps and the code generator are selected by command line switches. 

### 3.1 Preprocessing

The preprocessing is done by a separate program, `ivlpp`. This program implements the `include` and `define` directives producing output that is equivalent but without the directives. The output is a single file with line number directives, so that the actual compiler only sees a single input file. See `ivlpp/ivlpp.txt` for details.  

### 3.2 Parse

The Verilog compiler starts by parsing the Verilog source file. The output of the parse is a list of Module objects in "`pform`". The `pform` is mostly a direct reflection of the compilation step (see `pform.h`). There may be dangling references, and it is not yet clear which module is the root.  

One can see a human-readable version of the final `pform` by using the `-P <path>` flag to the `ivl` subcommand. This will cause `ivl` to dump the `pform` into the file named `<path>`. (Note that this is not normally done, unless debugging the `ivl` subcommand.)  

### 3.3 Elaboration

The elaboration takes the `pform` and generates a netlist. The driver selects (by user request or lucky guess) the root module to elaborate, resolves references and expands the instantiations to form the design netlist (see `netlist.txt`). Final semantic checks and some simple optimizations are performed during elaboration. The netlist includes all the behavioural descriptions, as well as gates and wires.  

The `elaborate()` function performs the elaboration.  

One can see a human-readable version of the final, elaborated and optimized netlist by using the `-N <path>` flag to the compiler. If elaboration succeeds, the final netlist (i.e., after optimizations but before code generation) will be dumped into the file named `<path>`.  

The elaboration is performed in two steps:  
- Scopes and parameters elaboration  
- Structural and behavioural elaboration.  

#### 3.3.1 Scope Elaboration

The scope elaboration traverses the `pform` looking for scopes and parameters. A NetScope object tree is built and placed in the Design object, with the root module represented by the root NetScope object. The `elab_scope.cc` file contains most of the code for handling this step.  

The tail of the elaborate_scope behaviour (after the pform is traversed) includes a scan of the NetScope object tree to locate the defparam assignments that were collected during scope elaboration. This is when the defparam overrides are applied to the parameters.  

#### 3.3.2 Netlist Elaboration

After the scopes and parameters are generated and the NetScope project tree is fully formed, the elaboration traverses the `pform` again to generate the structural and behavioural netlist. The parameters are also elaborated and evaluated, so all the constants of code generation are now known locally and the netlist can be generated by simply passing through the `pform`.  

### 3.4 Optimization

The optimization is a collection of processing steps that perform optimizations that do not depend on the target technology. Examples of some useful transformations are as follows:  
- Eliminate null effect circuitry 
- Combinational reduction  
- Constant propagation.  

The actual functions performed are specified on the `ivl` command line by the `-F` flags.  

### 3.5 Code Generation

The code generation takes the design netlist and uses it to drive the code generator (see `target.h`). This may require transforming the design to suit the technology.  

The `emit()` method of the Design class performs this step. It runs through the design elements, calling target functions as the need arises to generate actual output.  

The user selects the target code generator with the `-t` flag on the command line.  

## 4 Unsupported Constructs

- Specify blocks are parsed but ignored in general.  
- Trireg is not supported. `tri0` and `tri1` are supported.  
- Tran primitives, i.e. `tran`, `tranif1`, `tranif0`, `rtran`, `rtranif1`, and `rtranif0` are not supported.  
- Event controls inside non-blocking assignments are not supported, for example:  
    ```verilog
    a <= @(posedge clk) b;
    ```

## 5 Commonly Used Flags/Options

`-o <output file>`: Output the compilation result file.  
`-f <command file>`: Read in files with options.  
`-D <macro=value>`: Assign a value to the macro.  
`-s <root module>`: Set the root module to elaborate.  
`T min|typ|max`: Select the timings (minimum, typical, and maximum) to use for simulation. The default set is typical.  

