# Adding-a-PersonName-Type-to-PostgreSQL

• Defined a new data type called PersonName for research purposes at UNSW to represent people’s name in the format
FamilyName, GivenNames and defined multiple operations on the type with also trying to concoct indexes on the same.

• Used the existing PostgreSQL code to add the new type PersonName type using a circular linked list type for the strings and
created their canonical functions pname_in(), pname_out() to get the input and output of the PersonName in PostgreSQL
which can be stored in the database as this representation.

• Defined 6 different operations on PersonName including =, <, <>, ≥, >, ≤ and also created a function family(PersonName)
which returns the familyName of the person, given() which returns the given name and show() which produces the entire
PersonName irrespective of how it is stored in the system.

• Developed a massive testing suite to check whether all types of operations on PersonName are working and whether we can
work with a new type in PostgreSQL. Overall used this as the basis to improve the research at UNSW and create random
data types in PostgreSQL system possibly trying to experiment on the same in the near future.
