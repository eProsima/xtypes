## Divergences from standard (rationales)

### value-types instance of reference-types
This implementation of xtypes traits the DynamicType objects as value-types.
This could be created any copy (little copies) but gives to the user a more safety API.
For instance, future modifications of a DynamicType already in a Publisher/Subscriber do not broke the Publisher/Subscriber.
Also, the type definition is in configuration stage, where the performance is not significant.
RTI uses this architecture.

### LoanValue without lock
The internal implementation of DynamicTypes does not need to lokc/unlock the loan values.
This allow to simplify the API access of nested members in DynamicData

### Removed Annotation clases
Annotation concepts has been embebed into the places where they are necessary in a more intuitive way.

### Added some methods out of standard to ease the user live
* [] operator to access members and indexes.
* `for_each`
* `is_compatible`
* Iterators:
    - Collection iterators: DynamicData representing a CollectionType (StringType, WStringType, ArrayType and
      SequenceType) gives the user the expected behaviour of a C++11 iterable object.
    - Aggregation iterators: DynamicData representing an AggregationTYpe (StructType) allows to the user to iterate
      through its members using the method *items()*.
