# Lupi Zsh Addons
## Simple addons for your zsh terminal

> ## Install on macOS

```bash
git clone https://github.com/0netervezer0/Lupi-Zsh-Addons
cd Lupi-Zsh-Addons
chmod u+x lupi
sudo mv lupi /usr/local/bin/
```
Ready! Now use it with command ```lupi```

You can delete Lupi-Zsh-Addons folder (repository folder) after installation.
> ## Usage
### Default commands
| Command | Description |
| ------------- | ------------- |
| help | Returns the command list |
| cache  | Returns the terminal cache size and asks for clean it |
| new | Starts a new terminal session |
| newc | Starts a new terminal session in current directory |
| space | Returns disk usage of home direactory |
### Calendar commands
| Command | Description |
| ------------- | ------------- |
| cal | Returns actual month calendar |
| cal [month]  | Returns choosen month calendar |
### Environment commands
| Command | Description |
| ------------- | ------------- |
| hist | Returns the command history of your terminal |
| rc view | Returns .zshrc file information |
| rc edit | Edit .zshrc file using nano |
### Script commands
| Command | Description |
| ------------- | ------------- |
| script create [name] [.sh file] | Сreates a script in "my scripts" folderthat can be used by `lupi [name]` and execute actions from the .sh file |
| script create [name] | Creates an empty script in "my scripts" folderthat can be used by `lupi [name]` |
| script edit [name] | Edit your script using nano |
| script delete [name ] | Delete selected script |
| script list | Returns a list of your scipts |
### Git command
| Command | Description |
| ------------- | ------------- |
| gitstat [username] | Returns simple GitHub statistic of any user |
