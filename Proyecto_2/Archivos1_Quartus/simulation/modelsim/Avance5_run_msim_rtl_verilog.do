transcript on
if {[file exists rtl_work]} {
	vdel -lib rtl_work -all
}
vlib rtl_work
vmap work rtl_work

vlog -sv -work work +incdir+/home/noemi/Documentos/Arqui/Proyecto2/Avance5-codigoQuartus {/home/noemi/Documentos/Arqui/Proyecto2/Avance5-codigoQuartus/fixed_point_pkg.sv}
vlog -sv -work work +incdir+/home/noemi/Documentos/Arqui/Proyecto2/Avance5-codigoQuartus {/home/noemi/Documentos/Arqui/Proyecto2/Avance5-codigoQuartus/bilinear_interpolator.sv}

vlog -sv -work work +incdir+/home/noemi/Documentos/Arqui/Proyecto2/Avance5-codigoQuartus {/home/noemi/Documentos/Arqui/Proyecto2/Avance5-codigoQuartus/bilinear_interpolator_tb.sv}

vsim -t 1ps -L altera_ver -L lpm_ver -L sgate_ver -L altera_mf_ver -L altera_lnsim_ver -L cyclonev_ver -L cyclonev_hssi_ver -L cyclonev_pcie_hip_ver -L rtl_work -L work -voptargs="+acc"  tb_bilinear_interpolator

add wave *
view structure
view signals
run -all
