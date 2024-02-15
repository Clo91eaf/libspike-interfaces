{ stdenv, cmake, libspike }:

stdenv.mkDerivation rec {
  pname = "test";
  version = "0.1.0";
  
  src = ../../src;

  nativeBuildInputs = [ cmake ];
  buildInputs = [ libspike ];
}