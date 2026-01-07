{
  description = "A demo of monitoring a swarm of processes with libbpf";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-25.05";
    utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, utils, }:
    utils.lib.eachSystem [
      "x86_64-linux"
      "i686-linux"
      "aarch64-linux"
      "x86_64-darwin"
      "aarch64-darwin"
    ] (system:
      let
        # packages for the given system
        pkgs = import nixpkgs {
          inherit system;
          overlays = [ ];
        };
      in {
        # on `nix develop`
        devShells.default = pkgs.mkShell rec {
          nativeBuildInputs = [
            pkgs.gcc13
            pkgs.llvmPackages_20.clang-tools
            pkgs.llvmPackages_20.clang-unwrapped
            pkgs.gemini-cli
            pkgs.libbpf
            pkgs.bpftools
            pkgs.libsystemtap
            pkgs.linuxHeaders
            pkgs.elfutils
          ];

          buildInputs = [
          ];

          shellHook = ''
            export PS1="(dev) $PS1"
          '';
        };
      });
}
