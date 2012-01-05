stv_mid_ref_config: unconfig
	$(MKCONFIG) $(@:_config=)  arm aml_meson stv_mid_ref amlogic m1
stv_mid_ref_config_help:
	@echo =======================================================================
	@echo The mark in board is "MID"
	@echo config command: \"make $(@:%_help=%)\"
help:stv_mid_ref_config_help
