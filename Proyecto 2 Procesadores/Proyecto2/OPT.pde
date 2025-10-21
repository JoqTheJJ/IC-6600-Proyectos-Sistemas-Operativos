



class OPT extends ALG{
  
  
  int processes;
  int time;
  
  int ramKB;
  float ramPercentage;
  int virtualRamKB;
  float virtualRamPercentage;
  
  
  int loadedPages;
  int unloadedPages;
  
  int thrashingTime;
  float thrashingPercentage;
  int fragmentacion;
  
  
  OPT(int processes){
    
    this.processes = processes;
    time = 0;
    
    ramKB = 244;
    ramPercentage = 0.9999;
    virtualRamKB = 40;
    virtualRamPercentage = 0.04;
    
    
    loadedPages = 42;
    unloadedPages = 56;
    
    thrashingTime = 150;
    thrashingPercentage = 0.6;
    fragmentacion = 127;
    
    x = 0;
  }
  
  void callNew(int pid, int size) {
    
  }
  
  void callUse(int ptr) {
    
  }
  
  void callDelete(int ptr) {
    
  }
  
  void callKill(int pid) {
    
  }
  
  void update(){
    
  }
  
  void iNEW(){
    
  }
  
  void iUSE(){
    
  }
  
  void iDEL(){
    
  }
  
  void iKILL(){
    
  }
}
