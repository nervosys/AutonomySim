extern crate cbindgen;

use std::env;
use std::path::PathBuf;

fn main() {
    let crate_dir = env::var("CARGO_MANIFEST_DIR").unwrap();
    let output_file = PathBuf::from(&crate_dir)
        .join("include")
        .join("autonomysim.h");

    // Create include directory if it doesn't exist
    std::fs::create_dir_all(PathBuf::from(&crate_dir).join("include")).ok();

    cbindgen::Builder::new()
        .with_crate(crate_dir)
        .with_language(cbindgen::Language::C)
        .with_style(cbindgen::Style::Both)
        .with_include_guard("AUTONOMYSIM_H")
        .with_documentation(true)
        .with_pragma_once(true)
        .with_sys_include("stdint.h")
        .with_sys_include("stdbool.h")
        .with_line_length(100)
        .generate()
        .expect("Unable to generate bindings")
        .write_to_file(output_file);
}
