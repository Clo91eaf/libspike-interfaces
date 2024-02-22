{ stdenv, cmake, libspike-interfaces, libspike }:
let
  pname = "elfloader";
  version = "0.1.0";
in
stdenv.mkDerivation {
  inherit pname version;
  src = ../../tests;

  nativeBuildInputs = [ cmake ];
  buildInputs = [ libspike-interfaces libspike ];
}