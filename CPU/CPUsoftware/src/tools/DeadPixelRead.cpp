/* by Corrado Giammanco 30/04/2019*/

#include "DeadPixelRead.h"


DeadPixelMask::DeadPixelMask(){


     Pick_File();
     int a=ReadDead();

     }

/*Check if the file is in usb1, usb0, local*/
void DeadPixelMask::Pick_File(){



    std::string filename=this->direc_usb1+"/DeadPixelMask.txt";

    std::ifstream test_usb1(filename);


    if(!test_usb1.is_open()){

        filename=this->direc_usb0+"/DeadPixelMask.txt";
        std::ifstream test_usb0 (filename) ;


        if(!test_usb0.is_open()){

            filename=this->directory+"/DeadPixelMask.txt";


        }

    }


    this->readed_file=filename;

}


int DeadPixelMask::ReadDead(){


    std::string line;

    int flag=0;

    //int BOARD;
    //int ASIC;
    //int ECU;


   std::string filename=this->readed_file;

   std::ifstream ifile (filename);

   if (ifile.is_open()) {

    int nline=0;
    int X; //x and y indices inside of a chip to be converted in pixel number
    int Y;
    int Npixel;
    
    while(getline (ifile,line)){

       
      /*set a flag=0 to end the reading map*/
      if(line[0]=='^' && flag==1){
	flag=0;
	 getline (ifile,line);
      }

      /*set a flag=1 to read only the map*/
      if(line[0]=='^' && flag==0){
	flag=1;
	getline (ifile,line);	 
      }
      
      
      if(flag==1)
	{
	  /*remove white space and or tab from the matrix*/
	  line.erase(remove_if(line.begin(),line.end(),::isspace),line.end());
	  
	  /*check the position of 1 in the line */
	  if(line[0]!='\0'){
	     
	    int col; /*iteration variable to be remeber for check*/
	    for(col=0; line[col]!='\0'; col++){
	      
	      /*line[col]=49 it's the character 1*/
	       
	      if(line[col]==49){
		 
		/*having nline col coorinate of a dead pixel calculate  the BOARD ASIC and ECU and the number of pixel inside  of a chip*/
		pixel.BOARD=nline/8;
		pixel.ASIC=col/8;
		pixel.ECU=3*(nline/16)+col/16;
		X=fmod(nline,8);
		Y=fmod(col,8);
		pixel.Number=X*8+Y;
		
		cmaskline.line="slowctrl line "+  std::to_string(pixel.BOARD);
		cmaskline.asic="slowctrl asic "+  std::to_string(pixel.ASIC);
		cmaskline.pixel="slowctrl pixel "+std::to_string(pixel.Number);
		
		Dead.push_back(pixel);
		c2send.push_back(cmaskline);
		
		std::cout<<'('<<pixel.BOARD<<';'<<pixel.ASIC<<')'<<'('<<nline<<';'<<col<<')'<<pixel.ECU<<','<<X<<','<<Y<<','<<pixel.Number<<std::endl; 
		
	      }  
	      
	    }
	    
	    
	    //cout<<col<<endl;
	    if(col!=48){
	      
	      Dead.clear();
	      c2send.clear();
	      return 0;
	       
	    }
	    nline++;
	  }
	  
	}
      
    }
    ifile.close();
    
    //cout<<nline;
    if(nline!=48) {
      
      Dead.clear();
      c2send.clear();
      return 0; /*its a format error*/
      
    }
    
    
   }
   
   else {
     std::cout << "Unable to open "<<filename<<std::endl;
     Dead.clear();
     c2send.clear();
     return 0;
   }
   
   return 1;   
   
}


