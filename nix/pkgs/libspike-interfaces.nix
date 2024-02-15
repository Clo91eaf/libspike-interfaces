{ stdenv, cmake, libspike }:

stdenv.mkDerivation {
  name = "libspike-interfaces";

  src = ../../lib;

  nativeBuildInputs = [
    cmake
  ];

  buildInputs = [
    libspike
  ];
}