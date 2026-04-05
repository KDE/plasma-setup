{
  description = "plasma-setup development build";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
        kdePackages = pkgs.kdePackages;
      in
      {
        packages.default = kdePackages.plasma-setup.overrideAttrs (old: {
          src = ./.;
          version = "unstable-${self.shortRev or "dirty"}";
        });

        devShells.default = pkgs.mkShell {
          inputsFrom = [ kdePackages.plasma-setup ];
          packages = with pkgs; [
            cmake
            ninja
            gdb
            qtcreator
          ];
        };
      });
}
