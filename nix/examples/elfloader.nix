{ stdenv, cmake, libspike-interfaces, libspike }:
let
  pname = "elfloader";
  version = "0.1.0";
in
stdenv.mkDerivation {
  inherit pname version;
  src = ../../examples/elfloader;

  nativeBuildInputs = [ cmake ];
  buildInputs = [ libspike-interfaces libspike ];
}