



class OPT{
  
  
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
  }
  
  void update(){
    
  }
  
  void display(){
    
    int ancho = width/12;
    int altura = 20;
    
    int y = height/4;
    

    
    fill(255);
    rect(ancho, y, ancho*2, altura);
    fill(0);
    text("Processes", ancho +1, y + altura - 1);
    
    fill(255);
    rect(3*ancho, y, ancho*2, altura);
    fill(0);
    text("Simulation Time", 3*ancho +1, y + altura - 1);
    
    //\\ y //\\
    y += altura;
    
    fill(255);
    rect(ancho, y, ancho*2, altura*2);
    fill(0);
    text(processes, ancho +10, y + altura*1.5 - 1);
    
    fill(255);
    rect(3*ancho, y, ancho*2, altura*2);
    fill(0);
    text(time + "s", 3*ancho +10, y + altura*1.5 - 1);
    
    //\\ y //\\
    y += altura;
    y += altura;
    y += altura;
    
    fill(255);
    rect(ancho, y, ancho, altura);
    fill(0);
    text("RAM KB", ancho +1, y + altura - 1);
    
    fill(255);
    rect(2*ancho, y, ancho, altura);
    fill(0);
    text("RAM %", 2*ancho +1, y + altura - 1);
    
    fill(255);
    rect(3*ancho, y, ancho, altura);
    fill(0);
    text("V-RAM KB", 3*ancho +1, y + altura - 1);
    
    fill(255);
    rect(4*ancho, y, ancho, altura);
    fill(0);
    text("V-RAM %", 4*ancho +1, y + altura - 1);
    
    //\\ y //\\
    y += altura;
    
    fill(255);
    rect(ancho, y, ancho, altura);
    fill(0);
    text(ramKB, ancho +1, y + altura - 1);
    
    fill(255);
    rect(2*ancho, y, ancho, altura);
    fill(0);
    text(nf(ramPercentage * 100, 2, 2) + "%", 2*ancho +1, y + altura - 1);
    
    fill(255);
    rect(3*ancho, y, ancho, altura);
    fill(0);
    text(virtualRamKB, 3*ancho +1, y + altura - 1);
    
    fill(255);
    rect(4*ancho, y, ancho, altura);
    fill(0);
    text(nf(virtualRamPercentage * 100, 2, 2) + "%", 4*ancho +1, y + altura - 1);
    
    //\\ y //\\
    y += altura;
    y += altura;
    
    fill(255);
    rect(ancho, y, ancho*2, altura);
    fill(0);
    text("PAGES", ancho +1, y + altura - 1);
    
    fill(255);
    rect(3*ancho, y, ancho, altura);
    fill(0);
    text("Thrashing", 3*ancho +1, y + altura - 1);
    
    fill(255);
    rect(4*ancho, y, ancho, altura);
    fill(0);
    text("Fragmentacion", 4*ancho +1, y + altura - 1);
    
    //\\ y //\\
    y += altura;
    
    fill(255);
    rect(ancho, y, ancho, altura);
    fill(0);
    text("LOADED", ancho +1, y + altura - 1);
    
    fill(255);
    rect(2*ancho, y, ancho, altura);
    fill(0);
    text("UNLOADED", 2*ancho +1, y + altura - 1);
    
    fill(255);
    rect(3*ancho, y, ancho/2, altura*2);
    fill(0);
    text(thrashingTime + "s", 3*ancho +1, y + altura*1.5 - 1);
    
    fill(255);
    rect(3.5*ancho, y, ancho/2, altura*2);
    fill(0);
    text(nf(thrashingPercentage * 100, 2, 2) + "%", 3.5*ancho +1, y + altura*1.5 - 1);
    
    fill(255);
    rect(4*ancho, y, ancho, altura*2);
    fill(0);
    text(fragmentacion + " KB", 4*ancho +1, y + altura*1.5 - 1);
    
    //\\ y //\\
    y += altura;
    
    fill(255);
    rect(ancho, y, ancho, altura);
    fill(0);
    text(loadedPages, ancho +1, y + altura - 1);
    
    fill(255);
    rect(2*ancho, y, ancho, altura);
    fill(0);
    text(unloadedPages, 2*ancho +1, y + altura - 1);
  }
}
