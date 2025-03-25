# libFAUDES - A Discrete-Event Systems Library



## Overview


libFAUDES is a C++ library that implements data structures for the representation 
of finite automata and algorithms for supervisory control within the framework 
originally proposed by P.J. Ramadge and W.M. Wonham. The library provides an 
implementation and evaluation basis for the further development of 
algorithms for the analysis and synthesis of DESs.

The core library sources are provided for free under conditions of the GNU Lesser 
General Public License.  Different terms may apply to libFAUDES plug-ins.

## Usage
This library is used for the implementation of Changming Yang's Master Thesis 
on the topic "Effective Control synthesis of Omega-language under partial Observation",
With the correct environment, you can easily compile with 
```bash
make dist-clean
make configure
make -j20
make -j20 tutorial 
```
The implemented files are in the folder .../libfaudes/plugins/synthesis
at present you can go to the folder /tutorial , and run
`./syn_8_ecpo`

## Documentation

libFAUDES ships with auto-generated HTML documentaion which can be
accessed via ./doc/index.html. An online version is available at

https://fgdes.tf.fau.de/faudes

For your convenience, precompiled binaries are distributed at

https://fgdes.tf.fau.de/download.html

If you want to build your own, see the build process documentation

https://fgdes.tf.fau.de/faudes/faudes_build.html



## Authors/Copyright

libFAUDES is distributed under terms of the LGPL v2.1. Over time, many students and colleguous have contributed to the code base; see  https://fgdes.tf.fau.de/faudes for a complete list.

- Copyright (C) 2006, Bernd Opitz.
- Copyright (C) 2008 - 2010, Thomas Moor, Klaus Schmidt, Sebastian Perk. 
- Copyright (C) 2011 - 2025, Thomas Moor, Klaus Schmidt

