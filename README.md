# Rich presence support for MPD
Broadcasts status of what you're listening to on MPD via Discord's rich presence.

![example](/images/playing.png)
![example2](/images/playing_big.png)
![example3](/images/playing_big2.png)
![example4](/images/idle.png)
![example5](/images/paused.png)
![newline6](/images/playing_tiny.png)

## Command line args

Example: `mpd_discord_richpresence -h=211.111.111.112 -P=password -p=6606 --fork --no-idle`

| Paramater| Purpose  |
|--|--|
|`-h=ADDDRESS`|  the address where the MPD server (defaults to `127.0.0.1`)|
|`-p=PORT`|the port on which the target MPD server is listening  (defaults to `6600`)|
|`-P=PASSWORD`|the password to be sent after connection to the MPD server has been established in hopes of acquiring higher permissions. (default is empty, therfore no password sent.)|
|`--fork`|forks the process into the background.|
|`--no-idle`|Disables broadcasting of the idle state.|

## Compilation

### Dependencies
* pthread
* [discord-rpc](https://github.com/discordapp/discord-rpc)
* libmpdclient

A [build script](build.sh) is included.

The CMakeFile will take care of finding discord-rpc. If it cannot find it, it will pull the discordrpc github repo and compile from source.


## Similar

* [mpv-discordRPC, noaione](https://github.com/noaione/mpv-discordRPC) - MPV
* [foo_discord, NaamloosDT/](https://github.com/NaamloosDT/foo_discord) - foobar2000 rich presence

