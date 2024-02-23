{
  description = "A project for building spike interfaces and tests.";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
    rust-overlay.url = "github:oxalica/rust-overlay";
  };

  outputs = { self, nixpkgs, flake-utils, rust-overlay }@inputs:
    let
      overlay = import ./nix/overlay.nix;
    in
    flake-utils.lib.eachDefaultSystem (system:
      let
        overlays = [ (import rust-overlay) overlay ];
        pkgs = import nixpkgs { inherit system overlays; };
        rs-toolchain = pkgs.rust-bin.stable.latest.default.override {
          extensions = [ "rust-src" ];
        };
      in
      {
        legacyPackages = pkgs;
        formatter = pkgs.nixpkgs-fmt;
        devShells.default = pkgs.mkShell {
          buildInputs = [
            # Including latest cargo,clippy,cargo-fmt
            rs-toolchain
            # rust-analyzer comes from nixpkgs toolchain, I want the unwrapped version
            pkgs.rust-analyzer-unwrapped
          ];

          # To make rust-analyzer work correctly (The path prefix issue)
          RUST_SRC_PATH = "${rs-toolchain}/lib/rustlib/src/rust/library";
        };
      })
  // { inherit inputs; overlays.default = overlay; };
}