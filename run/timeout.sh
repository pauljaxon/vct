# Execute a command with a timeout

# Author:
#    http://www.pixelbeat.org/
# Notes:
#    If the timeout occurs the exit status is 128.
#    There is an asynchronous (and buggy) equivalent of this
#    script packaged with bash (under /usr/share/doc/ in my distro),
#    which I only noticed after writing this.
#    I noticed later again that there is a C equivalent of this packaged
#    with satan by Wietse Venema, and copied to forensics by Dan Farmer.
# Changes:
#    V1.0, Nov  3 2006, Initial release
#    V1.1, Nov 20 2007, Brad Greenlee <brad@footle.org>
#                       Make more portable by using the 'CHLD'
#                       signal spec rather than 17.

if [ "$#" -lt "2" ]; then
    echo "Usage:   `basename $0` timeout_in_seconds command" >&2
    echo "Example: `basename $0` 2 sleep 3 || echo timeout" >&2
    exit 1
fi

cleanup()
{
    kill %1 &>/dev/null             #kill sleep $timeout if running
#   kill %2 2>/dev/null && exit 128 #kill monitored job if running
    kill %2 2>/dev/null && echo timeout && exit 128 
}

set -m               #enable job control
trap "cleanup" CHLD  #cleanup after timeout or command
timeout=$1 && shift  #first param is timeout in seconds
sleep $timeout&      #start the timeout
# sleep $timeout &> /dev/null &   #start the timeout, swallow any output on kill
"$@"                 #start the job

