import controlP5.*;
import java.util.ArrayList;
import java.util.List;
import java.util.Random;
import java.util.Comparator;
import javax.swing.JOptionPane;


// ############################################### //
// ################## MetadataS ################## //
// ############################################### //

boolean pause = true; //Pause the simulation
boolean start = true; //Main menu of the simulation
int timer = -1;       //Timer for one advance frames
float offsetY = 0; //Scroll
int scrollMaxCount; //Scroll pages amount

// ################### <OTHER> ################### //

PFont hideFont;
PFont defaultFont;
PFont menuFontBig;
PFont menuFont;

// ############################################### //
// ################## ControlP5 ################## //
// ############################################### //

ControlP5 cp5;
Button pausa;
Button inicio;
Button menu;
Button crear;
Button cargar;
Button jump;
Slider frameRateSlider;
float framerate = 60;
float limitFrames = 600;

RadioButton menuProcesos;
int procesos = 50;
RadioButton menuOperaciones;
int operaciones = 5000;
RadioButton menuAlgoritmo;
int algoritmo = 0;
Textfield menuSemilla;
int semilla = 0;









// ############################################### //
// ################## Simulation ################# //
// ############################################### //

MUM mum;

OPT OPT;
ALG ALG;
ALG FIFO;
ALG SC;
ALG MRU;
ALG RND;


int mmuMax;

String instrucciones;
String[] listaInstrucciones;
int indice = -1;





// ############################################### //
// ################ Regex Patterns ############### //
// ############################################### //

String patronNew    = "new\\(.*";
String patronDelete = "delete\\(.*";
String patronUse    = "use\\(.*";
String patronKill   = "kill\\(.*";


void setup(){
  //fullScreen();
  size(1200, 600);
  
  //Color mode
  colorMode(HSB, 360, 100, 100);
  
  hideFont = createFont("Arial", 1);
  defaultFont = createFont("SansSerif", 10);
  menuFontBig = createFont("8bitOperatorPlus8-Regular.ttf", 50);
  menuFont = createFont("8bitOperatorPlus8-Regular.ttf", 20);
  
  cp5 = new ControlP5(this);
  
  //Slider framerate 
  frameRateSlider = cp5.addSlider("framerate")
          .setPosition(width/12 + 100, height/16)
          .setSize(width/3, 20)
          .setRange(5, limitFrames)
          .setValue(framerate)
          .setCaptionLabel("");
  frameRateSlider.hide();
          //.setFont(sliderFont)
          
  jump = cp5.addButton("jump")
     .setLabel(">")
     .setPosition(width/12 + 70, height/16)
     .setSize(20, 20);
  jump.hide();
  
  pausa = cp5.addButton("pausa")
     .setLabel("Pausar")
     .setPosition(width/12, height/16)
     .setSize(60, 20);
  pausa.hide();
  
  menu = cp5.addButton("menu")
     .setLabel("Menu")
     .setPosition(5*width/12 + 110, height/16)
     .setSize(60, 20);
  menu.hide();
  
  crear = cp5.addButton("crear")
     .setLabel("Crear")
     .setPosition(width/2 + 150, height/2)
     .setSize(80, 25)
     .setFont(menuFont);
     
  cargar = cp5.addButton("cargar")
     .setLabel("Cargar")
     .setPosition(width/2 + 150, height/2 + 30)
     .setSize(80, 25)
     .setFont(menuFont);
  
  inicio = cp5.addButton("inicio")
     .setLabel("Inicio")
     .setPosition(width/2, 3*height/4)
     .setSize(120, 40)
     .setFont(menuFont);
     
  menuProcesos = cp5.addRadioButton("mprocesos")
    .setPosition(width/2 - width/5, height/4)
    .setSize(25, 25)
    .setItemsPerRow(1)
    .setSpacingRow(6)
    .addItem("10", 10)
    .addItem("50", 50)
    .addItem("100", 100)
    .activate(procesos)
    .setFont(menuFont);
    
  menuOperaciones = cp5.addRadioButton("moperaciones")
    .setPosition(width/2, height/4)
    .setSize(25, 25)
    .setItemsPerRow(1)
    .setSpacingRow(6)
    .addItem("500", 500)
    .addItem("1000", 1000)
    .addItem("5000", 5000)
    .activate(operaciones)
    .setFont(menuFont);
    
  menuAlgoritmo = cp5.addRadioButton("malgoritmo")
    .setPosition(width/2 + width/5, height/4)
    .setSize(25, 25)
    .setItemsPerRow(1)
    .setSpacingRow(6)
    .addItem("FIFO", 0)
    .addItem("SC", 1)
    .addItem("MRU", 2)
    .addItem("RND", 3)
    .activate(algoritmo)
    .setFont(menuFont);
    
  menuSemilla = cp5.addTextfield("inputSemilla")
    .setPosition(width/2, height/2)
    .setSize(120, 25)
    .setAutoClear(false)
    .setInputFilter(ControlP5.INTEGER)
    .setText(str(int(semilla)))
    .setLabel("Semilla")
    .setFont(menuFont);
    
    
    
  for (Toggle t : menuProcesos.getItems()) {
    t.getCaptionLabel().setFont(menuFont);
    t.getCaptionLabel().setColor(color(0));
  }
  for (Toggle t : menuOperaciones.getItems()) {
    t.getCaptionLabel().setFont(menuFont);
    t.getCaptionLabel().setColor(color(0));
  }
  for (Toggle t : menuAlgoritmo.getItems()) {
    t.getCaptionLabel().setFont(menuFont);
    t.getCaptionLabel().setColor(color(0));
  }
  menuSemilla.getCaptionLabel().setColor(color(0));
  
  mmuMax = 6;
  mum = new MUM();
  mum.setRandom(semilla);
  mum.setProcesses(procesos);
  instrucciones = mum.randomInstructions(operaciones);
  listaInstrucciones = instrucciones.split("\\R");
  
  OPT = new OPT();

  scrollMaxCount = ((3*height/8)/15)-2;
}

