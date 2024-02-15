final: prev:

{
  libspike = final.callPackage ./pkgs/libspike.nix { };
  test = final.callPackage ./test { };
}