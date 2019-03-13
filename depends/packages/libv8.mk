PACKAGE=libv8
$(package)_version_version=7.3.492
$(package)_download_path=https://github.com/GiantPay/depot_tools/archive
$(package)_file_name=1.0.1.tar.gz
$(package)_sha256_hash=2350f88081ebe367b73636ef654fe41be4765fe0a01a820c02aaa82dfa92e329
$(package)_commit=9df9418edf64d6a080ba569ef43cd5ca0171a820

define $(package)_preprocess_cmds
  export PATH="${PATH}:$($(package)_build_dir)"	&& \
  mkdir -p $($(package)_build_dir)/v8 && \
  cd $($(package)_build_dir)/v8 && \
  $($(package)_build_dir)/fetch v8 && \
  cd $($(package)_build_dir)/v8/v8 && \
  $($(package)_build_dir)/v8/v8/tools/release/mergeinfo.py $($(package)_commit) && \
  git checkout $($(package)_commit) && \
  $($(package)_build_dir)/gclient sync --with_branch_heads && \
  $($(package)_build_dir)/gclient fetch
endef

define $(package)_config_cmds
  cd $($(package)_build_dir)/v8/v8 && \
  $($(package)_build_dir)/v8/v8/tools/dev/v8gen.py x64.release -- \
    v8_monolithic=true \
    is_component_build=false \
    v8_use_external_startup_data=false \
    v8_enable_i18n_support=false \
    icu_use_data_file=false \
    use_custom_libcxx=false \
    use_custom_libcxx_for_host=false \
    v8_enable_test_features=false
endef

define $(package)_build_cmds
  $($(package)_build_dir)/ninja -C $($(package)_build_dir)/v8/v8/out.gn/x64.release
endef

define $(package)_stage_cmds
  mkdir -p $($(package)_staging_prefix_dir)/lib/v8 && \
  mkdir -p $($(package)_staging_prefix_dir)/include && \
  cp -R $($(package)_build_dir)/v8/v8/include/* $($(package)_staging_prefix_dir)/include && \
  cp -R $($(package)_build_dir)/v8/v8/out.gn/x64.release/obj/* $($(package)_staging_prefix_dir)/lib/v8
endef
