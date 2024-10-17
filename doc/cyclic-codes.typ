
#text([#emoji.fox Notes on Cyclic Codes], size: 24pt)

#v(0.25in)

= Definitions
Definitions from @hill.


#set par(justify: true)
// #show: rest => columns(2, rest)

/ Polynomial Degree: if $f(x) = f_0 + f_1x + ... + f_(m)x^m$ is a polynomial with $f_m != 0$, then $m$ is called the degree of $f(x)$.

/ Leading Coefficient: In the polynomial above, $f_m$ is the leading coefficient.

/ Monic: A polynomial is called monic if the leading coefficient is 1.

= Thoughts

In @bergamasco, they define the code for RUNE-43 as a vector in $(ZZ_2)^43$ (which already doesn't make much sense to me, since they say that a linear code is $FF_q$  where $q = p^k in NN$, $p$ is prime and $k > 1$). Anyways, it would seem to me that the codes for the dotframes could be $(ZZ_4)^20$, but then $n$ is not prime, and the claim they make:

#quote(block: true, attribution: [#cite(<bergamasco>, form: "author")])[... by choosing an $n$ prime we only have classes either composed by a single element (constant codewords with $n$ repititions of the same symbol) or where all codewords are distinct.]

would not hold. And the codewords being unique is required, or else your recognizer would not be able to tell rotations apart from codewords. That is, if $c_1, c_2 in C$ are codewords, then some cyclic shift of $c_1$ may end up being identical to $c_2$. RUNE-Tag relies on this not happening so that it can uniquely identify the tags in any rotation.

I have to say, though: I do really like the idea behind this: they seem really robust to occlusion, and perhaps combined with @degol would make a really robust tracking solution. It still doesn't really give me any clues about how RealTalk actually does this, though.

#v(1cm)
// #bibliography("bib.yml", style: "association-for-computing-machinery")
#bibliography("bib.yml")