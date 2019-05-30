/* Copyright 2017 Columbia University, SLD Group */

#ifndef __SDP_HPP__
#define __SDP_HPP__

#include "nmf_data.hpp"

#include "sdp_config.hpp"

#include "sdp_directives.hpp"

#include "esp_templates.hpp"

//#include A_MEMORY_HEADER

#define STORE_CHUNK 32
#define N_STORE_CHUNKS (DMA_CHUNK / STORE_CHUNK)

class sdp: public esp_accelerator_3P<DMA_WIDTH>
{
    public:

        // -- Instances

        // Config process
        esp_config_proc cfg;

        // -- Handshakes

        // Additional handshake
        handshake_t output_done;

        // Declaration of the accelerator PLMs
        FPDATA_WORD PLM_A0[DMA_CHUNK];
        FPDATA_WORD PLM_A1[DMA_CHUNK];
        FPDATA_WORD PLM_B0[STORE_CHUNK];
        FPDATA_WORD PLM_B1[STORE_CHUNK];
	// A_MEMORY_TYPE<FPDATA_WORD, DMA_CHUNK> PLM_A0;
	// A_MEMORY_TYPE<FPDATA_WORD, DMA_CHUNK> PLM_A1;
        // A_MEMORY_TYPE<FPDATA_WORD, DMA_CHUNK> PLM_B0;
        // A_MEMORY_TYPE<FPDATA_WORD, DMA_CHUNK> PLM_B1;

        // -- Module constructor
        SC_HAS_PROCESS(sdp);
        sdp(const sc_module_name &name)
            : esp_accelerator_3P<DMA_WIDTH>(name)
            , cfg("configuration_process")
            , output_done("output_done")
        {
            // Signal binding with config
            cfg.bind_with<DMA_WIDTH>(*this);

            // Signal binding with the handshake
            output_done.bind_with<DMA_WIDTH>(*this);

            // Binding explicit memories
	    HLS_FLATTEN_ARRAY(PLM_A0);
	    HLS_FLATTEN_ARRAY(PLM_A1);
	    HLS_FLATTEN_ARRAY(PLM_B0);
	    HLS_FLATTEN_ARRAY(PLM_B1);
            // PLM_A0.clk(this->clk);
            // PLM_A1.clk(this->clk);
            // PLM_B0.clk(this->clk);
            // PLM_B1.clk(this->clk);
        }

        // -- Processes

        // Load input from memory
        void load_input();

        // Store output in memory
        void compute_kernel();

        // Store output in memory
        void store_output();

        // -- Functions
        inline void load_store_handshake();
        inline void store_load_handshake();

        // -- Kernels
};

#endif // __SDP_HPP__
