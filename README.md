# named_args

A header-only library providing named arguments for C++ 17.

---

## Usage

Declare a name by `named_arg`:

```cpp
named_arg(a)
named_arg(b)
named_arg(c)
```

Declare named parameters by `named_arg_list`. The method must have a parameter pack called `args`:

```cpp
template<typename ...Args>
void foo(Args&&... args)
{
    using named_args::optional;
    using named_args::required;

    named_arg_list
    (
        int&, a, required,
        int , b, optional,
        int , c, 0
    )

    // This will create 3 type aliases:
    //   using a_t = int&;
    //   using b_t = std::optional<int>;
    //   using c_t = int;
    //
    // and 3 variables:
    //   a_t a
    //   b_t b
    //   c_t c

    std::cout << "a: " << a << " b: ";
    
    if (b)
        std::cout << *b;
    else
        std::cout << '_';

    std::cout << " c: " << c << '\n';

    ++a;
}

```

Example:

```cpp
int main()
{
    using named_args::names::a;
    using named_args::names::b;
    using named_args::names::c;

    int val = 0;

    foo(a=val, b=4, c=5);
    foo(c=5, b=4, a=val);
    foo(a=val);

    // foo(b=0, c=1); 
    // ^ static assertion failed: missing required arg: 'a'.

    // foo(a=val, c=new int);
    // ^ static assertion failed: cannot convert the type of arg 'c' to 'int'.

    std::cout << "val: " << val << std::endl;

    return 0;
}

```

Outputs:

```
a: 0 b: 4 c: 5
a: 1 b: 4 c: 5
a: 2 b: _ c: 0
val: 3
```
