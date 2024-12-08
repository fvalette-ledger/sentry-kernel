# SPDX-FileCopyrightText: 2023-2024 Ledger SAS
# SPDX-License-Identifier: Apache-2.0

if not with_uapi_opt
    subdir_done()
endif

add_languages('rust', native: false, required: true)

# Set rust toolchain config entry according to kconfig
# Note:
#   configuration_file is done at top level meson.build in order to generate
#   rust-toolchain.toml at build root directory.
rust_toolchain_in = files('rust-toolchain.toml.in')
rust_toolchain_config = configuration_data()
rust_toolchain_config.set('channel', get_option('rust-channel'))
rust_toolchain_config.set('target', kconfig_data.get_unquoted('CONFIG_RUSTC_TARGET'))

global_rust_build_args = [
    '@' + fs.parent(kconfig_rustargs) / fs.name(kconfig_rustargs),
    target_rustargs,
    '-C', 'lto=true', '-C', 'relocation-model=pic', '-C', 'link-args=--emit-relocs'
]

rust_edition = '2021'
uapi_rust_std = 'rust_std=' + rust_edition
uapi_manifest = files('Cargo.toml')

subdir('include/uapi')
subdir('src')

cargo = find_program('cargo', required: true)
cargo_fmt = find_program('cargo-fmt', required: false)
cargo_clippy = find_program('cargo-clippy', required: false)
cargo_index = find_program('cargo-index', required: false)

if with_doc_opt
uapi_doc = custom_target(
    'doc-uapi',
    input: uapi_manifest,
    output: 'doc',
    command: [cargo, 'doc', '--manifest-path', '@INPUT@'],
    depend_files: [uapi_modules, uapi_libfile],
    env: {'CARGO_TARGET_DIR':meson.current_build_dir()},
    install: true,
    install_dir: get_option('datadir') / 'doc/sentry-uapi',
    install_tag: 'doc',
)

uapi_doc_alias = alias_target('doc-uapi', uapi_doc)
endif # with_doc

# Following target are format/lint check and test
# These are declared as test entry in the `uapi` test suite

if with_tests
cargo_manifest_opt = '--manifest-path=' + uapi_manifest[0].full_path()
cargo_targetdir_opt = '--target-dir=' + meson.current_build_dir()
cargo_target_opt = '--target=' + target_rust

if cargo_fmt.found()
test(
    'cargo-fmt',
    cargo_fmt,
    args: [cargo_manifest_opt, '--check' ],
    should_fail: true,
    suite: 'uapi',
)
endif # cargo_fmt found

if cargo_clippy.found()
test(
    'cargo-clippy',
    cargo,
    args: [
        'clippy',
        cargo_manifest_opt,
        cargo_targetdir_opt,
        cargo_target_opt,
        '--no-deps',
        '--',
        '-Dwarnings',
    ],
    should_fail: true,
    suite: 'uapi',
)
endif # cargo clippy found

test(
    'cargo-test',
    cargo,
    args: [
        'test',
        cargo_manifest_opt,
        cargo_targetdir_opt,
        '--',
        '--test-threads=1',
    ],
    is_parallel: false,
    should_fail: true,
    suite: 'uapi',
)

endif #with_tests


install_data(
  'task.Kconfig',
  install_dir : get_option('datadir') / 'configs',
)

summary(
    {
        'rust edition': rust_edition,
    },
    section: 'Target specific compile args'
)
