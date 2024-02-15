final: prev:

{
  libspike = final.callPackage ./pkgs/libspike.nix { };
  libspike-interfaces = final.callPackage ./pkgs/libspike-interfaces.nix { };
  test = final.callPackage ./test { };
}