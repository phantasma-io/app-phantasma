# app-phantasma

# to test use the CLI:

    https://github.com/coranos/phantasma-js-hw

# nano s
    export BOLOS_SDK=/home/coranos/git/LedgerHQ/nanos-secure-sdk

# nano x
    export BOLOS_SDK=/home/coranos/nano-x/sdk-nanox-1.2.4-1.5

# app-phantasma
    phantasma ledger app

    transaction serialization:
    https://github.com/phantasma-io/PhantasmaSDK/blob/master/C%2B%2B/Libs/Blockchain/Transaction.h#L14

    address from public key
    https://github.com/phantasma-io/PhantasmaSDK/blob/master/C%2B%2B/Libs/Cryptography/Address.h#L147

    api docs:
    https://docs.phantasma.io/#api-getChain


## Prerequisite

Be sure to have your environment correctly set up (see [Getting Started](https://developers.ledger.com/docs/nano-app/introduction/)) and [ledgerblue](https://pypi.org/project/ledgerblue/) and installed.

```
BOLOS_ENV=/opt/bolos-devenv
```

and do the same with `BOLOS_SDK` environment variable

```
BOLOS_SDK=/opt/nanos-secure-sdk
```

## Compilation

```
make DEBUG=1  # compile optionally with PRINTF
make load     # load the app on the Nano using ledgerblue
```

## Documentation

High level documentation such as [APDU](docs/messaging/APDU.md), [commands](docs/messaging/COMMANDS.md) and [transaction serialization](docs/messaging/TRANSACTION.md) are included in developer documentation which can be generated with [doxygen](https://www.doxygen.nl)

```
doxygen .doxygen/Doxyfile
```

the process outputs HTML and LaTeX documentations in `docs/html` and `docs/latex` folders.

## Tests & Continuous Integration

The flow processed in [GitHub Actions](https://github.com/features/actions) is the following:

- Code formatting with [clang-format](http://clang.llvm.org/docs/ClangFormat.html)
- Compilation of the application for Ledger Nano S in [ledger-app-builder](https://github.com/LedgerHQ/ledger-app-builder)
- Unit tests of C functions with [cmocka](https://cmocka.org/) (see [unit-tests/](unit-tests/))
- End-to-end tests with [Speculos](https://github.com/LedgerHQ/speculos) emulator (see [tests/](tests/))
- Code coverage with [gcov](https://gcc.gnu.org/onlinedocs/gcc/Gcov.html)/[lcov](http://ltp.sourceforge.net/coverage/lcov.php) and upload to [codecov.io](https://about.codecov.io)
- Documentation generation with [doxygen](https://www.doxygen.nl)

It outputs 4 artifacts:

- `phantasma-app-debug` within output files of the compilation process in debug mode
- `speculos-log` within APDU command/response when executing end-to-end tests
- `code-coverage` within HTML details of code coverage
- `documentation` within HTML auto-generated documentation
