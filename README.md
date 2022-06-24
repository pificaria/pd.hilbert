# About
Hilbert transform for Pure Data. This external has two objects: complex.mul~ and
hilbert~. The first one implements complex multiplication, thus it has two pairs
of inlets representing each a complex number (real and imaginary parts), and two
outlets. The last one has one inlet which is then processed by a hilbert
transform outputting the real and imaginary parts to the object's two outlets.
The transform's implementation is given by Dario Mambro's
[fork](https://github.com/unevens/hiir) of Laurent De Soras' HIIR library.
