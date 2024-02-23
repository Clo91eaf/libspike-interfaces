final: prev:

{
  libspike = final.callPackage ./pkgs/libspike.nix { };
  libspike-interfaces = final.callPackage ./pkgs/libspike-interfaces.nix { };
  elfloader = final.callPackage ./examples/elfloader.nix { };
  rs-elfloader = final.callPackage ./examples/rs-elfloader.nix { };
}