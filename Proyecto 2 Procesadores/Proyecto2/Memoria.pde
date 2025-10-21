


static class Page {
    static int nextID = 0;
    final int id;
    int pid;
    int laddr;
    int maddr;
    int daddr;
    int loadedtime;
    int lastCalledTime;
    boolean loaded;
    boolean mark;
    int memoryUsed;

    public Page(int pid, int laddr, int memoryUsed) {
        this.id =  ++nextID;
        this.pid = pid;
        this.laddr = laddr;
        this.memoryUsed = memoryUsed;
        this.loadedtime = 0;
        this.lastCalledTime = 0;
        this.loaded = true;
    }
    
    public Page(int loadedtime, int daddr, int maddr, int laddr, int pid, int id, boolean loaded, boolean mark) {
        this.loadedtime = loadedtime;
        this.daddr = daddr;
        this.maddr = maddr;
        this.laddr = laddr;
        this.pid = pid;
        this.id = id;
        this.loaded = loaded;
        this.mark = mark;
    }

    public void setMaddr(int maddr) {
        this.maddr = maddr;
    }

    public void setDaddr(int daddr) {
        this.daddr = daddr;
    }

    public void incrementTime(int seconds) {
        this.loadedtime += seconds;
    }

    public void changeLoaded() {
        this.loaded =  !this.loaded;
    }

    public void changeMark() {
        this.mark = !this.mark;
    }
}






class Memory {
  int len; //Tamano memoria actual
  int fallos;
  int[] ram;
  final int ramSize; //Tamano maximo
  
  Memory(int size){
    this.len = 0;
    this.fallos = 0;
    this.ramSize = size;
    this.ram = new int[ramSize];
  }
  
  
}

void drawMemory(Memory m, int algorythm){
  
  int posY = height/8 + algorythm*height/16;
  int squareWidth = 10*width/12 /m.ramSize;
  int posX = width/12;
  int incrementX = squareWidth;
  
  int squareHeight = 20;
  
  stroke(0);
  fill(360);
  
  for(int i = 0; i < m.ramSize; i++){
    rect(posX, posY, squareWidth, squareHeight);
    posX += incrementX;
  }
}


///////////////////////////////////////////////////////
//////////////           PAGES           //////////////
///////////////////////////////////////////////////////




void pageColor(int seed){
  randomSeed(seed);  
  float h = (seed*37+156) % 360;
  float s = 30 + (seed * 71) % 40;
  float b = 60 + (seed * 97) % 20;
  fill(h, s, b);
}

void drawPages(ArrayList<Page> pages, int algorythm){
  // algorythm = 0/1
  int posX = algorythm * width/2 + width/12;
  int posY = 5*height/8;
  int ancho = 4*width/12;
  
  int colWidth = ancho/8;
  
  fill(0);
  posY -= 2;
  text("Page ID", posX, posY);
  text("PID", posX + colWidth, posY);
  text("Loaded", posX + colWidth*2, posY);
  text("L-Addr", posX + colWidth*3, posY);
  text("M-Addr", posX + colWidth*4, posY);
  text("D-Addr", posX + colWidth*5, posY);
  text("Loaded-T", posX + colWidth*6, posY);
  text("Mark", posX + colWidth*7, posY);
  posY += 2;
  
  for (Page page:pages){
    pageColor(page.pid); //Assigns the color of the process based on the pid
    stroke(0);
    rect(posX, posY, ancho, 15);
    
    posY += 12;
    fill(0);
    text(page.id, posX + 5, posY);
    text(page.pid, posX + colWidth + 5, posY);
    
    posX += 7;
    if (page.loaded){
      text("X", posX + colWidth*2 + 5, posY);
    }
    
    text(page.laddr, posX + colWidth*3 + 5, posY);
    if (page.maddr != 0){
      text(page.maddr, posX + colWidth*4 + 5, posY);
    }
    if (page.daddr != 0){
      text(page.daddr, posX + colWidth*5 + 5, posY);
    }
    
    text(page.loadedtime, posX + colWidth*6 + 5, posY);
    
    if (page.mark){
      text("X", posX + colWidth*7 + 5, posY);
    }
    
    posX -= 7;
    posY += 3;
  }
  
  
}
