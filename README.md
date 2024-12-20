ofxSCLang
=====================================


Introduction
------------
This addons wraps SuperCollider's libSCLang for OpenFrameworks to interpret sclang code.  
Note that this doesn't provide the audio server functions, it only implements an sclang interpreter client.  
An SCLang client can control scsynth and interpret the SuperCollider language. [SCLang advantages over OSC control](https://doc.sccode.org/Guides/ClientVsServer.html#A%20final%20remark%20for%20the%20advanced%20reader).

Installation
------------

1. Install  
  There are submodules which you need to **clone recursively**.
  - `cd path/to/openframeworks/addons`
  - `git clone  --recursive https://github.com/daandelange/ofxSCLang.git`
2. Compile SClang  
  An automated update and compile script is available.
  - `cd path/to/ofxSCLang/libs_submodules`
  - `./sync.sh`
3. After compiling your ofApp, you need to copy (or symlink) `ofxSCLang/data/SCClassLibrary` to `yourOfApp.app/Contents/Macos/SCClassLibrary`.

Compatibility
------------
Tested on osx 10.15 + of_v0.12.0 using either Qt-creator, Xcode or makefiles.  
Windows and Linux versions need some effort to get it running.

Known Issues
------------
- SClang's objects have a build-in limitation on stringifying objects for returning the post message.  
  This can be fixed by editing `SCClassLibrary/Common/Core/Object.sc`: Replace `asString { arg limit = 512;` by `asString { arg limit = 2048;`.

Related
-------
- [ofxSuperCollider](http://github.com/genekogan/ofxSuperCollider) : openframeworks addon for controlling SuperCollider via OSC.
- [ofxSuperColliderServer](https://github.com/satoruhiga/ofxSuperColliderServer) : Run a server directly within openframeworks.

Licenses
--------
Made by [Daan de Lange](https://daandelange.com/) with the help of [José Miguel Fernandez](https://github.com/Josemiguelfernandez).

- ofxSCLang : [MIT License](./LICENSE.md).
- supercollider : [MIT License](https://github.com/supercollider/supercollider/blob/develop/COPYING).

