module top;

    import "DPI-C" function void init_python_env();
    import "DPI-C" function void deinit_python_env();
    
    // Due to the definition from SV-DPI docs, the appended square bracket pair is required,
    // in despite of the zero dimension data, e.g. `bit[N:0] val` should be `bit[N:0] val[]`,
    // or the compiler will parse it as `svBitVecVal` instead of `svOpenArrayHandle`.

    import "DPI-C" function void array_handle(
        string filename, string funcname, 
        input bit[23:0] array_in[5:0][1:0][], output bit[15:0] array_out[5:0][1:0][]);

    initial begin
        bit [23:0] da_value [5:0][1:0];
        bit [15:0] ad_value [5:0][1:0];

        foreach(da_value[i, j])
        begin
            da_value[i][j] = 24'(i) + 24'(j << 8);
        end

        init_python_env();

        array_handle("payload", "callback", da_value, ad_value);

        foreach(ad_value[i, j])
        begin
            $display("[SV] ad_value[%d][%d] = %d", i, j, ad_value[i][j]);
        end

        deinit_python_env();
        $finish;
    end

endmodule
