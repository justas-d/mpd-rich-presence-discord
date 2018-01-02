# Rich presence support for MPD
Broadcasts status of what you're listening to on MPD via Discord's rich presence.

![example](/images/playing.png)
![example](/images/idle.png)
![example](/images/paused.png)
![newline](/images/mpd.png)

## Command line args

Example: `mpd_discord_richpresence -h=211.111.111.112 -P=password -p=6606 --fork`

* `-h=` - the address where the MPD server (defaults to `127.0.0.1`)
* `-p=` - the port on which the target MPD server is listening  (defaults to `6600`)
* `-P=` - the password to be sent after connection to the MPD server has been established in hopes of acquiring higher permissions. (default is empty, therfore no password sent.)
* `--fork` - forks the process into the background.

## Compilation

### Dependencies
* pthread
* [discord-rpc](https://github.com/discordapp/discord-rpc)
* libmpdclient

General consensus is that you'll want to link against discord-rpc, libmpdclient and pthread.

For the lazy: run build.sh. It will pull discord-rpc, build it and then run cmake to build and link mpd rich presence.

## Similar

* [foo_discord, NaamloosDT/](https://github.com/NaamloosDT/foo_discord) - foobar2000 rich presence
* [discordify, Krognol](https://github.com/Krognol/discordify) - spotift rich presence
