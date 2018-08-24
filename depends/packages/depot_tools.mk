PACKAGE=depot_tools
$(package)_version_version=1.0
$(package)_download_path=https://chromium.googlesource.com/chromium/tools/depot_tools.git/+/1e82867e3d0f8f3f96672c0172ff4adad1777ce3#
$(package)_file_name=1e82867e3d0f8f3f96672c0172ff4adad1777ce3.tar.gz
$(package)_sha256_hash=73b61d74f63ccba6e4972bd4b52b1acb463c4bac9cfc5a5b52b2a1999520b266

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