#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
	int fr = open(argv[1], O_RDONLY);
	int fw = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);

	unsigned char buf[1024];
	unsigned char buf2[2048];
	int bytesread = 0;
	int counter = 0;
	int tracker = 0;

	while ((bytesread = read(fr, buf, sizeof(buf))) > 0)
	{
		for (int i = 0; i < bytesread; i++)
		{
			if (tracker == 2 && buf[i] != '\n')
			{
				tracker = 0;
				buf2[counter] = ' ';
				counter++;
			}
			buf2[counter] = buf[i];
			counter++;
			tracker++;
		}

		write(fw, buf2, counter);
	}

	close(fr);
	close(fw);
}
