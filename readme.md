# Prex Programming Language

**Prex** is a lightweight, compiled programming language designed for simplicity, readability, and practical use in areas like CLI tools, web development, and low-level systems. Built on top of **LLVM**, Prex supports multiple architectures and generates efficient machine code.

> Inspired by the elegance of Rust/C constructs and the simplicity of Go.

join our [discord](https://discord.gg/6nvBRGsyVm)!

---

## ✨ Features

* 🧰 **LLVM-Powered Compilation**: Targets multiple architectures out of the box
* 🔧 **Compiled**: Produces small, fast executables
* 🧠 **Minimal Syntax**: Easy to learn and understand
* 🖥️ **Multi-domain Use**: Ideal for CLI tools, WebAssembly, and embedded/low-level applications
* 📦 **Tooling**: Includes `prex` compiler and `ignis` CLI for managing projects
* 🔍 **Explicit Semantics**: Clear behavior with no hidden magic

---

## 🔤 Syntax Example — Simple Calculator

```prex
defun main() > i32 {
    i32 a;
    i32 b;
    str op;
    printf("type a: ");
    scanf("%d", &a);
    printf("type b: ");
    scanf("%d", &b);
    printf("+ - * / :");
    scanf("%s", op);

    if(op == "+"){
        printf("%d + %d = %d\n",a,b,a+b);
    }
    else if(op == "-"){
        printf("%d - %d = %d\n",a,b,a-b);
    }
    else if(op == "*"){
        printf("%d * %d = %d\n",a,b,a*b);
    }
    else if(op == "/"){
        printf("%d / %d = %d\n",a,b,a/b);
    } else {
        printf("wrongt\n");
        printf("you typed %s\n", op);
    }
    ret 0;
}
```

---

## 📦 Toolchain

* `prex` — The compiler for Prex source files (`.prx`), powered by LLVM
* `ignis` — Optional CLI for project creation, building, and running (like Cargo)

---

## 📁 File Structure

A minimal project looks like:

```
myapp/
├── prex.toml
├── src/
│   └── main.prx
```

The `prex.toml` defines metadata and build output path:

```toml
[package]
name = "myapp"
version = "0.1.0"

[build]
output = "output.elf"
```

---

## 🚀 Getting Started

1. Write `.prx` source code in a `src/` directory
2. Compile using the `prex` compiler:

```bash
prex src/main.prx
```

3. Or use the `ignis` tool:

```bash
ignis new hello
cd hello
ignis run
```

---

## 🧪 Status

Prex is **experimental** and under active development. Expect rapid changes and evolving features.

---

## 📚 Documentation

Coming soon! Planned sections:

* Language syntax
* Standard library 
* Interop with C and other languages
* Compilation targets (x86, ARM, WASM)

---

## 📝 License

Apache 2.0

