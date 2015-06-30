/* -*- C++ -*- */
/* Contains the definitions for the SOCK datagram abstraction. */

/* Here's the simple-minded constructor. */

inline
SOCK_Dgram::SOCK_Dgram (void)
{
}



/* Send an N byte datagram to ADDR (connectionless version). */

inline ssize_t
SOCK_Dgram::send (const void *buf, size_t n, const Addr &addr, int flags) const
{
  ssize_t ret;
  int sockfd = this->get_handle();
  sockaddr *saddr = (sockaddr *) addr.get_addr ();

  /*sergey
  size_t   len	  = addr.get_size ();
  */
  socklen_t len	  = addr.get_size ();

  struct sockaddr_in *abc;

  abc = (sockaddr_in *)saddr;
  unsigned int ttt;
  ttt = *((unsigned int *)&(abc->sin_addr));

  printf("SOCK_Dgram::send 2, sockfd=%d\n",sockfd);
  printf("SOCK_Dgram::send 2, saddr.sin_family=%d\n",abc->sin_family);
  printf("SOCK_Dgram::send 2, saddr.sin_port=%d\n",abc->sin_port);
  printf("SOCK_Dgram::send 2, saddr.sin_addr=%u (0x%08x) (%u.%u.%u.%u)\n",
		 ttt,ttt,(ttt&0xFF),((ttt>>8)&0xFF),((ttt>>16)&0xFF),((ttt>>24)&0xFF));

  ret = ::sendto (sockfd, (const char *) buf, n, flags, 
		          (const struct sockaddr *) saddr, len);

  printf("SOCK_Dgram::send 2: sendto returns %d\n",ret);
  if(ret<0)
  {
    perror("SOCK_Dgram::send 2: sendto ");
  }

  return(ret);
}

/* Recv an n byte datagram to ADDR (connectionless version). */

inline ssize_t
SOCK_Dgram::recv (void *buf, size_t n, Addr &addr, int flags) const
{
  /*printf("SOCK_Dgram::recv 2\n"); we are here every second */

  sockaddr *saddr   = (sockaddr *) addr.get_addr ();
  int	   addr_len = addr.get_size ();

  ssize_t status = ::recvfrom (this->get_handle (), (char *) buf, n, flags,
                               (sockaddr *) saddr, (socklen_t *)&addr_len);
			       /*Sergey (sockaddr *) saddr, &addr_len);*/
  addr.set_size (addr_len);
  return status;
}