void draw(){
  
  if (start){ //Menu Principal
    
    background(#CECECE);
    fill(0);
    menuPrincipal();
    
  } else { //Simulacion Principal
    
    background(200);
    
    translate(0, offsetY);
    frameRate(framerate);
    
    
    OPT.display();
    ALG.display();
    
    
    if (!pause){
      
      indice++;
      String instruccion;
      if (indice >= listaInstrucciones.length){
        instruccion = "uwu";
      } else {
        instruccion = listaInstrucciones[indice];
      }
      
      
      
      OPT.update(instruccion);
      ALG.update(instruccion);
      
    }
    
    
    
    drawMemoryFromPages(OPT.pages, 0);
    drawMemoryFromPages(ALG.pages, 1);
    
    drawPages(OPT.pages, 0);
    drawPages(ALG.pages, 1);
  }
  
  
  if (timer == 0){
    pause = true;
    timer--;
  } else if (timer > 0) {
    pause = false;
    timer--;
  }
}








void menu(){ //Boton
  start = true;
  
  menuProcesos.show();
  menuOperaciones.show();
  menuAlgoritmo.show();
  menuSemilla.show();
  inicio.show();
  cargar.show();
  crear.show();
   
  frameRateSlider.hide();
  pausa.hide();
  jump.hide();
  menu.hide();
}



void startSimulation(){
  textFont(defaultFont);
  
  pause = true;
  pausa.setLabel("Iniciar");
  //pid general = 0
  //pid opt = 0
   
  menuProcesos.hide();
  menuOperaciones.hide();
  menuAlgoritmo.hide();
  menuSemilla.hide();
  inicio.hide();
  cargar.hide();
  crear.hide();
   
  frameRateSlider.show();
  pausa.show();
  jump.show();
  menu.show();
  
  indice = -1;
  
  
  switch (algoritmo){
    case 0:
      ALG = new FIFO();
      
    case 1:
      ALG = new SC();
      
    case 2:
      ALG = new MRU();
      
    case 3:
      ALG = new RND();
  }
  
  OPT.traducir(listaInstrucciones);
  ALG.processes = procesos;
  ALG.time = 0;
}



void menuPrincipal(){
  
}
