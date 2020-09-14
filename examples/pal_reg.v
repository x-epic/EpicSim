/*
 * The register module is an 8 bit register that copies the input to
 * the output registers on the rising edge of the clk input. The
 * always statement creates a simple d-type flip-flop that is loaded
 * on the rising edge of the clock.
 *
 * The output drivers are controlled by a single active low output
 * enable. I used bufif0 devices in this example, but the exact same
 * thing can be achieved with a continuous assignment like so:
 *
 *   assign out = oe? 8'hzz : Q;
 *
 * Many people prefer the expression form. It is true that it does
 * seem to express the intent a bit more clearly.
 */
module register (out, val, clk, oe);

   output [7:0] out;
   input [7:0]	val;
   input	clk, oe;

   reg [7:0]	Q;

   wire [7:0]	out;

   bufif0 drv[7:0](out, Q, oe);

   always @(posedge clk) Q = val;

endmodule


/*
 * The module pal is used to attach pin information to all the pins of
 * the device. We use this to lock down the pin assignments of the
 * synthesized result. The pin number assignments are for a 22v10 in
 * a PLCC package.
 *
 * Note that this module has no logic in it. It is a convention I use
 * that I put all the functionality in a separate module (seen above)
 * and isolate the EpicSim specific $attribute madness into a
 * top-level module. The advantage of this style is that the entire
 * module can be `ifdef'ed out when doing simulation and you don't
 * need to worry that functionality will be affected.
 */
module pal;

   wire out7, out6, out5, out4, out3, out2, out1, out0;
   wire inp7, inp6, inp5, inp4, inp3, inp2, inp1, inp0;
   wire clk, oe;

   // The PAD attributes attach the wires to pins of the
   // device. Output pins are prefixed by a 'o', and input pins by an
   // 'i'. If not all the available output pins are used, then the
   // remaining are available for the synthesizer to drop internal
   // registers or extra logic layers.
   $attribute(out7, "PAD", "o27");
   $attribute(out6, "PAD", "o26");
   $attribute(out5, "PAD", "o25");
   $attribute(out4, "PAD", "o24");
   $attribute(out3, "PAD", "o23");
   $attribute(out2, "PAD", "o21");
   $attribute(out1, "PAD", "o20");
   $attribute(out0, "PAD", "o19");

   $attribute(inp7, "PAD", "i10");
   $attribute(inp6, "PAD", "i9");
   $attribute(inp5, "PAD", "i7");
   $attribute(inp4, "PAD", "i6");
   $attribute(inp3, "PAD", "i5");
   $attribute(inp2, "PAD", "i4");
   $attribute(inp1, "PAD", "i3");
   $attribute(inp0, "PAD", "i2");

   //$attribute(clk, "PAD", "CLK");
   $attribute(oe,  "PAD", "i13");

   register dev({out7, out6, out5, out4, out3, out2, out1, out0},
		{inp7, inp6, inp5, inp4, inp3, inp2, inp1, inp0},
		clk, oe);

endmodule // pal
