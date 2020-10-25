
# Getting Started with EpicSim

EpicSim is a Verilog compiler. It is suitable for use as a
simulator. EpicSim is recommended to run under RedHat and Centos.
Instructions are generally applicable to all environments.

Project is developed based on icarus iverilog under LGPL. 
Special thanks to Stephen Williams (steve@icarus.com).



## 1. Install From Source

In this case, see INSTALL_INSTRUCTION.txt that comes with the source.


## 2. Hello, World!

The first thing you want to do as a user is learn how to compile and
execute even the most trivial design. For the purposes of simulation,
we use as our example *the* most trivial simulation:
```verilog
  module main;

  initial
    begin
      $display("Hello, World");
      $finish ;
    end

  endmodule
```
By a text editor (or copy hello.vl from the EpicSim examples
directory) arrange for this program to be in a text file, "hello.vl".
Next, compile this program with a command like this:
```shell
	% epicsim hello.vl
  Hello, World
```
The results of this compile are placed into the file "epicsim-run",
and it is excuted.

The compiled program can also be excuted like so:
```shell
  % epicsim-driver hello.vl -o epicsim-run 
	% epicsim-vvp ./epicsim-run
	Hello, World
```
And there it is, the program has been executed. So what happened? The
first step, the "epicsim-driver" command, read and interpreted the source
file, then generated a compiled result. The compiled form may be
selected by command line switches, but the default form is the VVP
format, which is actually run by the "epicsim-vvp" command.

The "epicsim-driver" and "epicsim-vvp" commands are the only commands
that users use to invoke EpicSim. What the compiler actually does is
controlled by command line switches. In our little example, we asked
the compiler to compile the source program to the default epicsim-vvp form,
which is in turn executed by the epicsim-vvp program.

## 3. How EpicSim Works

This tool includes a parser which reads in Verilog (plus extensions)
and generates an internal netlist. The netlist is passed to various
processing steps that transform the design to more optimal/practical
forms, then is passed to a code generator for final output. The
processing steps and the code generator are selected by command line
switches.

### 3.1 Preprocessing

There is a separate program, ivlpp, that does the preprocessing. This
program implements the `include and `define directives producing
output that is equivalent but without the directives. The output is a
single file with line number directives, so that the actual compiler
only sees a single input file. See ivlpp/ivlpp.txt for details.

### 3.2 Parse

The Verilog compiler starts by parsing the Verilog source file. The
output of the parse is a list of Module objects in "pform". The pform
(see pform.h) is mostly a direct reflection of the compilation
step. There may be dangling references, and it is not yet clear which
module is the root.

One can see a human-readable version of the final pform by using the
``-P <path>'' flag to the ``ivl'' subcommand. This will cause ivl
to dump the pform into the file named <path>. (Note that this is not
normally done, unless debugging the ``ivl'' subcommand.)

### 3.3 Elaboration

This phase takes the pform and generates a netlist. The driver selects
(by user request or lucky guess) the root module to elaborate,
resolves references and expands the instantiations to form the design
netlist. (See netlist.txt.) Final semantic checks are performed during
elaboration, and some simple optimizations are performed. The netlist
includes all the behavioural descriptions, as well as gates and wires.

The elaborate() function performs the elaboration.

One can see a human-readable version of the final, elaborated and
optimized netlist by using the ``-N <path>'' flag to the compiler. If
elaboration succeeds, the final netlist (i.e., after optimizations but
before code generation) will be dumped into the file named <path>.

Elaboration is performed in two steps: scopes and parameters
first, followed by the structural and behavioural elaboration.

#### 3.3.1 Scope Elaboration

This pass scans through the pform looking for scopes and parameters. A
tree of NetScope objects is built up and placed in the Design object,
with the root module represented by the root NetScope object. The
elab_scope.cc file contains most of the code for handling this phase.

The tail of the elaborate_scope behaviour (after the pform is
traversed) includes a scan of the NetScope tree to locate defparam
assignments that were collected during scope elaboration. This is when
the defparam overrides are applied to the parameters.

#### 3.3.2 Netlist Elaboration

After the scopes and parameters are generated and the NetScope tree
fully formed, the elaboration runs through the pform again, this time
generating the structural and behavioural netlist. Parameters are
elaborated and evaluated by now so all the constants of code
generation are now known locally, so the netlist can be generated by
simply passing through the pform.

### 3.4 Optimization

This is a collection of processing steps that perform
optimizations that do not depend on the target technology. Examples of
some useful transformations are


        - eliminate null effect circuitry
        - combinational reduction
        - constant propagation

The actual functions performed are specified on the ivl command line by
the -F flags (see below).

### 3.5 Code Generation

This step takes the design netlist and uses it to drive the code
generator (see target.h). This may require transforming the
design to suit the technology.

The emit() method of the Design class performs this step. It runs
through the design elements, calling target functions as the need arises
to generate actual output.

The user selects the target code generator with the -t flag on the
command line.


## 4 Unsupported Constructs

  - Specify blocks are parsed but ignored in general.

  - trireg is not supported. tri0 and tri1 are supported.

  - tran primitives, i.e. tran, tranif1, tranif0, rtran, rtranif1
    and rtranif0 are not supported.

  - Event controls inside non-blocking assignments are not supported.
    i.e.: a <= @(posedge clk) b;

