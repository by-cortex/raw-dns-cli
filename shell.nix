{
  pkgs ? import <nixpkgs> { },
}:
pkgs.mkShell {
  buildInputs = with pkgs; [
    gcc
    gnumake
    bear
    gdb
    valgrind
    bind # dig
    tcpdump
  ];

  shellHook = ''
    echo "Nix shell done."
  '';
}
