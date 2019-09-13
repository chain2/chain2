Sample init scripts and service configuration for chain2
==========================================================

Sample scripts and configuration files for systemd, Upstart and OpenRC
can be found in the contrib/init folder.

    contrib/init/chain2d.service:    systemd service unit configuration
    contrib/init/chain2d.openrc:     OpenRC compatible SysV style init script
    contrib/init/chain2d.openrcconf: OpenRC conf.d file
    contrib/init/chain2d.conf:       Upstart service configuration file
    contrib/init/chain2d.init:       CentOS compatible SysV style init script

1. Service User
---------------------------------

All three startup configurations assume the existence of a "bitcoin" user
and group.  They must be created before attempting to use these scripts.

2. Configuration
---------------------------------

At a bare minimum, chain2d requires that the rpcpassword setting be set
when running as a daemon.  If the configuration file does not exist or this
setting is not set, chain2d will shutdown promptly after startup.

This password does not have to be remembered or typed as it is mostly used
as a fixed token that chain2d and client programs read from the configuration
file, however it is recommended that a strong and secure password be used
as this password is security critical to securing the wallet should the
wallet be enabled.

If chain2d is run with "-daemon" flag, and no rpcpassword is set, it will
print a randomly generated suitable password to stderr.  You can also
generate one from the shell yourself like this:

bash -c 'tr -dc a-zA-Z0-9 < /dev/urandom | head -c32 && echo'

Once you have a password in hand, set rpcpassword= in /etc/bitcoin/bitcoin.conf

For an example configuration file that describes the configuration settings, 
see contrib/debian/examples/bitcoin.conf.

3. Paths
---------------------------------

All three configurations assume several paths that might need to be adjusted.

Binary:              /usr/bin/chain2d
Configuration file:  /etc/chain2/bitcoin.conf
Data directory:      /var/lib/chain2
PID file:            /var/run/chain2/chain2d.pid (OpenRC and Upstart)
                     /var/lib/chain2/chain2d.pid (systemd)
Lock file:           /var/lock/subsys/chain2d (CentOS)

The configuration file, PID directory (if applicable) and data directory
should all be owned by the bitcoin user and group.  It is advised for security
reasons to make the configuration file and data directory only readable by the
bitcoin user and group.  Access to bitcoin-cli and other chain2d rpc clients
can then be controlled by group membership.

4. Installing Service Configuration
-----------------------------------

4a) systemd

Installing this .service file consists of just copying it to
/usr/lib/systemd/system directory, followed by the command
"systemctl daemon-reload" in order to update running systemd configuration.

To test, run "systemctl start chain2d" and to enable for system startup run
"systemctl enable chain2d"

4b) OpenRC

Rename chain2d.openrc to chain2d and drop it in /etc/init.d.  Double
check ownership and permissions and make it executable.  Test it with
"/etc/init.d/chain2d start" and configure it to run on startup with
"rc-update add chain2d"

4c) Upstart (for Debian/Ubuntu based distributions)

Drop chain2d.conf in /etc/init.  Test by running "service chain2d start"
it will automatically start on reboot.

NOTE: This script is incompatible with CentOS 5 and Amazon Linux 2014 as they
use old versions of Upstart and do not supply the start-stop-daemon utility.

4d) CentOS

Copy chain2d.init to /etc/init.d/chain2d. Test by running "service chain2d start".

Using this script, you can adjust the path and flags to the chain2d program by 
setting the CHAIN2 and FLAGS environment variables in the file 
/etc/sysconfig/chain2d. You can also use the DAEMONOPTS environment variable here.

5. Auto-respawn
-----------------------------------

Auto respawning is currently only configured for Upstart and systemd.
Reasonable defaults have been chosen but YMMV.


