PACKAGE=libv8
$(package)_version_version=6.9.427.19
$(package)_download_path=https://github.com/GiantPay/depot_tools/archive
$(package)_file_name=1.0.0.tar.gz
$(package)_sha256_hash=fa3a646f764d3605d19af5c001039cf820976cef962ccca59951fde95ff2556f

define $(package)_preprocess_cmds
  export PATH="${PATH}:$($(package)_build_dir)"	&& \
  mkdir -p $($(package)_build_dir)/v8 && \
  cd $($(package)_build_dir)/v8 && \
  $($(package)_build_dir)/fetch v8 && \
  cd $($(package)_build_dir)/v8/v8 && \
  $($(package)_build_dir)/gclient sync --with_branch_heads && \
  $($(package)_build_dir)/gclient fetch && \
  $($(package)_build_dir)/v8/v8/tools/release/mergeinfo.py aae279118869ad4ae1f31513a8b66a17ff2f0dd4 && \
  git checkout aae279118869ad4ae1f31513a8b66a17ff2f0dd4
endef

define $(package)_config_cmds
  cd $($(package)_build_dir)/v8/v8 && \
  $($(package)_build_dir)/v8/v8/tools/dev/v8gen.py x64.release -- \
    v8_monolithic=true \
    v8_use_external_startup_data=false \
    v8_enable_i18n_support=false \
    use_custom_libcxx=false \
    use_custom_libcxx_for_host=false
endef

define $(package)_build_cmds
  cd $($(package)_build_dir)/v8/v8 && \
  $($(package)_build_dir)/ninja -C out.gn/x64.release
endef

define $(package)_stage_cmds
  mkdir -p $($(package)_staging_prefix_dir)/lib/v8 && \
  mkdir -p $($(package)_staging_prefix_dir)/include && \
  cp -R $($(package)_build_dir)/v8/v8/include/* $($(package)_staging_prefix_dir)/include && \
  cp -R $($(package)_build_dir)/v8/v8/out.gn/x64.release/obj/* $($(package)_staging_prefix_dir)/lib/v8
endef
