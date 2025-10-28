


abstract class ALG{
    
  
    List<Page> pages;
    Random rand;
  
    int lastPtr;
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
    int fragmentation; // le cambie el nombre

    int x = width/2;
  
  
    ALG(){
      
        this.pages = new ArrayList<Page>();
        this.rand = new Random();
      
        this.lastPtr = 0;
        this.processes = 0;
        this.time = 0;

        this.ramKB = 0;
        this.virtualRamKB = 0;

        this.loadedPages = 0;
        this.unloadedPages = 0;

        this.thrashingTime = 0;
    }

    
    public void setProcesses(int processes) {
        this.processes += processes;
    }

    public void increaseTime(int seconds) {
        this.time += seconds;
    }

    public void setRamKB(int ramKB) {
        this.ramKB += ramKB;
    }

    public void setRamPercentage() {
        this.ramPercentage =  ((float) this.ramKB) / 400;
    }

    public void setVirtualRamKB(int virtualRamKB) {
        this.virtualRamKB += virtualRamKB;
    }

    public void setVirtualRamPercentage() {
        this.virtualRamPercentage = ((float) this.virtualRamKB) / 400;
    }

    public void setLoadedPages(int loadedPages) {
        this.loadedPages += loadedPages;
    }

    public void setUnloadedPages(int unloadedPages) {
        this.unloadedPages += unloadedPages;
    }

    public void increaseThrashingTime(int seconds) {
        this.thrashingTime += seconds;
    }

    public void setThrashingPercentage() {
        this.thrashingPercentage = ((float) this.thrashingTime) / ((float) this.time);
    }

    public void setFragmentation(int fragmentation) {
        this.fragmentation = fragmentation;
    }

    public abstract void callNew(int pid, int size);
    public abstract void callUse(int ptr);
    public abstract void callDelete(int ptr);
    public abstract void callKill(int pid);
  
    abstract void iNEW();
    abstract void iUSE();
    abstract void iDEL();
    abstract void iKILL();



    void update(String instruccion){
      int n;
      int open  = instruccion.indexOf('(');
      int close = instruccion.lastIndexOf(')');
      
      if (instruccion.matches(patronUse)){
        n = Integer.parseInt(instruccion.substring(open + 1, close).trim());
        callUse(n);
        
      } else if (instruccion.matches(patronNew)){
        String[] temp = instruccion.substring(open + 1, close).split("\\s*,\\s*");
        n = Integer.parseInt(temp[0]);
        int size = Integer.parseInt(temp[1]);
        callNew(n, size);
        
      } else if (instruccion.matches(patronDelete)){
        n = Integer.parseInt(instruccion.substring(open + 1, close).trim());
        callDelete(n);
        
      } else if (instruccion.matches(patronKill)){
        n = Integer.parseInt(instruccion.substring(open + 1, close).trim());
        callKill(n);
        
      } else {
        println("Angy ¯\\_(=/)_/¯");
      }
    }
  
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
      text(fragmentation + " KB", 4*ancho+x +1, y + altura*1.5 - 1);
      
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
