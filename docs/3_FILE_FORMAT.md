# File Format

`.bbs` files use a small tree-based text format.

This guide explains the syntax only.
The meaning of specific fields such as `targets(...)`, `repo(...)`, or `builddir(...)` is covered in the config guides.

## The Core Idea

Everything in the format is one of these two shapes:

```txt
name(value)
name(
  child(value)
  child2(value)
)
```

You can think of it as a tree of named nodes.

## Basic Rules

- files contain items
- an item can be a scalar value or a named node
- named nodes use `name(...)`
- nested sections are built by placing items inside `(...)`
- indentation is only for readability
- commas are optional and ignored by the parser

## Scalar Types

The parser recognizes these scalar types.

### String

Strings use double quotes.

```txt
name("My App")
```

Supported escapes:

- `\n`
- `\r`
- `\t`
- `\\`
- `\"`

Use strings when:

- the value contains spaces
- the value contains special characters
- you want escaping behavior

### Identifier

Bare words become identifiers.

```txt
id(my_app)
lang(c)
runtime(static)
```

Identifiers are commonly used for symbolic values such as names, modes, or enum-like options.

### Integer

```txt
stack_size(1048576)
auto_retry_count(3)
```

The parser accepts signed integers, but individual config fields may later reject negative values.

### Float

```txt
scale(1.25)
```

The parser supports floats even though most current `bbs` config fields do not use them.

### Version

Versions are dot-separated numeric values with at least three parts.

```txt
ver(1.2.3)
ver(1.2.3.4)
```

### Boolean

Accepted boolean spellings are:

- `true`
- `false`
- `on`
- `off`

Example:

```txt
warnings_as_errors(true)
archive(on)
```

## Sections

If a named node contains multiple child items, it is a section.

Example:

```txt
license(
  type("MIT")
  file("LICENSE.md")
)
```

Sections can contain:

- named scalar fields
- nested sections
- repeated named entries
- bare scalar list items

## Lists

There is no separate list syntax.
Lists are just sections that contain scalar child items.

Example:

```txt
configs(
  debug
  release
)

units(
  src/main.c
  src/app.c
)
```

Many list-valued fields also accept a single scalar form:

```txt
units(src/main.c)
```

## Repeated Entries

Some sections intentionally use repeated named items.

Example:

```txt
dist(
  precommand("echo preparing")
  precommand("cp README.md out/")
)
```

Whether repeated entries are meaningful depends on the specific section definition.

## Optional Top-Level Wrappers

Some `bbs` files accept either a direct top-level form or a wrapper node.

Project file, direct form:

```txt
id(my_app)
targets(
  console(
    units(src/main.c)
  )
)
```

Project file, wrapped form:

```txt
project(
  id(my_app)
  targets(
    console(
      units(src/main.c)
    )
  )
)
```

`user.bbs` also accepts an optional `user(...)` wrapper.

## Comments

Two comment styles are supported:

```txt
# shell-style comment
// c++-style comment
```

Comments run to the end of the line.

## Whitespace And Formatting

Whitespace is not semantic as long as the syntax stays valid.

This is valid:

```txt
id(my_app) name("My App") ver(0.1.0)
```

But the recommended style is one item per line with indentation for nested sections.

Recommended style:

```txt
id(my_app)
name("My App")
ver(0.1.0)

targets(
  console(
    output(my_app)
    units(
      src/main.c
    )
  )
)
```

## How Parsing And Validation Differ

It helps to separate two steps:

1. parsing: can the text be read as valid `.bbs` syntax?
2. validation: does the parsed tree match what the specific config file expects?

A file can be syntactically valid and still be rejected later.

Example:

- `warning_level(hello)` is valid syntax
- but it is invalid project config because `hello` is not a supported warning level

## Common Parse Errors

Typical syntax errors are:

- missing `)`
- unexpected `(`
- unexpected `)`
- unterminated string

## Practical Reading Path

After understanding the syntax here, continue with:

- [5_USER_CONFIG.md](./5_USER_CONFIG.md)
- [6_PROJECT_CONFIG.md](./6_PROJECT_CONFIG.md)
- [4_EXAMPLES.md](./4_EXAMPLES.md)
