/*! \mainpage GSM
 * Applicatioin that is supposed to run in background and auto-reply and save incoming text messages to GSM modem using at commands.<br>
 * Communication with serial interface can be a bit tricky and requires patience and much larger pauses for commands execution than suggested.
 * 
 */
 
 /**
 * @file serial.cpp
 *
 * @author Daniel Stodulka, dstodu@gmail.com
 *
 * @date 2015
 *
 * @brief Receive messages, send auto-replies
 *  
 * @see 
 * @see https://lvdmaaten.github.io/publications/papers/TR_Dimensionality_Reduction_Review_2009.pdf
 */


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

/**
 * @brief simple sms struct, text+number
 *
 */

struct sms 
{
	string text; /**< sms text. */
	string number; /**< sms recipient/sender number in +420/country equivalent format */
};

/**
 * @brief Port settings
 *
 * @param fd port descriptor
 * @param speed bit rate
 * @param parity bit parity
 */

int SetInterfaceAttribs (int fd, int speed, int parity)
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

	tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     
	tty.c_iflag &= ~IGNBRK; 
	tty.c_lflag = 0;        
	tty.c_oflag = 0;        
	tty.c_cc[VMIN]  = 0;    
	tty.c_cc[VTIME] = 5;    

	tty.c_iflag &= ~(IXON | IXOFF | IXANY); 

	tty.c_cflag |= (CLOCAL | CREAD);

	tty.c_cflag &= ~(PARENB | PARODD); 
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

/**
 * @brief Sets blocking mode - writing to port etc
 *
 * @param fd port descriptor
 * @param should_block 1/0 block port
 */

void SetBlocking (int fd, int should_block)
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


/**
 * @brief Reads sms
 *
 * Only AT command so messages can be read from port directly
 *
 * @param fd port descriptor
 */

void ReadSms(int fd)
{
	write (fd, "AT+CMGL=\"REC UNREAD\"\r", 21);

	usleep ((21+10 ) * 100000);
}
	
/**
 * @brief Loads messages from file
 *
 * @param filename
 */
	
std::vector<sms> GetMessages(string filename)
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

/**
 * @brief Saves messages
 *
 * @param filename 
 * @param content messages
 */

void SaveMessages(string filename, char* content)
{
	ofstream myfile;
	myfile.open(filename.c_str(), std::ofstream::out | std::ofstream::trunc);

	myfile << content;

	myfile.close();
}

/**
 * @brief Sends auto-replies
 *
 * @param fd port descriptor
 * @param messages messages to respond to
 */

void SendMessages(int fd,std::vector<sms> messages)
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
			write(fd,"OK!\x1A",14);				// custom auto-reply
		}
		else if(messages.at(i).text.compare("PORUCHA") == 0)
		{
			write(fd,"Not OK!\x1A",12);					// custom auto-reply
		}
		usleep(4000000);
	}
}

/**
 * @brief Example usage
 *
 * Sets up and opens port.<br>
 * In infite loop checks if any new messages were recieved, if yes auto-replies
 *
 */
	
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

	SetInterfaceAttribs (fd, B57600, 0); 

	SetBlocking (fd, 0); 
		
	write(fd,"AT+CMGF=1\r",10);
	usleep(1000000);
		
	cout << "Processing...\n";
	while(1)
	{              
		/* ----------------- READ UNREAD SMS ------------------*/
		usleep(1000000);
		tcflush(fd,TCIOFLUSH);
			
		ReadSms(fd);
			
		cout << "messages read\n";

		/* -------------------- READ PORT ---------------------*/								

		char content[1000];
		read(fd,content,sizeof content);
		cout << "port read\n";

		/* ------------------ SAVE TO FILE -----------------------*/

		SaveMessages("sms.txt",content);
		cout << "messages saved\n";

		/* ------------------ GET UNREAD MESSAGES ----------------------*/
	
		std::vector<sms> messages;
		messages = GetMessages("sms.txt");

		/* -------------------- PROCESS MESSAGES --------------------*/
		cout << "sending messages\n";
		SendMessages(fd,messages);	
		cout << "Waiting...\n";
		usleep(20000000);	
	}
	close(fd);
	return 0;
} 



