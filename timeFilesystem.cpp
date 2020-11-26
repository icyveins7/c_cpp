#include <iostream>
#include <chrono>

int main(int argc, char *argv[])
{
	int totalBytes = 100000000;
	int bytesPerFile = 1000000;
	if (argc != 3)
	{
		printf("Using default values\n");
	}
	else
	{
		totalBytes = atoi((const char*)argv[1]);
		bytesPerFile = atoi((const char*)argv[2]);
	}

	int numFiles = totalBytes/bytesPerFile;
	
	printf("Total bytes = %d. Bytes per file = %d. Num files = %d\n", totalBytes, bytesPerFile, numFiles);
	
	
	// first we make the filenames so that the timing is pristine
	const int filenamelen = 16;
	char *filenames = (char*)malloc(sizeof(char)*filenamelen * numFiles);
	for (int i = 0; i < numFiles; i++)
	{
		snprintf(&filenames[i*filenamelen], filenamelen, "%d.bin", i);
		
		if (i < 5)
		{
			printf("Sample filename for %d : %6s\n", i, &filenames[i*filenamelen]);
		}
	}
	
	// ======== create all the files but with no data to time fopen/fcloses alone
	FILE *fp;
	auto t1 = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < numFiles; i++)
	{
		fp = fopen(&filenames[i*filenamelen], "wb");
		fclose(fp);
	}
	auto t2 = std::chrono::high_resolution_clock::now();
	auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
	printf("Time for fopen/fclose per file (average over %d files) : %.8f s.\n", numFiles, time_span.count() / numFiles);

	// cleanup
	for (int i = 0; i < numFiles; i++)
	{
		if (remove(&filenames[i*filenamelen]) != 0){
			printf("Failed to delete %s\n", &filenames[i*filenamelen]);
		}
	}
	
	// ======== now create files with the data
	char *data = (char*)malloc(sizeof(char)*bytesPerFile);
	auto t3 = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < numFiles; i++)
	{
		fp = fopen(&filenames[i*filenamelen], "wb");
		fwrite(data, sizeof(char), bytesPerFile, fp);
		fclose(fp);
	}
	auto t4 = std::chrono::high_resolution_clock::now();
	auto time_span_write = std::chrono::duration_cast<std::chrono::duration<double>>(t4 - t3);
	printf("Time for %f MB write per file (average over %d files) : %.8f s.\n", (double)bytesPerFile/1e6, numFiles, time_span_write.count() / numFiles);
	printf("fopen/fclose time took up %f%%\n", time_span.count()/time_span_write.count()*100.0);
	
	// cleanup
	auto t5 = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < numFiles; i++)
	{
		if (remove(&filenames[i*filenamelen]) != 0){
			printf("Failed to delete %s\n", &filenames[i*filenamelen]);
		}
	}
	auto t6 = std::chrono::high_resolution_clock::now();
	auto time_span_del = std::chrono::duration_cast<std::chrono::duration<double>>(t6 - t5);
	printf("Time to delete per file (average over %d files) : %.8f s.\n", numFiles, time_span_del.count()/numFiles);
	
	// final cleanup
	free(filenames);
	free(data);
	return 0;
}