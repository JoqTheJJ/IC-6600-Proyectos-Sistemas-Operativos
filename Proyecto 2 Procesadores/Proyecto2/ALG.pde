


abstract class ALG{
  
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
  
  int x = width/2;
  
  
  ALG(int processes){
    this.processes = processes;
    time = 0;
    
    ramKB = 245;
    ramPercentage = 0.0001;
    virtualRamKB = 41;
    virtualRamPercentage = 0.96;
    
    
    loadedPages = 24;
    unloadedPages = 65;
    
    thrashingTime = 051;
    thrashingPercentage = 0.4;
    fragmentacion = 721;
  }
  
  abstract void update();
  abstract void iNEW();
  abstract void iUSE();
  abstract void iDEL();
  abstract void iKILL();
  
  void display(){
    
    int ancho = width/12;
    int altura = 20;
    
    int y = height/4;
    

    
    fill(360);
    rect(x+ancho, y, ancho*2, altura);
    fill(0);
    text("Processes", x+ancho +1, y + altura - 1);
    
    fill(360);
    rect(x+3*ancho, y, ancho*2, altura);
    fill(0);
    text("Simulation Time", x+3*ancho +1, y + altura - 1);
    
    //\\ y //\\
    y += altura;
    
    fill(360);
    rect(x+ancho, y, ancho*2, altura*2);
    fill(0);
    text(processes, x+ancho +10, y + altura*1.5 - 1);
    
    fill(360);
    rect(x+3*ancho, y, ancho*2, altura*2);
    fill(0);
    text(time + "s", x+3*ancho +10, y + altura*1.5 - 1);
    
    //\\ y //\\
    y += altura;
    y += altura;
    y += altura;
    
    fill(360);
    rect(x+ancho, y, ancho, altura);
    fill(0);
    text("RAM KB", x+ancho +1, y + altura - 1);
    
    fill(360);
    rect(x+2*ancho, y, ancho, altura);
    fill(0);
    text("RAM %", x+2*ancho +1, y + altura - 1);
    
    fill(360);
    rect(x+3*ancho, y, ancho, altura);
    fill(0);
    text("V-RAM KB", x+3*ancho +1, y + altura - 1);
    
    fill(360);
    rect(x+4*ancho, y, ancho, altura);
    fill(0);
    text("V-RAM %", x+4*ancho +1, y + altura - 1);
    
    //\\ y //\\
    y += altura;
    
    fill(360);
    rect(x+ancho, y, ancho, altura);
    fill(0);
    text(ramKB, x+ancho +1, y + altura - 1);
    
    fill(360);
    rect(x+2*ancho, y, ancho, altura);
    fill(0);
    text(nf(ramPercentage * 100, 2, 2) + "%", x+2*ancho +1, y + altura - 1);
    
    fill(360);
    rect(x+3*ancho, y, ancho, altura);
    fill(0);
    text(virtualRamKB, x+3*ancho +1, y + altura - 1);
    
    fill(360);
    rect(x+4*ancho, y, ancho, altura);
    fill(0);
    text(nf(virtualRamPercentage * 100, 2, 2) + "%", x+4*ancho +1, y + altura - 1);
    
    //\\ y //\\
    y += altura;
    y += altura;
    
    fill(360);
    rect(x+ancho, y, ancho*2, altura);
    fill(0);
    text("PAGES", x+ancho +1, y + altura - 1);
    
    if (thrashingTime > time*0.5){
      fill(0, 68, 99);
    } else {
      fill(360);
    }
    rect(x+3*ancho, y, ancho, altura);
    fill(0);
    text("Thrashing", x+3*ancho +1, y + altura - 1);
    
    fill(360);
    rect(x+4*ancho, y, ancho, altura);
    fill(0);
    text("Fragmentacion", x+4*ancho +1, y + altura - 1);
    
    //\\ y //\\
    y += altura;
    
    fill(360);
    rect(x+ancho, y, ancho, altura);
    fill(0);
    text("LOADED", x+ancho +1, y + altura - 1);
    
    fill(360);
    rect(x+2*ancho, y, ancho, altura);
    fill(0);
    text("UNLOADED", x+2*ancho +1, y + altura - 1);
    
    if (thrashingTime > time*0.5){
      fill(0, 68, 99);
    } else {
      fill(360);
    }
    rect(x+3*ancho, y, ancho/2, altura*2);
    fill(0);
    text(thrashingTime + "s", x+3*ancho +1, y + altura*1.5 - 1);
    
    if (thrashingTime > time*0.5){
      fill(0, 68, 99);
    } else {
      fill(360);
    }
    rect(3.5*ancho+x, y, ancho/2, altura*2);
    fill(0);
    text(nf(thrashingPercentage * 100, 2, 2) + "%", 3.5*ancho+x +1, y + altura*1.5 - 1);
    
    fill(360);
    rect(4*ancho+x, y, ancho, altura*2);
    fill(0);
    text(fragmentacion + " KB", 4*ancho+x +1, y + altura*1.5 - 1);
    
    //\\ y //\\
    y += altura;
    
    fill(360);
    rect(x+ancho, y, ancho, altura);
    fill(0);
    text(loadedPages, x+ancho +1, y + altura - 1);
    
    fill(360);
    rect(x+2*ancho, y, ancho, altura);
    fill(0);
    text(unloadedPages, x+2*ancho +1, y + altura - 1);
  }
}
