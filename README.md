TEXture PROcessing MAchine
===

TEXPROMA is a stack machine. All operators pop their arguments and push their results onto the stack.

Image generators
---

* `noise : (a : integer, b : integer) ↦ mono-buf`
* `light : (a : integer, b : float) ↦ mono-buf`
* `plasma : () ↦ mono-buf`
* `perlin-noise : (a : integer) ↦ mono-buf`
* `sine : (a : float) ↦ (mono-buf)`

  Generates vertical sine wave of amplitude `a` (range [-1.0, 1.0]).


Buffer operators
---

* `rotate : @mono-buf ↦ @mono-buf`

  Rotates buffer clock-wise by 90 degrees.

* `flip : @mono-buf ↦ @mono-buf`

  Flips buffer vertically.

* `move : (@mono-buf, float, float) ↦ (@mono-buf)`
* `explode : color-buf ↦ (mono-buf, mono-buf, mono-buf)`
* `implode : (mono-buf, mono-buf, mono-buf) ↦ color-buf`
* `insert : (color-buf, mono-buf, integer) ↦ color-buf`
* `extract : (color-buf, c : integer) ↦ (color-buf, mono-buf)`

  Extracts component `c` (for R : 0, G : 1, B : 2) from color buffer.

* `repeat : (@mono-buf, integer, integer) ↦ (@mono-buf)`

Pixel operators
---

* `add : (@mono-buf, @mono-buf) ↦ (@mono-buf)`
* `mul : (@mono-buf, @mono-buf) ↦ (@mono-buf)`
* `max : (@mono-buf, @mono-buf) ↦ (@mono-buf)`
* `mix : (@mono-buf, @mono-buf, integer) ↦ (@mono-buf)`
* `invert : (@mono-buf) ↦ (@mono-buf)`
* `mix-map : (mono-buf, mono-buf, mono-buf) ↦ (mono-buf)`
* `shade : (mono-buf, mono-buf) ↦ (mono-buf)`

Color operators
---

* `sine-color : (@mono-buf, integer) ↦ (@mono-buf)`
* `hsv-modify : (color-buf, float, float) ↦ (color-buf)`
* `contrast : (@mono-buf, float) ↦ (@mono-buf)`
* `brightness : (@mono-buf, float) ↦ (@mono-buf)`
* `grayscale : color-buf ↦ mono-buf`

  Converts color buffer to mono buffer (aka grayscale).

* `colorize : (mono-buf, c1 : integer, c2 : integer) ↦ color-buf`

   Maps input image pixels onto color range between `c1` and `c2`. Colors are encoded as hex numbers `0xRRGGBB`.

Distortion operators
---

* `twist : (@mono-buf, float) ↦ (@mono-buf)`
* `distort : (@mono-buf, mono-buf, mono-buf, float, float) ↦ (@mono-buf)`

Filters
---

* `edges : (@mono-buf) ↦ (@mono-buf)`
* `emboss : (@mono-buf) ↦ (@mono-buf)`
* `sharpen : (@mono-buf) ↦ (@mono-buf)`
* `blur-3x3 : (@mono-buf) ↦ (@mono-buf)`
* `blur-5x5 : (@mono-buf) ↦ (@mono-buf)`
* `gaussian-3x3 : (@mono-buf) ↦ (@mono-buf)`
* `gaussian-5x5 : (@mono-buf) ↦ (@mono-buf)`
* `median-3x3 : (@mono-buf) ↦ (@mono-buf)`
* `median-5x5 : (@mono-buf) ↦ (@mono-buf)`

Input-output
---

* `save-mono : (mono-buf, string) ↦ ()`
* `save-color : (color-buf, string) ↦ ()`
