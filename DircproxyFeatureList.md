# Feature List #

  * Runs on console, as a daemon or from inetd.
  * Able to proxy many simultaneous users and IRC connections.
  * Uses IRC server passwords to authenticate.
  * Remains connected to server when you detach. To reattach you just use the IRC server password again, no special commands!
  * Completely non-blocking throughout.
  * Can connect to servers that also require a password.
  * Can have a list of servers on the same network to connect to and it will cycle through this list.
  * Throttles data sent to server to ensure you are never kicked for flooding.
  * Can check servers to make sure they don't become "stoned".
  * Reconnect to servers if the connection is dropped.
  * Can automatically join channels for you on first attach.
  * Rejoins channels if you are kicked off.
  * Can leave channels when you detach and rejoin when you come back.
  * Can take measures to ensure you don't appear "idle" on IRC.
  * Host and password based security.
  * Easy to configure and get running.
  * Drop's unwanted user modes when you detach if you forget to de-opper yourself.
  * Will refuse to connect to servers that set certain modes such as +r.
  * Can bind to any IP on your host to change your appearance on IRC.
  * Can change what username is presented on IRC without affecting other users its proxying for.
  * Can send a message to all channels to indicate when you detach and reattach.
  * Can change your nickname when you detach.
  * Sets you /AWAY when you detach, if you forget to do so.
  * Fully configurable logging support so when you reattach you can see what you missed.
  * Can limit the size of log files to avoid eating disk space.
  * Log text recalled to your client is sent so your client sees it as ordinary IRC text.
  * Can time stamp text in log files so you know when it was sent and can also adjust the time stamp you see depending on how long ago it was sent.
  * Can make permanent log files for your own use of everything on channels or all private messages.
  * Can pass log text to a program of your choice (to send you an SMS for example).
  * Can adjust log time stamps depending on timezone difference between you and dircproxy.
  * Can proxy DCC chat and sends through itself.
  * Can capture DCC sends and store them on the dircproxy machine while you're detached and even while you're attached.
  * Captured DCC sends can be made subject to a size limit and have the sender's nickname included in the filename.
  * Can tunnel DCC sends and chats through ssh tunnels.  See the included source:/trunk/README.dcc-via-ssh#608 for more information.
  * Customizable message of the day for users which can include stats about log file sizes.
  * /DIRCPROXY command interface for users to do extra things with the proxy. Fully documented through /DIRCPROXY HELP command and the admin can enable and disable any command on a user-by-user basis.