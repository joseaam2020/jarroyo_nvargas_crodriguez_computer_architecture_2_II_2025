package fixed_point_pkg;
    typedef logic [15:0] fixed_point_t;

    parameter int FRAC_BITS = 8;
    parameter int INT_BITS  = 8;

    function automatic fixed_point_t fixed_mult(input fixed_point_t a, input fixed_point_t b);
        logic [31:0] product;
        product = a * b;
        return product[23:8];
    endfunction

    function automatic fixed_point_t fixed_add(input fixed_point_t a, input fixed_point_t b);
        return a + b;
    endfunction

endpackage