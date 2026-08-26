# Windowsでのビルド

このプロジェクトは、従来のVisual Studioプロジェクトに加えて、CMakeとMinGW-w64でもビルドできます。Visual Studio IDEは不要です。

## 必要なもの

- CMake 3.24以上
- Ninja
- MinGW-w64（GCC / G++。C++20対応版）
- Git（GLFWが未インストールの場合、初回構成時の取得に使用）

各コマンドをターミナルから実行できるよう、インストール先の `bin` ディレクトリを `PATH` に追加してください。

## 最短のビルド方法

リポジトリのルートで次を実行します。

```bat
build_windows_mingw.bat
```

または、CMakeを直接実行できます。

```bat
cmake --preset windows-mingw
cmake --build --preset windows-mingw
```

完成した実行ファイルは次の場所に生成されます。

```text
build\windows-mingw\bin\glCraft++.exe
```

実行時に必要な `assets` とMinGWランタイムDLLも、実行ファイルと同じディレクトリへ自動コピーされます。

## GLFWを自動取得しない場合

CMakeから見つかる場所へGLFW 3.3以上をインストールして、次のように構成します。

```bat
cmake -S . -B build\local -G Ninja ^
  -DCMAKE_BUILD_TYPE=Release ^
  -DGLCRAFT_FETCH_GLFW=OFF
cmake --build build\local
```

GLFWのCMakeパッケージが標準外の場所にある場合は、`CMAKE_PREFIX_PATH` でその場所を指定できます。

## Debugビルド

```bat
cmake -S . -B build\debug -G Ninja ^
  -DCMAKE_BUILD_TYPE=Debug ^
  -DCMAKE_C_COMPILER=gcc ^
  -DCMAKE_CXX_COMPILER=g++
cmake --build build\debug
```
