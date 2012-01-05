stv_mbx_ref_config: unconfig
	$(MKCONFIG) $(@:_config=)  arm aml_meson stv_mbx_ref amlogic m1
stv_mbx_ref_config_help:
	@echo =======================================================================
	@echo The mark in board is "STV MediaBox config for AMLogic Devlopment board"
	@echo config command: \"make $(@:%_help=%)\"
help:stv_mbx_ref_config_help
