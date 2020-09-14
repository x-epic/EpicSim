module main;

   wire clk, iclk;
   wire i0, i1;
   wire out;
   reg o0;

      // This simple logic gate get turned into a function unit.
      // The par program will map this into a CLB F or G unit.
   and (out, i0, i1);

     // This creates a global clock buffer. Notice how I attach an
     // attribute to the named gate to force it to be mapped to the
     // desired XNF device. This device will not be pulled into the
     // IOB associated with iclk because of the attribute.
   buf gbuf(clk, iclk);
   $attribute(gbuf, "XNF-LCA", "GCLK:O,I");

      // This is mapped to a DFF. Since o0 is connected to a PAD, it
      // is turned into a OUTFF so that it get placed into an IOB.
   always @(posedge clk) o0 = out;

      // These attribute commands assign pins to the listed wires.
      // This can be done to wires and registers, as internally both
      // are treated as named signals.
   $attribute(o0,  "PAD", "o150");
   $attribute(i0,  "PAD", "i152");
   $attribute(i1,  "PAD", "i153");
   $attribute(iclk,"PAD", "i154");

endmodule /* main */
