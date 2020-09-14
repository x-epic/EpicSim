module ram16x1 (q, d, a, we, wclk);
   output q;
   input d;
   input [3:0] a;
   input we;
   input wclk;

   reg mem[15:0];

   assign q = mem[a];
   always @(posedge wclk) if (we) mem[a] = d;

endmodule /* ram16x1 */

module main;
   wire q;
   reg d;
   reg [3:0] a;
   reg we, wclk;

   ram16x1 r1 (q, d, a, we, wclk);

   initial begin
      $monitor("q = %b", q);
      d = 0;
      wclk = 0;
      a = 5;
      we = 1;
      #1 wclk = 1;
      #1 wclk = 0;
   end
endmodule /* main */
