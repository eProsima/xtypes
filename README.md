# xtypes
Fast and lightweight C++11 header-only implementation of [OMG DDS-XTYPES](https://www.omg.org/spec/DDS-XTypes) standard.

## Getting Started
Given the following IDL,

```c++
struct Inner {
    long a;
};

struct Outer {
    long b;
    Inner c;
};
```

you can create the representative C++ code defining this IDL's types using *xtypes API*:

```c++
StructType inner("Inner");
inner.add_member("a", primitive_type<int32_t>());

StructType outer("Outer");
outer.add_member("b", primitive_type<int32_t>());
outer.add_member("c", inner);
```

or by parsing the IDL:
```c++
std::map<std::string, DynamicType::Ptr> from_idl = idl::parse(my_idl);
const StructType& inner = static_cast<const StructType&>(*from_idl.at("Inner"));
const StructType& outer = static_cast<const StructType&>(*from_idl.at("Outer"));
```

Once these types have been defined, you can instatiate them and access their data:
```c++
//create the DynamicData accordingly with the recently created "Outer" DynamicType
DynamicData data(outer);

//write value
data["c"]["a"] = 42;

// read value
int32_t my_value = data["c"]["a"];
```

## Why should you use *eProsima xtypes*?
- **OMG standard**: *eProsima xtypes* is based on the
  [DDS-XTYPES standard](https://www.omg.org/spec/DDS-XTypes/About-DDS-XTypes/) from the *OMG*.
- **C++11 API**: *eProsima xtypes* uses C++11 latest features, providing  an easy-to-use API.
- **Memory lightweight**: data instances use the same memory as types builts by the compiler.
  No memory penalty is introduced by using *eProsima xtypes* in relation to compiled types.
- **Fast**: Accesing to data members is swift and quick.
- **Header only library**: avoids the linking problems.
- **No external dependency**: *eProsima xtypes*'s only dependencies are from *std*.
- **Easy to use**: Compresive API and intuitive concepts.

## Build
*eprosima xtypes* is a header only library: in order to use it you simply have to copy the files located in the include folder into your project and include them.

For a better management and version control, we recommend to make use of the **CMake** project that xtypes offers.
Although there is no library generated by *xtypes*, you can do the linking with its target as following. This will enable the inclusion of *xtypes* headers:
```
target_link_libraries(${PROJECT_NAME} xtypes)
```

### Examples
To compile the examples located in the `examples/` folder, enable the `XTYPES_BUILD_EXAMPLES` cmake flag.
Supposing you are in a `build` folder inside of `xtypes` top folder, you can run the following to compile the examples.
```bash
$ cmake .. -DXTYPES_BUILD_EXAMPLES=ON
$ make
```

### Tests
*xtypes* uses [GTest](https://github.com/google/googletest) framework for testing.
The *CMake* project will download this internally if you compile with the `XTYPES_BUILD_TESTS` flag enabled.
Supposing you are in a `build` folder inside of `xtypes` top folder, you can run the following to compile the tests.
```bash
$ cmake .. -DXTYPES_BUILD_TESTS=ON
$ make
```

## API usage
*Examples can be found in [example folder](examples).*

The API is divided into two different and yet related conceps.
1. Type definition: classes and methods needed for your runtime type definition.
2. Data instance: set of values organized accordingly with its own type definition.

### Type definition
All types inherit from the base abstract type `DynamicType` as shown in the following diagram:
![](http://www.plantuml.com/plantuml/png/ZP5DoeD038RtEOKNy0OVedpT5WeANSTf60BZQ3EPWj335pSInVvidpppXibR9qNHF0Iu20-i_A1kdgWeyrG-g-8qHnpOBGWQxuKyAe_ndV8_Xa3kam6jIdPgnxjSW4O4PsjiO-6S5Vj0bXwvwpwEtXg7p-7wgzZIFLDqzDq4x9CAEhKNME7ktsPWOom_tk82fbHisllhAgWftfPQNm00)

#### PrimitiveType
Represents the system's basic types.
In order to create a `PrimitiveType`, a helper function must be used:
```c++
const DynamicType& t = primitive_type<T>();
```
with `T` being one of the following basic types:
`bool` `char` `wchar_t` `uint8_t` `int16_t` `uint16_t` `int32_t` `uint32_t` `int64_t` `uint64_t` `float` `double`
`long double`

#### Enumerated Type
EnumeratedTypes are a special kind of PrimitiveTypes. They are internally represented by a PrimitiveType, but only
allows a user-defined subset of values.

##### EnumerationType
Similar to C++ enum, enumerations are the most basic kind of EnumeratedTypes.
It can be bound to three different PrimitiveTypes, `uint8_t`, `uint16_t`, and `uint32_t`.
The possible values for the enumeration are defined adding them as identifiers.
The value can be explicitly specified or auto-assigned. Once an identifier is added, the next identifier's value must
be greater than the previous one. By default, the first added value is zero (0).

```c++
EnumerationType<uint32_t> my_enum("MyEnum");
my_enum.add_enumerator("A");                // The value of MyEnum::A is 0. The default initial value.
my_enum.add_enumerator("B", 10);            // The value of MyEnum::B is 10. Explicitely defined.
my_enum.add_enumerator("C");                // The value of MyEnum::C is 11. Implicitely assigned.

DynamicData enum_data(my_enum);             // DynamicData of type "MyEnum".
enum_data = my_enum.value("C");             // Assign to the data the value of MyEnum::C.

uint32_t value = enum_data;                 // Retrieve the data as its primitive type.
DynamicData enum_data2 = enum_data;         // Copy the DynamicData.
enum_data2 = static_cast<uint32_t>(10);     // Assign to the copy, a raw value from its primitive type.
```

Assign or retrieve enumeration data of a different primitive type isn't allowed,
so the user should cast the value by himself.

Assign a value that doesn't belongs to the enumeration isn't supported.

#### Collection Type
As pointed by the self-explanatory name, CollectionTypes provide a way to create the most various collections.
There are several collection types:

- `ArrayType`: fixed-size set of elements. Similar to *C-like* arrays.
- `SequenceType`: variable-size set of elements. Equivalent to *C++* `*std::vector`
- `StringType`: variable-size set of char-type elements. Similar to *C++* `std::string`
- `WStringType`: variable-size set of wchar-type elements. Similar to *C++* `std::wstring`

```c++
ArrayType a1(primitive_type<int32_t>(), 10); //size 10
ArrayType a2(structure, 10); //Array of structures (structure previously defined as StructType)
SequenceType s1(primitive<float>()); //unbounded sequence
SequenceType s2(primitive<float>(),30); //bounded sequence, max size will be 30
SequenceType s3(SequenceType(structure), 20); //bounded sequence of unbounded sequences of structures.
StringType str1; //unbounded string
StringType str2(50); //bounded string
WStringType wstr(); //unbounded wstring
size_t a1_bounds = a1.bounds(); // As a1 is an ArrayType, its bounds are equal to its size.
size_t s1_bounds = s1.bounds(); // As s1 is an unbounded sequence, its bounds are 0.
size_t s2_bounds = s2.bounds(); // As s2 is a bounded sequence, its bounds are 30.
size_t s3_bounds = s3.bounds(); // As s3 is a bounded sequence, its bounds are 20.
size_t str1_bounds = str1.bounds(); // As str1 is an unbounded string, its bounds are 0.
size_t str2_bounds = str2.bounds(); // As str2 is a bounded string, its bounds are 50.
```

#### StructType
Similarly to a *C-like struct*, a `StructType` represents an aggregation of members.
You can specify a `StructType` given the type name of the structure.
```c++
StructType my_struct("MyStruct");
```
Once the `StructType` has been declared, any number of members can be added.
```c++
my_struct.add_member(Member("m_a", primitive_type<int32_t>()));
my_struct.add_member(Member("m_b", StringType()));
my_struct.add_member(Member("m_c", primitive_type<double>().key().id(42))); //with annotations
my_struct.add_member("m_d", ArrayType(25)); //shortcut version
my_struct.add_member("m_e", other_struct)); //member of structs
my_struct.add_member("m_f", SequenceType(other_struct))); //member of sequence of structs
```
Note: once a `DynamicType` is added to an struct, a copy is performed.
This allows modifications to `DynamicType` to be performed without side effects.
It also and facilitates the user's memory management duties.

#### `is_compatible` function of `DynamicType`
Any pair of `DynamicType`s can be checked for their mutual compatibility.
```c++
TypeConsistency consistency = tested_type.is_compatible(other_type);
```
This line will evaluate consistency levels among the two types.
The returned `TypeConsistency` is going to be a subset of the following *QoS policies*:

- `NONE`: Unknown way to interpret both types as equivalents.
- `EQUALS`: The evaluation is analogous to an equal evaluation.
- `IGNORE_TYPE_SIGN`: the evaluation will be true independently of the sign.
- `IGNORE_TYPE_WIDTH`: the evaluation will be true if the width of the some primitive types are less or
  equals than the other type.
- `IGNORE_SEQUENCE_BOUNDS`: the evaluation will be true if the bounds of the some sequences are less or
  equals than the other type.
- `IGNORE_ARRAY_BOUNDS`: same as `IGNORE_SEQUENCE_BOUNDS` but for the case of arrays.
- `IGNORE_STRING_BOUNDS`: same as `IGNORE_SEQUENCE_BOUNDS` but for the case of string.
- `IGNORE_MEMBER_NAMES`: the evaluation will be true if the names of some members differs (but no the position).
- `IGNORE_MEMBERS`: the evaluation will be true if some members of `other_type` are ignored.

Note: `TypeConsistency` is an enum with `|` and `&` operators overrided to manage it as a set of QoS policies.

### Module
Modules are equivalent to C++ namespaces. They can store sets of StructType, Constants and other Modules
(as submodules).
They allow organizing types into different scopes, allowing scope solving when accessing the stored types.
```c++
Module root;
Module& submod_a = root.create_submodule("a");
Module& submod_b = root.create_submodule("b");
Module& submod_aa = submod_a.create_submodule("a");
root.structure(inner);
submod_aa.structure(outer);

std::cout << std::boolalpha;
std::cout << "Does a::a::OuterType exists?: " << root.has_structure("a::a::OuterType") << std::endl;   // true
std::cout << "Does ::InnerType exists?: " << root.has_structure("::InnerType") << std::endl;           // true
std::cout << "Does InnerType exists?: " << root.has_structure("InnerType") << std::endl;               // true
std::cout << "Does OuterType exists?: " << root.has_structure("OuterType") << std::endl;               // false

DynamicData module_data(root["a"]["a"].structure("OuterType")); // ::a::a::OuterType
module_data["om3"] = "This is a string.";
```
As can be seen in the example, a module allows the user to access their internal definitions in two ways:
Accessing directly using a scope name (`root.structure("a::a::OuterType")`), or navigating manually through the
inner modules (`root["a"]["a"].structure("OuterType")`).

### Data instance
#### Initialization
To instantiate a data, only is necessary a `DynamicType`:
```c++
DynamicData data(my_defined_type);
```

This line allocates all the memory necesary to hold the data defined by `my_defined_type`
and initializes their content to *0* or to the corresponding *default values*.
Please, note that the type must have a higher lifetime than the DynamicData,
since `DynamicData` only saves a reference to it.
Other ways to initalizate a `DynamicData` are respectively *copy* and *compatible copy*.

```c++
DynamicData data1(type1); //default initalization
DynamicData data2(data1); //copy initialization
DynamicData data3(data1, type2); //compatible copy initialization
```

The last line creates a compatible `DynamicData` with the values of `data1` that can be accessed being a `type2`.
To archieve this, `type2` must be compatible with `type1`.
This compatibility can be checked with `is_compatible` function.

#### Internal data access
Depending on the type, the `DynamicData` will behave in different ways.
The following methods are available when:
1. `DynamicData` represents a `PrimitiveType`, `StringType` or `WStringType` (of `int32_t` as example):
    ```c++
    data.value(42); //sets the value to 42
    data = 23; // Analogous to the one above, assignment from primitive, sets the value to 23
    int32_t value = data.value<int32_t>(); //read the value
    int32_t value = data; //By casting operator
    data = "Hello again!"; // set string value
    data = L"Hello again! \u263A"; // set string value
    ```
1. `DynamicData` represents an `AggregationType`
    ```c++
    data["member_name"] = 42; //set value 42 to the int member called "member_name"
    data[2] = 42; //set value 42 to the third int member.
    int32_t value = data["member_name"]; // get the value from member_name member.
    int32_t value = data[2]; // get the value from third member.
    data["member_name"].value(dynamic_data_representing_a_value);
    WritableDynamicDataRef ref = data["member_name"];
    size_t size = data.size(); //number of members
    for (ReadableDynamicDataRef::MemberPair&& elem : data.items()) // Iterate through its members.
    {
        if (elem.kind() == TypeKind::INT_32_TYPE)
        {
            std::cout << elem.type().name() << " " << elem.name() << ": " << elem.value<int32_t>() << std::endl;
        }
    }
    ```
1. `DynamicData` represents a `CollectionType`
    ```c++
    data[2] = 42; // set value 42 to position 2 of the collection.
    int32_t value = data[2]; // get value from position 2 of the collection
    data[2] = dynamic_data_representing_a_value;
    WritableDynamicDataRef ref = data[2]; //references to a DynamicData that represents a collection
    size_t size = data.size(); //size of collection
    for (WritableDynamicDataRef&& elem : data) // Iterate through its contents.
    {
        elem = 0;
    }
    ```
1. `DynamicData` represents a `StringType`.
    Same as `CollectionType` plus:
    ```c++
    data = "Hello again!"; // set string value
    const std::string& s1 = data.value<std::string>(); //read the string value
    const std::string& s2 = data.string(); // shortcut version for string
    for (WritableDynamicDataRef&& elem : data) // Iterate through its contents.
    {
        elem = 'A';
    }
    ```
1. `DynamicData` represents a `WStringType`.
    Same as `CollectionType` plus:
    ```c++
    data = L"Hello again! \u263A"; // set string value
    const std::wstring& s1 = data.value<std::wstring>(); //read the string value
    const std::wstring& s2 = data.wstring(); // shortcut version for string
    for (WritableDynamicDataRef&& elem : data) // Iterate through its contents.
    {
        elem = L'A';
    }
    ```
1. `DynamicData` represents a `SequenceType`.
    Same as `CollectionType` plus:
    ```c++
    data.push(42); // push back new value to the sequence.
    data.push(dynamic_data_representing_a_value);
    data.resize(20); //resize the vector (same behaviour as std::vector::resize())
    size_t max_size = data.bounds(); // Maximum size of the sequence
    for (WritableDynamicDataRef&& elem : data) // Iterate through its contents.
    {
        elem = 0;
    }
    ```

#### References to `DynamicData`
There are two ways to obtain a reference to a `DynamicData`:
1. `ReadableDynamicDataRef` for constant references.
2. `WritableDynamicDataRef` for mutable references.

A reference does not contain any value and only points to an already existing `DynamicData`, or to part of it.
You can obtain a reference by accessing data with `[]` operator or by calling `ref()` and `cref()`.
Depending on whether the reference comes from a `const DynamicData` or a `DynamicData`, a `ReadableDynamicDataRef`
or a `WritableDynamicDataRef` is returned.

#### `==` function of `DynamicData`
A `DynamicData` can be compared in depth with another one.
The type should be the same.

#### `for_each` function of `DynamicData`
This function provides an easy way to iterate the `DynamicData` tree.
`for_each` receive a visitor callback that will be called for each node of the tree.
```c++
data.for_each([&](const DynamicData::ReadableNode& node)
{
    switch(node.type().kind())
    {
        case TypeKind::STRUCTURE_TYPE:
        case TypeKind::SEQUENCE_TYPE:
        case TypeKind::ARRAY_TYPE:
        case TypeKind::STRING_TYPE:
        case TypeKind::UINT_32_TYPE:
        case TypeKind::FLOAT_32_TYPE:
        //...
    }
});
```
The `node` callback parameters can be either `ReadableNode` or `WritableNode`, depending on `DynamicData` mutability,
and provides the following methods for introspection
```c++
node.data() // for a view of the data
node.type() // related type
node.deep() // nested deep
node.parent() // parent node in the data tree
node.from_index() // index used when accessed from parent to this node
node.from_member() // member used when accessed from parent to this node
```
Note: `node.data()` will return a `ReadableDynamicDataRef` or a `WritableDynamicDataRef` depending of the node type.

In order to break the tree iteration, the user can throw a boolean value.

For example:
```c++
case TypeKind::FLOAT_128_TYPE: // The user app does not support 128 float values
    throw false; // Break the tree iteration
```
The boolean exception value will be returned by `for_each` function call.
If the visitor callback implementation does not call an exception, `for_each` function returns `true` by default.

#### Iterators of `DynamicData`

There exists two kinds of iterators:
- Collection iterators: Allows to iterate through collections in the same way of native C++11 types. They can be
  accessed from `ReadableDynamicDataRef::Iterator` (for read-only operations) or `WritableDynamicDataRef::Iterator`
  (for read-write operations).

  ```cpp
  ReadableDynamicDataRef::Iterator it = data.begin();
  WritableDynamicDataRef::Iterator wit = data.begin();
  for (ReadableDynamicDataRef&& elem : data) { [...] }
  for (WritableDynamicDataRef&& elem : data) { [...] }
  ```

- Aggregation iterator: Allows to iterate through members of an aggregation.
  They make use of an auxiliar class **MemberPair** to allow access both, data and member information such as name.
  They can be accessed from `ReadableDynamicDataRef::MemberIterator` (read-only) or
  `WritableDynamicDataRef::MemberIterator` (read-write), through the `items()` or `citems()` methods.

  ```cpp
  // Explicit request for ReadableDynamicDataRef using citems().
  ReadableDynamicDataRef::MemberIterator it = my_data.citems().begin();
  // Typically items() is enough
  WritableDynamicDataRef::MemberIterator it = my_data.items().begin();
  for (ReadableDynamicDataRef::MemberPair&& elem : my_data.items()) { [...] }
  for (WritableDynamicDataRef::MemberPair&& elem : my_data.items()) { [...] }
  ```

## Performance
The `DynamicData` instance uses the minimal allocations needed to store any data.
If its associated type only contains the following (nested) types: `PrimitiveType`, `StructType`, or `ArrayType`;
the memory required can be allocated with only **one allocation** during the creation phase of the `DynamicData`.
On the other hand, each inner `MutableCollectionType` will trigger a further allocation.

Let see an example:
```c++
StructType inner("Inner");
inner.add_member("m_int", primitive_type<int32_t>());
inner.add_member("m_float", primitive_type<float>());
inner.add_member("m_array", ArrayType(primitive_type<uint16_t>(), 4));

DynamicData data(inner);
```
Such data will be represented in memory as follows. Only one memory allocation is needed:
![](docs/inner-memory.png)

The next complex type:
```c++
StructType outer("Outer");
outer.add_member("m_inner", inner);
outer.add_member("m_array_inner", ArrayType(inner, 4));
outer.add_member("m_sequence_inner", SequenceType(inner, 4));
outer.add_member("m_string", StringType());

DynamicData data(outer);
```
In this case, two allocations will be needed: one for the sequence, and a second one for the string:
![](docs/outer-memory.png)

## Debugging DynamicData
As a `DynamicData` is fully built at runtime, no static checks can ensure its correct behaviour.
As an attempt to solve this, asserts have been placed accross various methods to avoid the overload of checks in
*release mode*.
We strongly recommend to compile in *debug mode* during developing phase: this will allow `xtypes` library to
perform all possible checks.
Following the same line of thoughts, in order to improve the performance the final product should be compiled in
*release mode*. In this case, no checks will be performed.

Reminder: `assert` function from `std` emit an *abort* signal.
If you need more information about why a concrete `assert` was reached, the *stacktrace* is really useful.
