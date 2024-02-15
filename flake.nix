{
  description = "A project for building spike interfaces and tests.";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }@inputs:
    let
      overlay = import ./nix/overlay.nix;
    in
    flake-utils.lib.eachDefaultSystem (system:
      let
        overlays = [ overlay ];
        pkgs = import nixpkgs { inherit system overlays; };
      in
      {
        legacyPackages = pkgs;
        formatter = pkgs.nixpkgs-fmt;
      })
  // { inherit inputs; overlays.default = overlay; };
}