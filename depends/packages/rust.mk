package=rust
$(package)_version=1.47.0
$(package)_download_path=https://static.rust-lang.org/dist
$(package)_file_name_linux=rust-$($(package)_version)-x86_64-unknown-linux-gnu.tar.gz
$(package)_sha256_hash_linux=d0e11e1756a072e8e246b05d54593402813d047d12e44df281fbabda91035d96
$(package)_file_name_darwin=rust-$($(package)_version)-x86_64-apple-darwin.tar.gz
$(package)_sha256_hash_darwin=84e5be6c5c78734deba911dcf80316be1e4c7da2c59413124d039ad96620612f
$(package)_file_name_freebsd=rust-$($(package)_version)-x86_64-unknown-freebsd.tar.gz
$(package)_sha256_hash_freebsd=650af0288d099c9debef7258a27caf15dd8aaf033ee1a099b4c5216c95ecfeaa

# Mapping from GCC canonical hosts to Rust targets
# If a mapping is not present, we assume they are identical, unless $host_os is
# "darwin", in which case we assume x86_64-apple-darwin.
$(package)_rust_target_x86_64-w64-mingw32=x86_64-pc-windows-gnu
$(package)_rust_target_i686-pc-linux-gnu=i686-unknown-linux-gnu
$(package)_rust_target_riscv64-linux-gnu=riscv64gc-unknown-linux-gnu
$(package)_rust_target_riscv64-unknown-linux-gnu=riscv64gc-unknown-linux-gnu
$(package)_rust_target_x86_64-linux-gnu=x86_64-unknown-linux-gnu
$(package)_rust_target_x86_64-pc-linux-gnu=x86_64-unknown-linux-gnu

# Mapping from Rust targets to SHA-256 hashes
$(package)_rust_std_sha256_hash_aarch64-unknown-linux-gnu=0019c302a0a02d8a9e40c3bcdd5a31b9b2704161563d72df3572521989182b0c
$(package)_rust_std_sha256_hash_arm-unknown-linux-gnueabihf=06f81e14b008e55fc3623cc165e45f3494585cf13d8bd0cb04934eb203e8b136
$(package)_rust_std_sha256_hash_i686-unknown-linux-gnu=2a5b8e26c3b61046ab9f420d905b680779cb5521a64b3d2a1846f5dab68c0ba7
$(package)_rust_std_sha256_hash_riscv64gc-unknown-linux-gnu=be69761efc78d7b8ae537498c91af1adf10d9641555ae3cef8cacc2cb3d17e70
$(package)_rust_std_sha256_hash_x86_64-apple-darwin=6b86bcdad5a6eff87a67b6387051d7f10a48e088b8f92d76869d201500b9ce13
$(package)_rust_std_sha256_hash_x86_64-pc-windows-gnu=92e1a1012011cc455a8dff15f6f4770a5f6a01d495526b0969afae655998a5ad
$(package)_rust_std_sha256_hash_x86_64-unknown-linux-gnu=17ecad27d96b331608e4a96dfa3cad05ccb2ccecb888894ed35054e0d1f5207f

define rust_target
$(if $($(1)_rust_target_$(2)),$($(1)_rust_target_$(2)),$(if $(findstring darwin,$(3)),x86_64-apple-darwin,$(2)))
endef

ifneq ($(canonical_host),$(build))
$(package)_rust_target=$(call rust_target,$(package),$(canonical_host),$(host_os))
$(package)_exact_file_name=rust-std-$($(package)_version)-$($(package)_rust_target).tar.gz
$(package)_exact_sha256_hash=$($(package)_rust_std_sha256_hash_$($(package)_rust_target))
$(package)_build_subdir=buildos
$(package)_extra_sources=$($(package)_file_name_$(build_os))

define $(package)_fetch_cmds
$(call fetch_file,$(package),$($(package)_download_path),$($(package)_download_file),$($(package)_file_name),$($(package)_sha256_hash)) && \
$(call fetch_file,$(package),$($(package)_download_path),$($(package)_file_name_$(build_os)),$($(package)_file_name_$(build_os)),$($(package)_sha256_hash_$(build_os)))
endef

define $(package)_extract_cmds
  mkdir -p $($(package)_extract_dir) && \
  echo "$($(package)_sha256_hash)  $($(package)_source)" > $($(package)_extract_dir)/.$($(package)_file_name).hash && \
  echo "$($(package)_sha256_hash_$(build_os))  $($(package)_source_dir)/$($(package)_file_name_$(build_os))" >> $($(package)_extract_dir)/.$($(package)_file_name).hash && \
  $(build_SHA256SUM) -c $($(package)_extract_dir)/.$($(package)_file_name).hash && \
  mkdir $(canonical_host) && \
  tar --strip-components=1 -xf $($(package)_source) -C $(canonical_host) && \
  mkdir buildos && \
  tar --strip-components=1 -xf $($(package)_source_dir)/$($(package)_file_name_$(build_os)) -C buildos
endef

define $(package)_stage_cmds
  bash ./install.sh --destdir=$($(package)_staging_dir) --prefix=$(host_prefix)/native --disable-ldconfig && \
  ../$(canonical_host)/install.sh --destdir=$($(package)_staging_dir) --prefix=$(host_prefix)/native --disable-ldconfig
endef
else

define $(package)_stage_cmds
  bash ./install.sh --destdir=$($(package)_staging_dir) --prefix=$(host_prefix)/native --disable-ldconfig
endef
endif
