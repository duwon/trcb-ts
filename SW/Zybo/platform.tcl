# 
# Usage: To re-create this platform project launch xsct with below options.
# xsct C:\Users\duwon\OneDrive\Documents\Code_Xilinx\Zybo_EVM\Zybo_SW\Zybo\platform.tcl
# 
# OR launch xsct and run below command.
# source C:\Users\duwon\OneDrive\Documents\Code_Xilinx\Zybo_EVM\Zybo_SW\Zybo\platform.tcl
# 
# To create the platform in a different location, modify the -out option of "platform create" command.
# -out option specifies the output directory of the platform project.

platform active {Zybo}
domain active {zynq_fsbl}
bsp reload
catch {bsp regenerate}
domain active {standalone_ps7_cortexa9_0}
bsp reload
catch {bsp regenerate}
platform generate
bsp reload
bsp reload
catch {bsp regenerate}
bsp reload
domain active {zynq_fsbl}
bsp reload
domain active {standalone_ps7_cortexa9_0}
bsp reload
platform clean
platform generate
bsp reload
platform config -updatehw {C:/Users/duwon/Documents/FPGA_CODE/Zybo_EVM/Zybo_HW/Zybo.xsa}
platform config -updatehw {C:/Users/duwon/OneDrive/Documents/Code_Xilinx/Zybo_EVM/Zybo_HW/Zybo.xsa}
domain active {zynq_fsbl}
bsp reload
catch {bsp regenerate}
domain active {standalone_ps7_cortexa9_0}
bsp reload
platform generate
platform config -updatehw {C:/Users/duwon/OneDrive/Documents/Code_Xilinx/Zybo_EVM/Zybo_HW/Zybo.xsa}
platform generate -domains 
platform config -updatehw {C:/Users/duwon/OneDrive/Documents/Code_Xilinx/Zybo_EVM/Zybo_HW/Zybo.xsa}
platform generate -domains 
platform generate
platform clean
