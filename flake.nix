{
  description = "A basic multiplatform flake";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
    # Search for the "ref" field of a nix package here!
    # https://lazamar.co.uk/nix-versions/
    # gcc_nixpkgs.url = "github:nixos/nixpkgs?ref=253272ce9f1d83dfcd80946e63ef7c1d6171ba0e";
  };

  outputs = {nixpkgs, ...}: let
    # System types to support.
    supportedSystems = ["x86_64-linux" "x86_64-darwin" "aarch64-linux" "aarch64-darwin"];

    # Helper function to generate an attrset '{ x86_64-linux = f "x86_64-linux"; ... }'.
    forAllSystems = nixpkgs.lib.genAttrs supportedSystems;

    # Nixpkgs instantiated for supported system types.
    nixpkgsFor = forAllSystems (system: import nixpkgs {inherit system;});
  in {
    devShells = forAllSystems (system: let
      pkgs = nixpkgsFor.${system};
    in {
      default = pkgs.mkShell {
        packages = [
          # pkgs.pkgsi686Linux.gcc
          pkgs.gcc-arm-embedded-13
          pkgs.pkgsCross.arm-embedded.dfu-util
          pkgs.xc
          pkgs.bear
        ];

        shellHook = ''
          export CC=arm-none-eabi-gcc
          export CXX=arm-none-eabi-g++
          export AR=arm-none-eabi-ar
          cd ./rodos/
          source setenvs.sh
          rodos-lib.sh cm4
        '';
      };
    });
  };
}
