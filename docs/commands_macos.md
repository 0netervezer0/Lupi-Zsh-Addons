# Lupi Zsh Addons
> ## Usage on macOS

### Default commands
| Command | Description |
| ------------- | ------------- |
| help / -h | Returns the command list and aliases |
| version / -v | Prints the current version and repository URL |
| cache | Returns the terminal cache size and asks whether to clean it |
| new | Starts a new terminal session |
| newc | Starts a new terminal session in the current directory |
| space | Returns disk usage of the home directory |

### Environment commands
| Command | Description |
| ------------- | ------------- |
| rc view | Returns .zshrc file contents |
| rc edit | Edits .zshrc using nano |

### Script commands
| Command | Description |
| ------------- | ------------- |
| script / -s create / a [name] [.sh file] | Creates a copy of a shell script in `~/my scripts` and makes it callable via `lupi [name]` |
| script / -s create / a [name] | Creates an empty script template in `~/my scripts` with argument instructions |
| script / -s edit / e [name] | Opens an existing script in nano |
| script / -s rename / rn [old-name] [new-name] | Renames a script in `~/my scripts` |
| script / -s remove / rm [name] | Deletes a selected script |
| script / -s list / ls | Returns a list of all user scripts |

### Script arguments
Use placeholders in the script body:

```bash
#!/bin/bash
cd ~/blackbird
python3 blackbird.py --username $arg0$
```

Then run it as:

```bash
lupi myscript username
```

Rules:
- `$arg0$` is the first argument
- `$arg1$` is the second argument, and so on
- the number of passed arguments must exactly match the number of placeholders used in the script
- if the script expects 2 arguments and the user passes 3, the command exits with an error

### Short aliases
| Alias | Full command |
| ------------- | ------------- |
| -h | help |
| -v | version |
| -s | script |
| -s a | (script) create |
| -s e | (script) edit |
| -s rn | (script) rename |
| -s rm | (script) remove |
| -s ls | (script) list |

Repository: https://github.com/0netervezer0/Lupi-Zsh-Addons

