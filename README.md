Didactic implementation of callbacks as proposed by Rich Hickey in 1994. Here
is the paper link: <http://www.tutok.sk/fastgl/callback.html>. My
implementation differs from the proposed one because I wanted to experiment.


## Design choices

In order to try and understand some of the design choices he made, I compile
the project with `-std=c++98`. I am not sure it is enough, though. It looks
like either GCC and Clang don't enforce the standard properly or Rich Hickey is
using a pre-98 standard.


### Functor

The callback class is called `Callback` instead of `Functor`. What is usually
referred to as "functor" in C++ is really a "function object". "Functor" has a
different meaning in functional languages.

### Parametrization

`Callback` is parametrized on a function signature so that I can avoid having
different names of `Callback` depending on:

 * How many parameters the callback function has
 ```cpp
Functor0 // Not a template - nothing to parameterize
Functor1<P1>
Functor2<P1, P2>
// ...

// vs
Callback<void()>
Callback<void(P1)>
Callback<void(P1, P2)>
// ...
 ```

 * Whether the callback function returns or not:
 ```cpp
Functor0wRet<RT>
Functor1wRet<P1, RT>
Functor2wRet<P1, P2, RT>
// ...

// vs
Callback<RT()>
Callback<RT(P1)>
Callback<RT(P1, P2)>
// ...
 ```

(Again, as stated above, I am not sure this was supported at the time Rich
Hickey wrote his paper.)


### `makeFunctor`

In the discussion about the `makeFunctor` interface, he claims that:

> I must come clean at this point, and point out that the syntax above for
> `makeFunctor` is possible only in the proposed language, because it requires
> template members (specifically, the Functor constructors would have to be
> templates).
In the current language the same result can be achieved by
> passing to `makeFunctor` a dummy parameter of type
> ptr-to-the-Functor-type-you-want-to-create. [...] Simply cast `0` to provide
> this argument:
>
> ```cpp
> makeFunctor((Functor0 *)0, &wow);
> makeFunctor((Functor0 *)0, cd, &CDPlayer::play);
> ```

However, I am able to use constructor templates.

