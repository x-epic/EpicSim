module main;

   wire clk, iclk;
   wire i0, i1;
   wire out;

   wire [1:0] D = {i1, i0};

      // This statement declares Q to be a 2 bit reg vector. The
      // initial assignment will cause the synthesized device to take
      // on an initial value specified here. Without the assignment,
      // the initial value is unspecified. (Verilog simulates it as 2'bx.)
   reg  [1:0] Q = 2'b10;

      // This simple logic gate get turned into a function unit.
      // The par program will map this into a CLB F or G unit.
   and (out, Q[0], Q[1]);

     // This creates a global clock buffer. Notice how I attach an
     // attribute to the named gate to force it to be mapped to the
     // desired XNF device. This device will not be pulled into the
     // IOB associated with iclk because of the attribute.
   buf gbuf(clk, iclk);
   $attribute(gbuf, "XNF-LCA", "GCLK:O,I");

      // This is mapped to a DFF. Since Q and D are two bits wide, the
      // code generator actually makes two DFF devices that share a
      // clock input.
   always @(posedge clk) Q <= D;

      // These attribute commands assign pins to the listed wires.
      // This can be done to wires and registers, as internally both
      // are treated as named signals.
   $attribute(out, "PAD", "o150");
   $attribute(i0,  "PAD", "i152");
   $attribute(i1,  "PAD", "i153");
   $attribute(iclk,"PAD", "i154");

endmodule /* main */
