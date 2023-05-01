#include "Vtop.h"
#include "Vtop__Dpi.h"
#include <string.h>

int main(int argc, char **argv) {
  // See a similar example walkthrough in the verilator manpage.

  // This is intended to be a minimal example.  Before copying this to start a
  // real project, it is better to start with a more complete example,
  // e.g. examples/c_tracing.

  // Construct a VerilatedContext to hold simulation time, etc.
  VerilatedContext *contextp = new VerilatedContext;

  // Pass arguments so Verilated code can see them, e.g. $value$plusargs
  // This needs to be called before you create any model
  contextp->commandArgs(argc, argv);

  // Construct the Verilated model, from Vtop.h generated from Verilating
  // "top.v"
  Vtop *top = new Vtop{contextp};

  // Simulate until $finish
  while (!contextp->gotFinish()) {

    // Evaluate model
    top->eval();
  }

  // Final model cleanup
  top->final();

  // Destroy model
  delete top;

  // Return good completion status
  return 0;
}