# babygit

A toy version of git written in C.
## Features

- Repository initialization
- File staging
- Commits
- Branching and Merging
- Status
- Stash
- Checkout
- Reset
- Hashing (SHA-1)


## Installation

### Install using make (for testing and development purposes)

```bash
    git clone git@github.com:SavvyHex/babygit.git
    cd babygit
    make
```

This will create the `bin/` folder inside which the `babygit` executable will exist.

### Install using install.sh

```bash
    git clone git@github.com:SavvyHex/babygit.git
    cd babygit
    ./install.sh
```


## Usage/Examples

### init

After installation, you can initialize a `babygit` project using the following command.

```bash
    babygit init
```

### Staging

```bash
    babygit add .
```

### Committing Changes

```bash
    babygit commit "Initial Commit" "SavvyHex"
```
### Creating a Branch

```bash
    babygit branch testing
```

### Switching to Another Branch

```bash
    babygit checkout main
```

### Checking the Status

```bash
    babygit status
```

### Merging Branches

```bash
    babygit merge main
```

### Stashing Changes

```bash
    babygit stash
```

## License

[MIT](https://choosealicense.com/licenses/mit/)

