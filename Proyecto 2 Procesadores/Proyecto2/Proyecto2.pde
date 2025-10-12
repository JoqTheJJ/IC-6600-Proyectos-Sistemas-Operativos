import controlP5.*;
import java.util.ArrayList;

ControlP5 cp5;

Memoria mOPT;
Memoria mALG;

ArrayList<Page> MMUOPT;
ArrayList<Page> MMUALG;

OPT OPT;

int mmuMax;


float offsetY = 0; //Scroll
void setup(){
  //fullScreen();
  size(1000, 600);
  cp5 = new ControlP5(this);
  
  mmuMax = 39;
  
  mOPT = new Memoria(100);
  mALG = new Memoria(100);
  
  MMUOPT = new ArrayList<Page>();
  MMUALG = new ArrayList<Page>();
  
  OPT = new OPT(2);
  
  MMUOPT.add(new Page(0,0,0,0,0,0,false,false));
  MMUOPT.add(new Page(1,0,0,0,0,0,true,true));
  MMUOPT.add(new Page(2,0,0,0,0,0,false,true));
  MMUOPT.add(new Page(0,0,0,0,0,0,true,false));
  MMUOPT.add(new Page(2,0,0,0,0,0,false,true));
  MMUOPT.add(new Page(3,0,0,0,0,0,true,false));
  
  MMUALG.add(new Page(0,0,0,0,0,0,false,true));
  MMUALG.add(new Page(1,0,0,0,0,0,true,true));
  MMUALG.add(new Page(2,0,0,0,0,0,true,false));
  MMUALG.add(new Page(0,0,0,0,0,0,false,false));
  MMUALG.add(new Page(2,0,0,0,0,0,true,true));
  MMUALG.add(new Page(3,0,0,0,0,0,true,true));
}

void draw(){
  background(200);
  
  translate(0, offsetY);
  
  
  
  OPT.display();
  OPT.update();
  
  stroke(255);
  line(width/2, 0, width/2, height);
  
  drawMemoria(mOPT, 50);
  drawMemoria(mALG, 80);
  
  drawPages(MMUOPT, 0);
  drawPages(MMUALG, 1);
}






void mouseWheel(MouseEvent event){
  int x = max(mmuMax - 40, 0);
  offsetY -= event.getCount() * 25;
  offsetY = constrain(offsetY, -x * 15, 0);
}

void mousePressed(){
  if (mouseButton == LEFT){
    mmuMax++;
  } else if (mouseButton == RIGHT){
    mmuMax--;
  }

}
