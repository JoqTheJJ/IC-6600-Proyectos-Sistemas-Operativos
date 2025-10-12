



class Page {
  int id;
  int pid;
  int laddr;
  int maddr;
  int daddr;
  int loadedtime;
  boolean loaded;
  boolean mark;
  
  Page(int id, int pid, int laddr, int maddr, int daddr, int loadedtime,
          boolean loaded, boolean mark){
    this.id = id;
    this.pid = pid;
    this.loaded = loaded;
    this.laddr = laddr;
    this.maddr = maddr;
    this.daddr = daddr;
    this.loadedtime = loadedtime;
    this.mark = mark;
  }
}





class Memoria {
  int len; //Tamano memoria actual
  int fallos;
  int[] ram;
  final int ramSize; //Tamano maximo
  
  Memoria(int size){
    this.len = 0;
    this.fallos = 0;
    this.ramSize = size;
    this.ram = new int[ramSize];
  }
  
  
}

void drawMemoria(Memoria m, int posY){
  int squareWidth = 10*width/12 /m.ramSize;
  int posX = width/12;
  int incrementX = squareWidth;
  
  int squareHeight = 20;
  
  stroke(0);
  fill(255);
  
  for(int i = 0; i < m.ramSize; i++){
    rect(posX, posY, squareWidth, squareHeight);
    posX += incrementX;
  }
}


/*
fill(255);
    rect(ancho, y, ancho, altura);
    fill(0);
    text(loadedPages, ancho +1, y + altura - 1);
*/

color pageColor(int seed){
  randomSeed(seed);
  return color(random(155)+100, random(155)+100, random(155)+100);
}

void drawPages(ArrayList<Page> pages, int algorythm){
  // algorythm = 0/1
  int posX = algorythm * width/2 + width/12;
  int posY = 5*height/8;
  int ancho = 4*width/12;
  
  text("Page ID", posX + 5, posY);
  text("PID", posX + 45, posY);
  text("Loaded", posX + 85, posY);
  text("L-Addr", posX + 125, posY);
  text("M-Addr", posX + 165, posY);
  text("D-Addr", posX + 205, posY);
  text("Loaded-T", posX + 245, posY);
  text("Mark", posX + 285, posY);
  
  for (Page page:pages){
    fill(pageColor(page.id));
    stroke(0);
    rect(posX, posY, ancho, 15);
    
    posY += 12;
    fill(0);
    text(page.id, posX + 5, posY);
    text(page.pid, posX + 45, posY);
    
    if (page.loaded){
      text("X", posX + 85, posY);
    }
    
    text(page.laddr, posX + 125, posY);
    text(page.maddr, posX + 165, posY);
    text(page.daddr, posX + 205, posY);
    
    text(page.loadedtime, posX + 245, posY);
    
    if (page.mark){
      text("X", posX + 285, posY);
    }
    
    posY += 3;
  }
  
  
}
