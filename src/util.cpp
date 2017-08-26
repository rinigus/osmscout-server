#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#include <unistd.h>

#include <iostream>

bool port_free(int port)
{
  int sockfd;
  struct sockaddr_in serv_addr;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
    {
      std::cerr << "ERROR opening socket\n";
      return false;
    }

  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);

  bool port_free = false;
  if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
    port_free = true;

  close(sockfd);

  return port_free;
}


bool wait_till_port_is_free(int port)
{
  const size_t cycles = 10;
  for (size_t i=0; true || i < cycles; ++i)
    if (port_free(port))
      {
        return true;
      }
    else
      {
        std::cout << "Waiting for port: " << port << std::endl;
        sleep(1);
      }

  return false;
}
