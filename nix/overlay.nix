final: prev:

{
  libspike = final.callPackage ./pkgs/libspike.nix { };
  libspike_interfaces = final.callPackage ./pkgs/libspike_interfaces.nix { };
  elfloader = final.callPackage ./examples/elfloader.nix { };
  myRustToolchain = final.rust-bin.stable.latest.default.override { extensions = [ "rust-src" ]; };
  rs_elfloader = final.callPackage ./examples/rs_elfloader.nix { };
}