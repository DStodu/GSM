#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

	struct sms {
  		string text;
		string number;
	};

	/* ------------------ PORT SETTINGS START ---------------- */

	int set_interface_attribs (int fd, int speed, int parity)
	{
		    struct termios tty;
		    memset (&tty, 0, sizeof tty);
		    if (tcgetattr (fd, &tty) != 0)
		    {
		            printf ("error %d from tcgetattr", errno);
		            return -1;
		    }

		    cfsetospeed (&tty, speed);
		    cfsetispeed (&tty, speed);

		    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
		    tty.c_iflag &= ~IGNBRK;         // disable break processing
		    tty.c_lflag = 0;                // no signaling chars, no echo
		    tty.c_oflag = 0;                // no remapping, no delays
		    tty.c_cc[VMIN]  = 0;            // read doesn't block
		    tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

		    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

		    tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,

		    tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
		    tty.c_cflag |= parity;
		    tty.c_cflag &= ~CSTOPB;
		    tty.c_cflag &= ~CRTSCTS;

		    if (tcsetattr (fd, TCSANOW, &tty) != 0)
		    {
		            printf ("error %d from tcsetattr", errno);
		            return -1;
		    }
		    return 0;
	}

	void set_blocking (int fd, int should_block)
	{
		    struct termios tty;
		    memset (&tty, 0, sizeof tty);
		    if (tcgetattr (fd, &tty) != 0)
		    {
		            printf ("error %d from tggetattr", errno);
		            return;
		    }

		    tty.c_cc[VMIN]  = should_block ? 1 : 0;
		    tty.c_cc[VTIME] = 5;            

		    if (tcsetattr (fd, TCSANOW, &tty) != 0)
		            printf ("error %d setting term attributes", errno);
	}

	/* ------------------ PORT SETTINGS END ------------------------ */

	void read_sms(int fd)
	{
		write (fd, "AT+CMGL=\"REC UNREAD\"\r", 21);

		usleep ((21+10 ) * 100000);
	}
	
	std::vector<sms> get_messages(string filename)
	{
		fstream myfile;

		string line;
		int pos = 0;
		bool numb_set = false;
		int it = 0;

		std::vector<sms> messages;

  		myfile.open (filename.c_str());

		while ( getline (myfile,line) )
		{
			if(line.compare(0,1,"+") == 0 )
			{
				messages.push_back(sms());
				pos = line.find(",");
				pos = line.find(",",pos+1)+2;
				messages.at(it).number = line.substr(pos,13);
				numb_set = true;
			}
			else if(numb_set && int(line[0]) != 0)
			{
				messages.at(it).text = line;
				numb_set = false;
				it++;
			}
    	}

		myfile.close();		
		
		return messages;
	}

	void save_messages(string filename, char* content)
	{
		ofstream myfile;
		myfile.open(filename.c_str(), std::ofstream::out | std::ofstream::trunc);

		myfile << content;

		myfile.close();
	}

	void send_messages(int fd,std::vector<sms> messages)
	{
		string command;
		for(unsigned int i = 0; i < messages.size(); i++)
		{
			command = "AT+CMGS=\"" + messages.at(i).number + "\"\r";
			write(fd,command.c_str(),24);
			cout << command << std::endl;
			usleep(1000000);

			int text_length = messages.at(i).text.size();
			if ((int)messages.at(i).text.at(text_length-1) == 13)
			{
				messages.at(i).text.resize(text_length-1);
			}
			if(messages.at(i).text.compare("STAV") == 0)
			{				
				write(fd,"Vsechno bezi!\x1A",14);				
			}
			else if(messages.at(i).text.compare("PORUCHA") == 0)
			{
				write(fd,"Nic nebezi!\x1A",12);				
			}
			usleep(4000000);
		}
	}

/* --------------------------- MAIN ---------------------------*/
	
	int main()
	{

		/* ----------------- SET UP PORT -----------------------*/
		
		string portname = "/dev/ttyUSB0";
		
		int fd = open (portname.c_str(), O_RDWR | O_NOCTTY | O_SYNC | O_NONBLOCK);
		if (fd < 0)
		{
				cout << "error" << errno << "opening" << portname << ":" << strerror(errno) << std::endl;
				return -1;
		}

		set_interface_attribs (fd, B57600, 0); 

		set_blocking (fd, 0); 
		
		write(fd,"AT+CMGF=1\r",10);
		usleep(1000000);
		
		cout << "Processing...\n";
		while(1)
		{              
			/* ----------------- READ UNREAD SMS ------------------*/
			usleep(1000000);
			tcflush(fd,TCIOFLUSH);
			
			read_sms(fd); // AT COMMAND ONLY
			
			cout << "messages read\n";

			/* -------------------- READ PORT ---------------------*/								

			char content[1000];
			read(fd,content,sizeof content);
			cout << "port read\n";

			/* ------------------ SAVE TO FILE -----------------------*/

			save_messages("sms.txt",content);
			cout << "messages saved\n";

			/* ------------------ GET UNREAD MESSAGES ----------------------*/
	
			std::vector<sms> messages;
			messages = get_messages("sms.txt");

			/*for(unsigned int i=0; i < messages.size(); i++)
			{
				cout << messages.at(i).number << "\t" << messages.at(i).text << std::endl;
			}*/
		
			/* -------------------- PROCESS MESSAGES --------------------*/
			cout << "sending messages\n";
			send_messages(fd,messages);	
			cout << "Waiting...\n";
			usleep(20000000);	
		}
		close(fd);
		return 0;
	} 



