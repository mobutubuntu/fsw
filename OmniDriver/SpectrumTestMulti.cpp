// Program to continuosly read from the USB2000+ spectrometer aboard Cload Agronomics 
// Arial Measurement Unit. Follows the paradigm of a single main reader thread conducting 
// timme contiguous reads and dispatching writer threads to handle writing to a file 
// 
// this is going to be pretty heavy with some instrumentation jargon bullshit 
// so you might want to check out the OceanView Glossary but I will try and be as clear
// as possible ... https://oceanoptics.com/glossary/
// 
// Author: Jarod Boone 
#include <stdio.h>
#include <conio.h>
#include <iostream>
#include <fstream>
#include <exception>
#include <string>
// #include <type_traits>
// #include <typeinfo>
// #include <string.h>
#include <cstdlib>
#include <chrono>
#include <ctime>
#include <windows.h>
#include <direct.h>
#include <boost/thread.hpp> // C:\Program Files\boost_1_55_0\boost

// will need to include boost here at some point
#include "ArrayTypes.h"									
#include "Wrapper.h"

// break function to stop thread 
#define BREAK() do {   											\
		std::cout << "Press Enter to Continue" << std::endl; 	\
		std::cin.ignore();										\
	} while (0)		

// if defined will issue print statements to stdout 
#define DEBUG 1

// note that the usb2000+ supports 1ms - 65 s of integration time 
// in general integration time is porportional to noise so keep this low
#define INTEGRATION_TIME 500000 // microseconds of integration

// we want the USB2000+
#define DEV_TYPE "USB2+1"

// writer thread to put 
void writer(double *wlen, double *spec,size_t elements,SYSTEMTIME *tstamps,int count) { 
	// start the writer, this function should be called with a unique data and 
	// tstamp address that has been dynamically allocated prior to the function call
	#ifdef DEBUG 
		std::cout << "spawned a writer" << std::endl; 
	#endif 

	try { 
		// move into the data directory in the E: drive to write files 
		_chdir("E:\\USB2000_data");
	} catch (std::exception const &e) { 
		std::cerr << "Could not switch into E drive: " << e.what() << std::endl;
		exit(EXIT_FAILURE);
	}
	 
	// create the data filename verbose
	std::ostringstream filename_ss;

	filename_ss << "USB2000_data" << tstamps[0].wYear << "_" << tstamps[0].wMonth \
		<< "_" << tstamps[0].wDay << "_read" << count << ".csv";


  	std::string filename = filename_ss.str();

	std::ofstream outfile(filename.c_str());
	outfile << "start time = " << tstamps[0].wHour << ":" << tstamps[0].wMinute \
		<< ":" << tstamps[0].wSecond << ":" << tstamps[0].wMilliseconds << "\n";
	outfile << "end time = " << tstamps[1].wHour << ":" << tstamps[1].wMinute \
		<< ":" << tstamps[1].wSecond << "." << tstamps[1].wMilliseconds << "\n";
	outfile << "wavelen,spectrum\n";

	for (int i = 0; i < elements; i++) { 
		outfile << wlen[i];
		outfile << ","; 
		outfile << spec[i];
		outfile << "\n";
	}

	outfile.close();

	// clean that joan 
	std::free(wlen); 
	std::free(spec); 
	std::free(tstamps); 

	return; 
}

// the main reading function 
int main() {	 
	// this is the number of CCD elements or pixels given by the USB2000+ we will retrieve this 
	// on the first read but it is a static value so we only should need it once 
	size_t CCD_elements;		

	// just a check variable, need to be >= 1 but should be == 1 								
	int dev_num;	

	// the index of the spectrometer that we want to use, will search the 
	// DEV_TYPE string for the type of spectrometer 
	int spectrometer_index = 0; // default to zero 

	DoubleArray spec_array;	 // pixel values from the CCD elements
	DoubleArray wlen_array;	 // wavelengths (in nanometers) corresponding to each CCD element

	// See OmniDriver Wrapper API 
	Wrapper wrapper;				

	// get the number of spectrometers attached to the wrapper 
	dev_num = wrapper.openAllSpectrometers();	

	#ifdef DEBUG 
		std::cout << "Found : " << dev_num << " ocean optics spectrometers" << std::endl; 
	#endif 

	// make sure we have an attached spectrometer see programming manual for options 
	if (dev_num <= 0) { 
		std::cerr << "FATAL: Unable to find any spectrometers. Please check connections" << std::endl; 
		#ifdef DEBUG
			BREAK(); // pause the terminal to give the user a chance to see the error output 
		#endif 
		exit(EXIT_FAILURE);
	}
							
	// determine the device index we want to read from 
	for (int i=0;  i< dev_num; ++i) {
		#ifdef DEBUG 
			std::cout << "index " << i << " --> serial: " << wrapper.getSerialNumber(i).getASCII() << std::endl;
		#endif

		// scan for the right device 
		if (strncmp(wrapper.getSerialNumber(i).getASCII(),DEV_TYPE,5) == 0) {
			spectrometer_index = i; 
			break; 
		}
	}

	// Sets the target device integration time 
	wrapper.setIntegrationTime(spectrometer_index,INTEGRATION_TIME);		
	#ifdef DEBUG 
		std::cout << "Integration time set to : " << INTEGRATION_TIME << std::endl;
	#endif

	// flag to set the wavelength array if we need it 
	bool setwlen = true; 
	int count = 0; 
	SYSTEMTIME *tstamps; 
	double *wavelengths, *spectrum;

	// main read loop 
	for (;;) { 
		if (setwlen) setwlen = false; 

		tstamps = (SYSTEMTIME *) std::malloc(sizeof(SYSTEMTIME) * 2);

		GetSystemTime(&tstamps[0]);
		spec_array = wrapper.getSpectrum(spectrometer_index);	
		GetSystemTime(&tstamps[1]);

		if (wrapper.getWrapperExtensions().isSpectrumValid(spectrometer_index)) {
			#ifdef DEBUG 	
				std::cout << "spectrometer read successful" << std::endl;
			#endif 
		} else { 
			std::cerr << "Spectrometer read unsuccessful" << std::endl;
			exit(EXIT_FAILURE);
		}
		
		wlen_array = wrapper.getWavelengths(spectrometer_index);
		CCD_elements = spec_array.getLength();

		wavelengths = (double *) std::malloc(sizeof(double) * CCD_elements); 
		spectrum = (double *) std::malloc(sizeof(double) * CCD_elements + 1000); 
		
		wavelengths = wlen_array.getDoubleValues();	// Sets a pointer to the values of the wavelength array 
		spectrum = spec_array.getDoubleValues();			// Sets a pointer to the values of the Spectrum array 

		// dispatch a writer thread with the data and timelogs 
		boost::thread wrt(writer, wavelengths, spectrum, CCD_elements,tstamps,count);

		// we will not be cleaning this thread up 
		wrt.detach();

		// increment data
		count++; 

		printf("Spectrum complete, press enter to exit the program...\n");
		// getchar();

	}
	
	return 0;
}