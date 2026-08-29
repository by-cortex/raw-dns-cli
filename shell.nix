{
  pkgs ? import <nixpkgs> { },
}:
pkgs.mkShell {
  buildInputs = with pkgs; [
    gcc
    python3
    gnumake
    clang-tools
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
