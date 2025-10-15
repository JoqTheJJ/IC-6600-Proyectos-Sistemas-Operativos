import controlP5.*;
import java.util.ArrayList;
import java.util.List;
import java.util.Random;



boolean pause = false; //Pause the simulation
float offsetY = 0; //Scroll
int scrollMaxCount; //Scroll pages amount

ControlP5 cp5;
Slider frameRateSlider;
float framerate = 60;

Memory mOPT;
Memory mALG;

ArrayList<Page> MMUOPT;
ArrayList<Page> MMUALG;

OPT OPT;
ALG ALG;

int mmuMax;



void setup(){
  //fullScreen();
  size(1000, 600);
  
  //Color mode
  colorMode(HSB, 360, 100, 100);
  
  
  cp5 = new ControlP5(this);
  
  //slider framerate
  PFont sliderFont = createFont("Arial", 1);
  frameRateSlider = cp5.addSlider("framerate")
          .setPosition(width/12, height/16)
          .setSize(width/3, 20)
          .setRange(5, 150)
          .setValue(framerate)
          .setCaptionLabel("");
          //.setFont(sliderFont)
  
  mmuMax = 6;
  
  mOPT = new Memory(100);
  mALG = new Memory(100);
  
  MMUOPT = new ArrayList<Page>();
  MMUALG = new ArrayList<Page>();
  
  OPT = new OPT(2);
  ALG = new ALG(10);
  
  MMUOPT.add(new Page(0,0,0,0,0,0,false,false));
  MMUOPT.add(new Page(1,0,0,0,0,0,true,true));
  MMUOPT.add(new Page(2,0,0,0,0,0,false,true));
  MMUOPT.add(new Page(0,0,0,0,0,0,true,false));
  MMUOPT.add(new Page(2,0,0,0,0,0,false,true));
  MMUOPT.add(new Page(3,0,0,0,0,0,true,false));
  
  MMUALG.add(new Page(0,0,0,0,0,0,false,true));
  MMUALG.add(new Page(1,0,0,0,0,0,true,true));
  MMUALG.add(new Page(2,1,0,0,0,0,true,false));
  MMUALG.add(new Page(0,2,0,0,0,0,false,false));
  MMUALG.add(new Page(2,1,0,0,0,0,true,true));
  MMUALG.add(new Page(3,3,0,0,0,0,true,true));
  
  scrollMaxCount = ((3*height/8)/15)-2;
}

void draw(){
  background(200);
  
  translate(0, offsetY);
  frameRate(framerate);
  
  
  OPT.display();
  ALG.display();
  
  
  if (!pause){
    OPT.update();
    ALG.update();
    
    addRandomPage();
  
    if (MMUOPT.size() > scrollMaxCount){
      MMUOPT.clear();
    }
  }
  
  
  
  stroke(255);
  line(width/2, 0, width/2, height);
  
  drawMemory(mOPT, 0);
  drawMemory(mALG, 1);
  
  drawPages(MMUOPT, 0);
  drawPages(MMUALG, 1);
}






void mouseWheel(MouseEvent event){
  int mmuMax = max(MMUOPT.size(), MMUALG.size());
  int x = max(mmuMax - scrollMaxCount, 0);
  offsetY -= event.getCount() * 25;
  offsetY = constrain(offsetY, -x * 15, 0);
}

void mousePressed(){
  
  if (mouseButton == LEFT){
    pruebaMUM();
  }
  
  if (mouseButton == RIGHT){
    //deleteLastPage();
  }

}

void keyPressed(){
  if (key == ' '){
    pause = !pause;
  }
}

void addRandomPage(){
  randomSeed((long)System.nanoTime());
  MMUOPT.add(new Page(
      mmuMax++,
      (int)random(10),
      (int)random(10),
      (int)random(10),
      (int)random(10),
      (int)random(10),
      false,
      true
      ));
}

void deleteLastPage(){
  MMUOPT.remove(MMUOPT.size() - 1);
  mmuMax--;
}
