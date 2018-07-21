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
#include <boost/thread.hpp> // C:\Program Files\boost_1_55_0\boost

// will need to include boost here at some point
#include "ArrayTypes.h"									
#include "Wrapper.h"

// note that the usb2000+ supports 1ms - 65 s of integration time 
// in general integration time is porportional to noise so keep this low
#define INTEGRATION_TIME 500000 // microseconds of integration 

// const char* getCharFromString(JNIEnv* env, jstring string){
//     if(string == NULL)
//         return NULL;

//     return  env->GetStringUTFChars(string ,0);
// }

// // function to display the device state (stdout)
// void dev_state(Wrapper &obj) { 
// 	// sanity check 
// 	assert(obj != NULL); 

// 	std::cout << "number devices attached to wrapper" << obj.openAllSpectrometers() << std::endl; 

// }

int main() {	 
	JavaVM *jvm;                      // Pointer to the JVM (Java Virtual Machine)
	JNIEnv *env;                      // Pointer to native interface
		//================== prepare loading of Java VM ============================
	JavaVMInitArgs vm_args;                        // Initialization arguments
	JavaVMOption* options = new JavaVMOption[1];   // JVM invocation options
	options[0].optionString = "-Djava.class.path=.";   // where to find java .class
	vm_args.version = JNI_VERSION_1_6;             // minimum Java version
	vm_args.nOptions = 1;                          // number of options
	vm_args.options = options;
	vm_args.ignoreUnrecognized = false;     // invalid options make the JVM init fail
		//=============== load and initialize Java VM and JNI interface =============
	jint rc = JNI_CreateJavaVM(&jvm, (void**)&env, &vm_args);  // YES !!
	delete options;    // we then no longer need the initialisation options. 
	if (rc != JNI_OK) {
			// TO DO: error processing... 
			std::cin.get();
			exit(EXIT_FAILURE);
	}
		//=============== Display JVM version =======================================
	std::cout << "JVM load succeeded: Version ";
	jint ver = env->GetVersion();
	std::cout << ((ver>>16)&0x0f) << "."<<(ver&0x0f) << std::endl;

	// TO DO: add the code that will use JVM <============  (see next steps)

	jvm->DestroyJavaVM();
	std::cin.get();
	// this is the number of CCD elements or pixels given by the USB2000+ we will retrieve this 
	// on the first read but it is a static value so we only should need it once 
	int	CCD_elements;		

	// just a check variable, need to be >= 1 but should be == 1 								
	int dev_num;	

	DoubleArray spectrumArray;								// pixel values from the CCD elements
	DoubleArray wavelengthArray;							// wavelengths (in nanometers) corresponding to each CCD element

	// See OmniDriver Wrapper API 
	Wrapper wrapper;				

	dev_num = wrapper.openAllSpectrometers();	// Gets an array of spectrometer objects

	JString name = wrapper.getName(0); 
	// printf (name); 
	printf ("\n\nNumber of spectrometers found: %d\n", dev_num);
	printf ("Press enter to continue...\n");
	getchar();
	if (dev_num <= 0)
		return 0;										// there are no attached spectrometers

	for (int index=0; index<dev_num; ++index)
	{
		printf("attached spectrometer serial number: %s\n",wrapper.getSerialNumber(index).getASCII());
	}
	printf("\n");

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