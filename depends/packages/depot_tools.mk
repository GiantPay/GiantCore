PACKAGE=depot_tools
$(package)_version_version=1.0
$(package)_download_path=https://github.com/GiantPay/depot_tools/archive
$(package)_file_name=1.0.0.tar.gz
$(package)_sha256_hash=fa3a646f764d3605d19af5c001039cf820976cef962ccca59951fde95ff2556f

define $(package)_set_vars
  $(package)_config_opts=--disable-static
  $(package)_config_opts_linux=--with-pic
endef

define $(package)_fetch_cmds
$(call fetch_file,$(package),$($(package)_download_path),$($(package)_download_file),$($(package)_file_name),$($(package)_sha256_hash))

endef

define $(package)_config_cmds
  $($(package)_autoconf)
endef

define $(package)_build_cmds
  $(MAKE)
endef

define $(package)_stage_cmds
  $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef

define $(package)_build_cmds
  gclient
endef

define $(package)_build_cmds
  $(package)_fetch_cmds=fetch v8
  cd v8
  gclient sync
endef

define $(package)_build_cmds
./build/install-build-deps.sh
endef

define $(package)_build_cmds
tools/dev/v8gen.py x64.release
endef

define $(package)_build_cmds
ninja -C out.gn/x64.release
endef

define $(package)_build_cmds
tools/run-tests.py --gn
endef