{ stdenv, cmake, libspike }:
let
  pname = "libspike-interfaces";
  version = "0.1.0";
  cmakeConfig = ''
    add_library(libspike-interfaces STATIC IMPORTED GLOBAL)
    set_target_properties(libspike-interfaces PROPERTIES
      IMPORTED_LOCATION "${placeholder "out"}/lib/libspike-interfaces.so")
    target_include_directories(libspike-interfaces AFTER INTERFACE
      "${placeholder "out"}/include"
    )
  '';
in
stdenv.mkDerivation rec {
  inherit pname version cmakeConfig;
  src = ../../src;
  nativeBuildInputs = [ cmake ];
  propagatedBuildInputs = [ libspike ];
  postInstall = ''
    mkdir -p $out/include $out/lib/cmake/libspike-interfaces
    cp $src/spike-interfaces.h $out/include
    echo "$cmakeConfig" > $out/lib/cmake/libspike-interfaces/libspike-interfaces-config.cmake
  '';
}