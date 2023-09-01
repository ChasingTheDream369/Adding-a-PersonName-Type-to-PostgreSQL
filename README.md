# Adding-a-PersonName-Type-to-PostgreSQL

## Summary

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

# Adding a PersonName Type to PostgreSQL

![PostgreSQL Logo](images/postgresql_logo.png)

Welcome to the "Adding a PersonName Type to PostgreSQL" repository! This project extends PostgreSQL by introducing a new user-defined data type called `PersonName`. With this type, you can store and manipulate person names efficiently in your PostgreSQL database.

## Table of Contents
- [Introduction](#introduction)
- [Features](#features)
- [Getting Started](#getting-started)
- [Usage](#usage)
- [Contributing](#contributing)
- [License](#license)

## Introduction

![PersonName Type](images/person_name.png)

The `PersonName` type allows you to work with person names in a structured manner within your PostgreSQL database. Whether you're building an application that needs to store and query person names or conducting research that requires accurate name handling, this extension simplifies the process.

## Features

- **Structured Name Handling**: The `PersonName` type provides a structured way to store and manipulate names, including first names and last names.

- **Comparison Operators**: Easily compare `PersonName` values using standard comparison operators (`<`, `<=`, `=`, `>=`, `>`, `<>`).

- **Hashing Support**: Use hashing functions to efficiently index and search `PersonName` values.

- **Formatted Output**: Retrieve formatted name strings with the `show` function for display purposes.

## Getting Started

To get started with using the `PersonName` type in your PostgreSQL database, follow these steps:

1. Clone this repository to your local machine:

   ```
   git clone https://github.com/your-username/Adding-a-PersonName-Type-to-PostgreSQL.git
   ```
   
2. Build and install the extension:

   ```
   cd Adding-a-PersonName-Type-to-PostgreSQL
   make && make install
   ```

3. Create a new PostgreSQL database or use an existing one.

   Load the extension in your database:

   ```
   CREATE EXTENSION personname;
   ```
    
Now you're ready to use the PersonName type in your PostgreSQL database!

## Usage

Here's a quick example of how to use the PersonName type in your SQL queries:

```sql
-- Create a table with a PersonName column
CREATE TABLE people (
    id serial PRIMARY KEY,
    full_name PersonName
);

-- Insert some data
INSERT INTO people (full_name) VALUES
    ('John Doe'::PersonName),
    ('Alice Johnson'::PersonName);

-- Query for people with the last name 'Doe'
SELECT * FROM people WHERE full_name->'family' = 'Doe';

-- Output:
--  id | full_name
-- ----+-----------
--   1 | John Doe
```

For more detailed information on how to use this extension, please refer to the documentation of postgreSQL.

## Contributing

We welcome contributions from the community! If you'd like to contribute to this project, please check out our contribution guidelines.

## License

This project is licensed under the MIT License.

Happy PostgreSQL coding! 🐘
