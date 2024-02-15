{ stdenv, cmake, libspike }:

stdenv.mkDerivation rec {
  pname = "spike-interfaces";
  version = "0.1.0";
  
  src = ./.;

  nativeBuildInputs = [ cmake ];
  buildInputs = [ libspike ];

  cmakeFlags = [
    "-DENABLE_TESTING=OFF"
    "-DENABLE_INSTALL=ON"
  ];
}