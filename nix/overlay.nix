final: prev:

{
  libspike = final.callPackage ./pkgs/libspike.nix { };
  spike-interfaces = final.callPackage ./spike-interfaces { };
}