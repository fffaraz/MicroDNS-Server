MicroDNS
========

This is a "one-trick pony" DNS server. This compact and simple DNS server does only one thing: Always return the same IP for any query sent to the DNS server. This IP is given as the first argument to the program. The second, optional, argument specifies the IP the server has; if not given, the DNS server will attempt to bind to all IPs the server running this program has.
For example, to have the program always give the IP 172.16.72.129 to any query, and listen on the IP 127.0.0.1, invoke the program thusly:

microdns 172.16.72.129 127.0.0.1

http://samiam.org/software/microdns.html


microdns: 
   This is a tiny DNS server that does only one thing: It
   always sends a given IPv4 IP to any and all queries sent to the server.
   The IP to send the user is given in the first argument; the second optional
   argument is the IP of this tiny DNS server.  If the second argument is not
   given, microdns binds to "0.0.0.0": All the IP addresses the server has.

   For example, to have micrdns always give the IP address 10.1.2.3 on the
   IP 127.0.0.1:

   microdns 10.1.2.3 127.0.0.1


microdns handles a problem people frequently ask on serverfault: “How can I set up a DNS server to always return the same IP in reply to any query?” The program takes one argument: The IP we return. This program binds to all IP addresses a given machine has on the DNS port (port 53).

