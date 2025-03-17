- [Tiny Pointer Library](#tiny-pointer-library)
  - [Overview](#overview)
  - [Features](#features)
  - [Getting Started](#getting-started)
    - [Installation](#installation)
  - [Usage](#usage)
    - [Examples using Basic API Functions](#examples-using-basic-api-functions)
    - [Advanced Variants](#advanced-variants)
  - [Running Unit Tests](#running-unit-tests)
    - [Instructions](#instructions)
  - [Credits \& References](#credits--references)
  - [Contributing](#contributing)
  - [License](#license)

---

# Tiny Pointer Library

A production–quality C library implementing **Tiny Pointer Dereference Tables** – space–efficient data structures that compress pointer representations to dramatically reduce memory overhead. This library supports multiple variants of the tiny pointer concept:

- **Simple Variant:** A basic bucket–based dereference table with dynamic resizing.
- **Fixed–Size Variant:** Uses a primary/secondary sub–table strategy to return fixed-length tiny pointers.
- **Variable–Size Variant:** Employs a multi–level container that supports variable–length tiny pointers by dynamically adjusting pointer size based on table occupancy.

---


## Overview

Tiny pointers replace conventional pointers with a compressed representation, reducing memory overhead while still allowing constant–time operations. This library offers:
- Optimized bit–packing and bit–parallel operations.
- Cache–friendly layouts.
- Robust hash functions inspired by MurmurHash3.
- Thread–safety via POSIX mutexes.
- Dynamic resizing (supported for the Simple variant).
- Enhanced unit tests that cover all use cases and corner cases for each variant.

---

## Features

- **Space Efficiency:**
  Tiny pointers use significantly fewer bits than standard pointers.

- **Optimized Bit–Packing & Bit–Parallel Operations:**  
  Uses compiler–intrinsic functions (e.g. `__builtin_ctz`) for rapid free–slot lookup and efficient bit manipulation.

- **Cache–Friendly Layouts:**  
  Organizes data into contiguous buckets to maximize cache performance.

- **Polished Hash Functions:**  
  Implements a 32–bit hash inspired by MurmurHash3 to ensure robust key–mixing and low collision probability.

- **Thread–Safety:**  
  All operations are protected by POSIX mutexes, enabling safe concurrent use in multi–threaded applications.

- **Dynamic Resizing:**  
  The simple variant supports re–hashing and dynamic resizing to adjust to growing datasets.

- **Fine–Tuned Parameters:**  
  Parameters such as bucket size, load factor, and container levels can be configured both at compile–time (via macros) and at runtime.

- **Comprehensive Testing:** Three separate test suites (one per variant) cover:
  - Basic allocation, deallocation, and dereference operations.
  - Multiple allocations using the same key.
  - Allocation until full, then recovery after freeing.
  - Multi-threaded scenarios.
  - Reallocation after free.
  - Handling of null pointers and double free cases.

---

## Getting Started

### Installation

1. **Clone the repository:**
    ```bash
    git clone https://github.com/rodrigch18/TinyPointers.git
    cd TinyPointers
    ```

2. **Build the library:**

    A `makefile` is provided. 
    - To generate all variations of the library and the unified interface, run:
      ```makefile
      make all
      ```
    - To generate the simple lib variation, run:
      ```makefile
      make simple
      ```
    - To generate the fixed lib variation, run:
      ```makefile
      make fixed
      ```
    - To generate the variable lib variation, run:
      ```makefile
      make variable
      ```

## Usage

Include the header file in your project:
```c
#include "tiny_ptr.h"
```

### Examples using Basic API Functions

`**This section may be out of date**`

- Create a Table:
    ```c
    size_t capacity = 1024;
    double load_factor = 0.9; // For advanced variants (ignored for simple variant)
    tiny_ptr_table_t *table = tiny_ptr_create(capacity, TINY_PTR_SIMPLE, load_factor);
    if (!table) {
      // handle error
    }
    ```

- Allocate an Entry:
    ```c
    int key = 1234;
    int value = key * 10;
    int tiny_ptr = tiny_ptr_allocate(table, key, value);
    if (tiny_ptr == -1) {
        // Allocation failed.
    }
    ```

- Dereference an Entry:
    ```c
    int retrieved_value = tiny_ptr_dereference(table, key, tiny_ptr);
    if (retrieved_value == -1) {
        // Dereference error.
    }
    ```

- Free an Entry:
    ```c
    tiny_ptr_free(table, key, tiny_ptr);
    ```

- Resize the Table (Simple Variant Only):
    ```c
    if (tiny_ptr_resize(table, capacity * 2) != 0) {
        // Resizing failed.
    }
    ```

- Destroy the Table:
    ```c
    tiny_ptr_destroy(table);
    ```

### Advanced Variants

To use the Fixed or Variable variant, specify the corresponding `TinyPtrVariant` when creating the table:

```c
tiny_ptr_table_t *fixed_table = tiny_ptr_create(capacity, TINY_PTR_FIXED, load_factor);
tiny_ptr_table_t *variable_table = tiny_ptr_create(capacity, TINY_PTR_VARIABLE, load_factor);
```

Each variant uses a slightly different internal encoding for the tiny pointer (e.g. extra bits to indicate sub–table or level). The API remains the same, ensuring transparent use of the underlying data structure.

## Running Unit Tests

The repository includes an extensive suite of unit tests in the `tests` folder as three separate test executables, one per variant:

- test_tiny_ptr_simple:
  Covers all enhanced tests for the SIMPLE variant (including resize tests).

- test_tiny_ptr_fixed:
  Covers all enhanced tests for the FIXED variant (resizing is not supported).

- test_tiny_ptr_variable:
  Covers all enhanced tests for the VARIABLE variant (resizing is not supported).

### Instructions

**1. Compile the tests.**

  For example, if using CMake, add the Google Test dependency and compile this test file along with your library.

  Otherwise, Use the `make tests` after providing the google test framework files in the tests directory.

**2. Run the tests.**

  The tests will be run automatically using `make tests`.
  
  You can also execute the resulting test binaries in the `build` folder to run tests:

  ```bash
  ./test_simple
  ./test_fixed
  ./test_variable
  ```

---

This test suite verifies that all core functions of the Tiny Pointer Library work as expected across all supported variants, including:

- Single–threaded correctness for allocation, dereferencing, freeing, and resizing.
- Multi–threaded scenarios to ensure thread–safety and proper synchronization.
- Stress tests for different variants.

## Credits & References

This library is inspired by the research paper:

<p><strong>Tiny Pointers</strong><br>
<i>Michael A. Bender, Alex Conway, Martín Farach-Colton, William Kuszmaul, Guido Tagliavini</i><br>
ACM Trans. Algor. 2024<br>
<a href="[url](https://dl.acm.org/doi/10.1145/3700594)">DOI: 10.1145/3700594</a></p>

The original work introduces the concept of using compressed pointer representations (tiny pointers) to achieve near–optimal space efficiency in various data–structural applications. This library builds upon those ideas and provides a production–quality implementation.

## Contributing

Contributions, bug reports, and feature requests are welcome! Please open an issue or submit a pull request on [GitHub](https://github.com/rodrigch18/TinyPointers).

## License

This project is licensed under the MIT License. See the [LICENSE](https://github.com/rodrigch18/TinyPointers/blob/main/LICENSE) file for details.

---

**Happy coding, and enjoy building with Tiny Pointers!**
