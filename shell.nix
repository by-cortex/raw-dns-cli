{
  pkgs ? import <nixpkgs> { },
}:
pkgs.mkShell {
  buildInputs = with pkgs; [
    gcc
    gnumake
    gdb
    valgrind
    bind # dig
    tcpdump
  ];

  shellHook = ''
    echo "Nix shell done."
  '';
}
