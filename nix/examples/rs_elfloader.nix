{ makeRustPlatform
, myRustToolchain
, libspike
, libspike_interfaces
}:
let
  myRustPlatform = makeRustPlatform {
    cargo = myRustToolchain;
    rustc = myRustToolchain;
  };

  self = myRustPlatform.buildRustPackage {
    pname = "rs_elfloader";
    version = "0.1.0";
    src = ../../examples/rs_elfloader;
    buildInputs = [ libspike libspike_interfaces ];
    cargoLock = {
      lockFile = ../../examples/rs_elfloader/Cargo.lock;
      outputHashes = {
        "fst-native-0.6.3" = "sha256-18g/euQQjS8euF+za7za9/0/esGkfZfioB1rlAt59S4=";
      };
    };
  };
in
self