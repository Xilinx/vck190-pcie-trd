// (C) Copyright 2023 Advanced Micro Devices, Inc.
// SPDX-License-Identifier: Apache-2.0
`timescale 1 ns / 1 ps

	module pcie_reg_space_v1_2 #
	(
		// Users to add parameters here

		// User parameters ends
		// Do not modify the parameters beyond this line


		// Parameters of Axi Slave Bus Interface S00_AXI
		parameter integer C_S00_AXI_DATA_WIDTH	= 32,
		parameter integer C_S00_AXI_ADDR_WIDTH	= 8,

		// Parameters of Axi Slave Bus Interface S01_AXI
		parameter integer C_S01_AXI_DATA_WIDTH	= 32,
		parameter integer C_S01_AXI_ADDR_WIDTH	= 8
	)
	(
		// Users to add ports here
		output wire  IRQ1_to_PS,
		output wire  IRQ2_to_PS,
		output wire  IRQ3_to_PS,
		output wire  IRQ4_to_PS,		
		output wire   [3 :0]IRQ1_to_Host,
		// User ports ends
		// Do not modify the ports beyond this line


		// Ports of Axi Slave Bus Interface S00_AXI
		input wire  s00_axi_aclk,
		input wire  s00_axi_aresetn,
		input wire [C_S00_AXI_ADDR_WIDTH-1 : 0] s00_axi_awaddr,
		input wire [2 : 0] s00_axi_awprot,
		input wire  s00_axi_awvalid,
		output wire  s00_axi_awready,
		input wire [C_S00_AXI_DATA_WIDTH-1 : 0] s00_axi_wdata,
		input wire [(C_S00_AXI_DATA_WIDTH/8)-1 : 0] s00_axi_wstrb,
		input wire  s00_axi_wvalid,
		output wire  s00_axi_wready,
		output wire [1 : 0] s00_axi_bresp,
		output wire  s00_axi_bvalid,
		input wire  s00_axi_bready,
		input wire [C_S00_AXI_ADDR_WIDTH-1 : 0] s00_axi_araddr,
		input wire [2 : 0] s00_axi_arprot,
		input wire  s00_axi_arvalid,
		output wire  s00_axi_arready,
		output wire [C_S00_AXI_DATA_WIDTH-1 : 0] s00_axi_rdata,
		output wire [1 : 0] s00_axi_rresp,
		output wire  s00_axi_rvalid,
		input wire  s00_axi_rready,

		// Ports of Axi Slave Bus Interface S01_AXI
		input wire  s01_axi_aclk,
		input wire  s01_axi_aresetn,
		input wire [C_S01_AXI_ADDR_WIDTH-1 : 0] s01_axi_awaddr,
		input wire [2 : 0] s01_axi_awprot,
		input wire  s01_axi_awvalid,
		output wire  s01_axi_awready,
		input wire [C_S01_AXI_DATA_WIDTH-1 : 0] s01_axi_wdata,
		input wire [(C_S01_AXI_DATA_WIDTH/8)-1 : 0] s01_axi_wstrb,
		input wire  s01_axi_wvalid,
		output wire  s01_axi_wready,
		output wire [1 : 0] s01_axi_bresp,
		output wire  s01_axi_bvalid,
		input wire  s01_axi_bready,
		input wire [C_S01_AXI_ADDR_WIDTH-1 : 0] s01_axi_araddr,
		input wire [2 : 0] s01_axi_arprot,
		input wire  s01_axi_arvalid,
		output wire  s01_axi_arready,
		output wire [C_S01_AXI_DATA_WIDTH-1 : 0] s01_axi_rdata,
		output wire [1 : 0] s01_axi_rresp,
		output wire  s01_axi_rvalid,
		input wire  s01_axi_rready
	);
	wire	 [C_S00_AXI_DATA_WIDTH-1:0] slv_reg0_Host;
	wire	 [C_S00_AXI_DATA_WIDTH-1:0] slv_reg1_Host;
	wire	 [C_S00_AXI_DATA_WIDTH-1:0] slv_reg2_Host;
	wire	 [C_S00_AXI_DATA_WIDTH-1:0] slv_reg3_Host;
	wire	 [C_S00_AXI_DATA_WIDTH-1:0] slv_reg4_Host;
	wire	 [C_S00_AXI_DATA_WIDTH-1:0] slv_reg5_Host;
	wire	 [C_S00_AXI_DATA_WIDTH-1:0] slv_reg6_Host;
	wire	 [C_S00_AXI_DATA_WIDTH-1:0] slv_reg7_Host;
	wire	 [C_S00_AXI_DATA_WIDTH-1:0] slv_reg8_Host;
	wire	 [C_S00_AXI_DATA_WIDTH-1:0] slv_reg9_Host;
	wire	 [C_S00_AXI_DATA_WIDTH-1:0] slv_reg10_Host;
	wire	 [C_S00_AXI_DATA_WIDTH-1:0] slv_reg11_Host;
	wire	 [C_S00_AXI_DATA_WIDTH-1:0] slv_reg12_Host;
	wire	 [C_S00_AXI_DATA_WIDTH-1:0] slv_reg13_Host;
	wire	 [C_S00_AXI_DATA_WIDTH-1:0] slv_reg14_Host;
	wire	 [C_S00_AXI_DATA_WIDTH-1:0] slv_reg15_Host;
	wire	 [C_S00_AXI_DATA_WIDTH-1:0] slv_reg16_Host;
	wire	 [C_S00_AXI_DATA_WIDTH-1:0] slv_reg17_Host;
	wire	 [C_S00_AXI_DATA_WIDTH-1:0] slv_reg18_Host;
	wire	 [C_S00_AXI_DATA_WIDTH-1:0] slv_reg19_Host;
	wire	 [C_S00_AXI_DATA_WIDTH-1:0] slv_reg20_Host;
	wire	 [C_S00_AXI_DATA_WIDTH-1:0] slv_reg21_Host;
	wire	 [C_S00_AXI_DATA_WIDTH-1:0] slv_reg22_Host;
	wire	 [C_S00_AXI_DATA_WIDTH-1:0] slv_reg23_Host;
	wire	 [C_S00_AXI_DATA_WIDTH-1:0] slv_reg24_Host;
	wire	 [C_S00_AXI_DATA_WIDTH-1:0] slv_reg25_Host;
	wire	 [C_S00_AXI_DATA_WIDTH-1:0] slv_reg26_Host;
	wire	 [C_S00_AXI_DATA_WIDTH-1:0] slv_reg27_Host;
	wire	 [C_S00_AXI_DATA_WIDTH-1:0] slv_reg28_Host;
	wire	 [C_S00_AXI_DATA_WIDTH-1:0] slv_reg29_Host;
	wire	 [C_S00_AXI_DATA_WIDTH-1:0] slv_reg30_Host;
	wire	 [C_S00_AXI_DATA_WIDTH-1:0] slv_reg31_Host;
	
	wire	 [C_S01_AXI_DATA_WIDTH-1:0] slv_reg0_PS;
	wire	 [C_S01_AXI_DATA_WIDTH-1:0] slv_reg1_PS;
	wire	 [C_S01_AXI_DATA_WIDTH-1:0] slv_reg2_PS;
	wire	 [C_S01_AXI_DATA_WIDTH-1:0] slv_reg3_PS;
	wire	 [C_S01_AXI_DATA_WIDTH-1:0] slv_reg4_PS;
	wire	 [C_S01_AXI_DATA_WIDTH-1:0] slv_reg5_PS;
	wire	 [C_S01_AXI_DATA_WIDTH-1:0] slv_reg6_PS;
	wire	 [C_S01_AXI_DATA_WIDTH-1:0] slv_reg7_PS;
	wire	 [C_S01_AXI_DATA_WIDTH-1:0] slv_reg8_PS;
	wire	 [C_S01_AXI_DATA_WIDTH-1:0] slv_reg9_PS;
	wire	 [C_S01_AXI_DATA_WIDTH-1:0] slv_reg10_PS;
	wire	 [C_S01_AXI_DATA_WIDTH-1:0] slv_reg11_PS;
	wire	 [C_S01_AXI_DATA_WIDTH-1:0] slv_reg12_PS;
	wire	 [C_S01_AXI_DATA_WIDTH-1:0] slv_reg13_PS;
	wire	 [C_S01_AXI_DATA_WIDTH-1:0] slv_reg14_PS;
	wire	 [C_S01_AXI_DATA_WIDTH-1:0] slv_reg15_PS;
	wire	 [C_S01_AXI_DATA_WIDTH-1:0] slv_reg16_PS;
	wire	 [C_S01_AXI_DATA_WIDTH-1:0] slv_reg17_PS;
	wire	 [C_S01_AXI_DATA_WIDTH-1:0] slv_reg18_PS;
	wire	 [C_S01_AXI_DATA_WIDTH-1:0] slv_reg19_PS;
	wire	 [C_S01_AXI_DATA_WIDTH-1:0] slv_reg20_PS;
	wire	 [C_S01_AXI_DATA_WIDTH-1:0] slv_reg21_PS;
	wire	 [C_S01_AXI_DATA_WIDTH-1:0] slv_reg22_PS;
	wire	 [C_S01_AXI_DATA_WIDTH-1:0] slv_reg23_PS;
	wire	 [C_S01_AXI_DATA_WIDTH-1:0] slv_reg24_PS;
	wire	 [C_S01_AXI_DATA_WIDTH-1:0] slv_reg25_PS;
	wire	 [C_S01_AXI_DATA_WIDTH-1:0] slv_reg26_PS;
	wire	 [C_S01_AXI_DATA_WIDTH-1:0] slv_reg27_PS;
	wire	 [C_S01_AXI_DATA_WIDTH-1:0] slv_reg28_PS;
	wire	 [C_S01_AXI_DATA_WIDTH-1:0] slv_reg29_PS;
	wire	 [C_S01_AXI_DATA_WIDTH-1:0] slv_reg30_PS;
	wire	 [C_S01_AXI_DATA_WIDTH-1:0] slv_reg31_PS;
// Instantiation of Axi Bus Interface S00_AXI
	pcie_reg_space_v1_2_S00_AXI # ( 
		.C_S_AXI_DATA_WIDTH(C_S00_AXI_DATA_WIDTH),
		.C_S_AXI_ADDR_WIDTH(C_S00_AXI_ADDR_WIDTH)
	) pcie_reg_space_v1_2_S00_AXI_inst (
		.S_AXI_ACLK(s00_axi_aclk),
		.S_AXI_ARESETN(s00_axi_aresetn),
		.S_AXI_AWADDR(s00_axi_awaddr),
		.S_AXI_AWPROT(s00_axi_awprot),
		.S_AXI_AWVALID(s00_axi_awvalid),
		.S_AXI_AWREADY(s00_axi_awready),
		.S_AXI_WDATA(s00_axi_wdata),
		.S_AXI_WSTRB(s00_axi_wstrb),
		.S_AXI_WVALID(s00_axi_wvalid),
		.S_AXI_WREADY(s00_axi_wready),
		.S_AXI_BRESP(s00_axi_bresp),
		.S_AXI_BVALID(s00_axi_bvalid),
		.S_AXI_BREADY(s00_axi_bready),
		.S_AXI_ARADDR(s00_axi_araddr),
		.S_AXI_ARPROT(s00_axi_arprot),
		.S_AXI_ARVALID(s00_axi_arvalid),
		.S_AXI_ARREADY(s00_axi_arready),
		.S_AXI_RDATA(s00_axi_rdata),
		.S_AXI_RRESP(s00_axi_rresp),
		.S_AXI_RVALID(s00_axi_rvalid),
		.S_AXI_RREADY(s00_axi_rready),
		.slv_reg0_output(slv_reg0_Host),
		.slv_reg1_output(slv_reg1_Host),
		.slv_reg2_output(slv_reg2_Host),
		.slv_reg3_output(slv_reg3_Host),
		.slv_reg4_output(slv_reg4_Host),
		.slv_reg5_output(slv_reg5_Host),
		.slv_reg6_output(slv_reg6_Host),
		.slv_reg7_output(slv_reg7_Host),
		.slv_reg8_output(slv_reg8_Host),
		.slv_reg9_output(slv_reg9_Host),
		.slv_reg10_output(slv_reg10_Host),
		.slv_reg11_output(slv_reg11_Host),
		.slv_reg12_output(slv_reg12_Host),
		.slv_reg13_output(slv_reg13_Host),
		.slv_reg14_output(slv_reg14_Host),
		.slv_reg15_output(slv_reg15_Host),
		.slv_reg16_output(slv_reg16_Host),
		.slv_reg17_output(slv_reg17_Host),
		.slv_reg18_output(slv_reg18_Host),
		.slv_reg19_output(slv_reg19_Host),
		.slv_reg20_output(slv_reg20_Host),
		.slv_reg21_output(slv_reg21_Host),
		.slv_reg22_output(slv_reg22_Host),
		.slv_reg23_output(slv_reg23_Host),
		.slv_reg24_output(slv_reg24_Host),
		.slv_reg25_output(slv_reg25_Host),
		.slv_reg26_output(slv_reg26_Host),
		.slv_reg27_output(slv_reg27_Host),
		.slv_reg28_output(slv_reg28_Host),
		.slv_reg29_output(slv_reg29_Host),
		.slv_reg30_output(slv_reg30_Host),
		.slv_reg31_output(slv_reg31_Host),		
		.slv_reg32_input(slv_reg0_PS),
		.slv_reg33_input(slv_reg1_PS),
		.slv_reg34_input(slv_reg2_PS),
		.slv_reg35_input(slv_reg3_PS),
		.slv_reg36_input(slv_reg4_PS),
		.slv_reg37_input(slv_reg5_PS),
		.slv_reg38_input(slv_reg6_PS),
		.slv_reg39_input(slv_reg7_PS),
		.slv_reg40_input(slv_reg8_PS),
		.slv_reg41_input(slv_reg9_PS),
		.slv_reg42_input(slv_reg10_PS),
		.slv_reg43_input(slv_reg11_PS),
		.slv_reg44_input(slv_reg12_PS),
        .slv_reg45_input(slv_reg13_PS),
		.slv_reg46_input(slv_reg14_PS),
		.slv_reg47_input(slv_reg15_PS),
		.slv_reg48_input(slv_reg16_PS),
		.slv_reg49_input(slv_reg17_PS),
		.slv_reg50_input(slv_reg18_PS),
		.slv_reg51_input(slv_reg19_PS),
		.slv_reg52_input(slv_reg20_PS),
		.slv_reg53_input(slv_reg21_PS),
		.slv_reg54_input(slv_reg22_PS),
		.slv_reg55_input(slv_reg23_PS),
		.slv_reg56_input(slv_reg24_PS),
		.slv_reg57_input(slv_reg25_PS),
		.slv_reg58_input(slv_reg26_PS),
		.slv_reg59_input(slv_reg27_PS),	
		.slv_reg60_input(slv_reg28_PS),
		.slv_reg61_input(slv_reg29_PS),	
		.slv_reg62_input(slv_reg30_PS),
		.slv_reg63_input(slv_reg31_PS),
		.IRQ1_to_PS(IRQ1_to_PS),
		.IRQ2_to_PS(IRQ2_to_PS),
		.IRQ3_to_PS(IRQ3_to_PS),
		.IRQ4_to_PS(IRQ4_to_PS),
	    .S_AXI_ARADDR_clr(s01_axi_araddr),
	    .S_AXI_ARVALID_CLR(s01_axi_arvalid)

	);

// Instantiation of Axi Bus Interface S01_AXI
	pcie_reg_space_v1_2_S01_AXI # ( 
		.C_S_AXI_DATA_WIDTH(C_S01_AXI_DATA_WIDTH),
		.C_S_AXI_ADDR_WIDTH(C_S01_AXI_ADDR_WIDTH)
	) pcie_reg_space_v1_2_S01_AXI_inst (
		.S_AXI_ACLK(s01_axi_aclk),
		.S_AXI_ARESETN(s01_axi_aresetn),
		.S_AXI_AWADDR(s01_axi_awaddr),
		.S_AXI_AWPROT(s01_axi_awprot),
		.S_AXI_AWVALID(s01_axi_awvalid),
		.S_AXI_AWREADY(s01_axi_awready),
		.S_AXI_WDATA(s01_axi_wdata),
		.S_AXI_WSTRB(s01_axi_wstrb),
		.S_AXI_WVALID(s01_axi_wvalid),
		.S_AXI_WREADY(s01_axi_wready),
		.S_AXI_BRESP(s01_axi_bresp),
		.S_AXI_BVALID(s01_axi_bvalid),
		.S_AXI_BREADY(s01_axi_bready),
		.S_AXI_ARADDR(s01_axi_araddr),
		.S_AXI_ARPROT(s01_axi_arprot),
		.S_AXI_ARVALID(s01_axi_arvalid),
		.S_AXI_ARREADY(s01_axi_arready),
		.S_AXI_RDATA(s01_axi_rdata),
		.S_AXI_RRESP(s01_axi_rresp),
		.S_AXI_RVALID(s01_axi_rvalid),
		.S_AXI_RREADY(s01_axi_rready),
		.slv_reg0_output(slv_reg0_PS), 
		 .slv_reg1_output(slv_reg1_PS), 
		 .slv_reg2_output(slv_reg2_PS), 
		 .slv_reg3_output(slv_reg3_PS), 
		 .slv_reg4_output(slv_reg4_PS), 
		 .slv_reg5_output(slv_reg5_PS), 
		 .slv_reg6_output(slv_reg6_PS), 
		 .slv_reg7_output(slv_reg7_PS), 
		 .slv_reg8_output(slv_reg8_PS), 
		 .slv_reg9_output(slv_reg9_PS), 
		 .slv_reg10_output(slv_reg10_PS),
		 .slv_reg11_output(slv_reg11_PS),
		 .slv_reg12_output(slv_reg12_PS),
		 .slv_reg13_output(slv_reg13_PS),
		 .slv_reg14_output(slv_reg14_PS),
		.slv_reg15_output(slv_reg15_PS), 
		 .slv_reg16_output(slv_reg16_PS), 
		 .slv_reg17_output(slv_reg17_PS), 
		 .slv_reg18_output(slv_reg18_PS), 
		 .slv_reg19_output(slv_reg19_PS), 
		 .slv_reg20_output(slv_reg20_PS), 
		 .slv_reg21_output(slv_reg21_PS), 
		 .slv_reg22_output(slv_reg22_PS), 
		 .slv_reg23_output(slv_reg23_PS), 
		 .slv_reg24_output(slv_reg24_PS), 
		 .slv_reg25_output(slv_reg25_PS),
		 .slv_reg26_output(slv_reg26_PS),
		 .slv_reg27_output(slv_reg27_PS),
		 .slv_reg28_output(slv_reg28_PS),
		 .slv_reg29_output(slv_reg29_PS),   
		 .slv_reg30_output(slv_reg30_PS),
		 .slv_reg31_output(slv_reg31_PS),
		 .slv_reg32_input(slv_reg0_Host),
		.slv_reg33_input(slv_reg1_Host),
		.slv_reg34_input(slv_reg2_Host),
		.slv_reg35_input(slv_reg3_Host),
		.slv_reg36_input(slv_reg4_Host),
		.slv_reg37_input(slv_reg5_Host),
		.slv_reg38_input(slv_reg6_Host),
		.slv_reg39_input(slv_reg7_Host),
		.slv_reg40_input(slv_reg8_Host),
		.slv_reg41_input(slv_reg9_Host),
		.slv_reg42_input(slv_reg10_Host),
		.slv_reg43_input(slv_reg11_Host),
		.slv_reg44_input(slv_reg12_Host),
        .slv_reg45_input(slv_reg13_Host),
		.slv_reg46_input(slv_reg14_Host),
		.slv_reg47_input(slv_reg15_Host),
		.slv_reg48_input(slv_reg16_Host),
		.slv_reg49_input(slv_reg17_Host),
		.slv_reg50_input(slv_reg18_Host),
		.slv_reg51_input(slv_reg19_Host),
		.slv_reg52_input(slv_reg20_Host),
		.slv_reg53_input(slv_reg21_Host),
		.slv_reg54_input(slv_reg22_Host),
		.slv_reg55_input(slv_reg23_Host),
		.slv_reg56_input(slv_reg24_Host),
		.slv_reg57_input(slv_reg25_Host),
		.slv_reg58_input(slv_reg26_Host),
		.slv_reg59_input(slv_reg27_Host),	
		.slv_reg60_input(slv_reg28_Host),
		.slv_reg61_input(slv_reg29_Host),	
		.slv_reg62_input(slv_reg30_Host),
		.slv_reg63_input(slv_reg31_Host), 
		.IRQ1_to_Host(IRQ1_to_Host),
		.S_AXI_ARADDR_clr(s00_axi_araddr),
	   	.S_AXI_ARVALID_CLR(s00_axi_arvalid)      
	);

	// Add user logic here

	// User logic ends

	endmodule
