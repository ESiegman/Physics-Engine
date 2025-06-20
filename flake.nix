{
  description = "C/C++ development environment";

  inputs = {
    nixpkgs.url = "https://flakehub.com/f/NixOS/nixpkgs/0.1.*.tar.gz";
  
    imgui-docking = {
      url = "github:ocornut/imgui/docking";
      flake = false;
    };

    tinyobjloader = {
      url = "github:tinyobjloader/tinyobjloader";
      flake = false;
    };
  };
  outputs = { self, nixpkgs, imgui-docking, tinyobjloader }:
    let
      supportedSystems = [ "x86_64-linux" "aarch64-linux" ];
      forEachSupportedSystem = f:
        nixpkgs.lib.genAttrs supportedSystems (system:
          f {
            pkgs = import nixpkgs { inherit system; };
          });
    in
    {
      devShells = forEachSupportedSystem ({ pkgs }: {
        default = pkgs.mkShell {
          packages = with pkgs; [
            python3
            python312Packages.glad
            clang-tools
            xmake
            pkg-config
            glfw
            glew
            libGL
            xorg.libX11
            xorg.libxcb
            xorg.libXrandr
            xorg.libXinerama
            xorg.libXext
            xorg.libXi
            gdb
            glm
          ];

          shellHook = ''
            echo "Adding X11 packages to path..."
            export LD_LIBRARY_PATH="${pkgs.lib.makeLibraryPath [
              pkgs.xorg.libX11
              pkgs.xorg.libxcb
              pkgs.xorg.libXrandr
              pkgs.xorg.libXinerama
              pkgs.xorg.libXext
              pkgs.xorg.libXi
              pkgs.libGL
              pkgs.glew
            ]}:$LD_LIBRARY_PATH"
            echo "Checking if an Xwayland session is running..."
            pgrep -fa Xwayland
            mkdir -p external
            echo "Linking ImGui Docking branch to /external"
            ln -sfn ${imgui-docking} external/imgui
            echo "Linking tinyobjloader Docking branch to /external"
            ln -sfn ${tinyobjloader} external/tinyobjloader
            echo "Checking if GLAD source is generated..."
            if [ ! -d glad-generated ]; then
              echo "Generating GLAD source..."
              glad --profile=core --api="gl=4.6" --generator=c --out-path=glad-generated
              echo "GLAD source generated in glad-generated/"
            else
              echo "GLAD already generated."
            fi
            echo "Finished setting up development environment"
          '';
        };
      });
    };
}

