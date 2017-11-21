# Rich presence support for MPD
Broadcasts status of what you're listening to on MPD via Discord's rich presence.

![exmaple](/images/tiny.png)

## Command line args

Example: `mpd_discord_richpresence -h=211.111.111.112 -P=password -p=6606`

* `-h=` - the address where the MPD server (defaults to `127.0.0.1`)
* `-p=` - the port on which the target MPD server is listening  (defaults to `6600`)
* `-P=` - the password to be sent after connection to the MPD server has been established in hopes of acquiring higher permissions. (default is empty, therfore no password sent.)

## Building

Currently, the project is only designed to be build using CLion. Build scripts are on the way.

### Dependencies
* pthread
* [discord-rpc](https://github.com/discordapp/discord-rpc)
* libmpdclient

## More Screenshots

![exmaple](/images/idle.png)
![exmaple](/images/paused.png)
![exmaple](/images/mpd.png)

## Similar

* [foo_discord, NaamloosDT/](https://github.com/NaamloosDT/foo_discord) - foobar2000 rich presence
