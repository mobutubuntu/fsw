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
#include <type_traits>
#include <typeinfo>
#include <string.h>
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

#define TARGET_SERIAL 12

int main() {	 
	// this is the number of CCD elements or pixels given by the USB2000+ we will retrieve this 
	// on the first read but it is a static value so we only should need it once 
	int	CCD_elements;		

	// just a check variable, need to be >= 1 but should be == 1 								
	int dev_num;	

	DoubleArray spectrumArray;								// pixel values from the CCD elements
	DoubleArray wavelengthArray;							// wavelengths (in nanometers) corresponding to each CCD element

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
							
	#ifdef DEBUG
		for (int i=0;  i< dev_num; ++i) {
			std::cout << "index " << i << " --> serial: " << typeid(wrapper.getSerialNumber(i).getASCII()).name() << std::endl;
			if (strncmp(wrapper.getSerialNumber(i).getASCII(),"USB2+U10109",4) == 0) {
				std::cout << "lit" << std::endl; 
			}
		}
	#endif 	

	BREAK(); 
	return 0; 

	wrapper.setIntegrationTime(0,INTEGRATION_TIME);		// Sets the integration time of the first spectrometer to 100ms
	printf ("Integration time of the first spectrometer has been set to %d microseconds\n", INTEGRATION_TIME);
	printf ("Press enter to get the spectrum from this spectrometer...\n");
	getchar();
	spectrumArray = wrapper.getSpectrum(0);						// Retreives the spectrum from the first spectrometer
	if (wrapper.getWrapperExtensions().isSpectrumValid(0) == false) {
		printf("Error occured during spectrum acquisition\n");
		return 0;
	}
	if (wrapper.getWrapperExtensions().isSpectrumValid(0) == true) {
		printf("Spectrum acquisition was successful\n");
	}
	wavelengthArray = wrapper.getWavelengths(0);			    // Retreives the wavelengths of the first spectrometer 
	CCD_elements = spectrumArray.getLength();					// Sets CCD_elements to the length of the spectrumArray 
	double *wavelengths = wavelengthArray.getDoubleValues();	// Sets a pointer to the values of the wavelength array 
	double *spectrum = spectrumArray.getDoubleValues();			// Sets a pointer to the values of the Spectrum array 
	for(int i = 0; i < CCD_elements; i++){					// Loop to print the spectral data to the screen
		printf("Wavelength: %1.2f      Spectrum: %f \n", wavelengths[i], spectrum[i]); 
	}
	printf("Spectrum complete, press enter to exit the program...\n");
	getchar();
	return 0;
}