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

Memory mOPT;
Memory mALG;

ArrayList<Page> MMUOPT;
ArrayList<Page> MMUALG;

OPT OPT;
ALG ALG;

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
    .addItem("500", 0)
    .addItem("1000", 1)
    .addItem("5000", 2)
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
  
  mOPT = new Memory(100);
  mALG = new Memory(100);
  
  MMUOPT = new ArrayList<Page>();
  MMUALG = new ArrayList<Page>();
  
  OPT = new OPT(2);
  ALG = new FIFO();
  
  
  
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
  
  //println(frameRate); //Aprox framerate
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
      
      addRandomPage();
    
      if (MMUOPT.size() > scrollMaxCount){
        MMUOPT.clear();
      }
    }
    
    
    

    
    drawMemory(mOPT, 0);
    drawMemory(mALG, 1);
    
    drawPages(MMUOPT, 0);
    
    println("Paginas en ALG:" + ALG.pages.size());
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






void mouseWheel(MouseEvent event){
  int mmuMax = max(MMUOPT.size(), MMUALG.size());
  int x = max(mmuMax - scrollMaxCount, 0);
  offsetY -= event.getCount() * 25;
  offsetY = constrain(offsetY, -x * 15, 0);
}

void mousePressed(){
  
  if (mouseButton == LEFT){
    //pruebaMUM();
  }
  
  if (mouseButton == RIGHT){
    println(instrucciones);
  }

}

void keyPressed(){
  if (key == ' '){
    pausa();
  }
  
}

void pausa(){
  if(!start){
    pause = !pause;
    
    if (pause){
      pausa.setLabel("Reanudar");
    } else {
      pausa.setLabel("Pausar");
    }
  }
}

void jump(){
  timer = 1;
}

void inicio(){ //Boton
  start = false;
  startSimulation();
}

void crear(){ //Boton
  
  mum.setRandom(semilla);
  mum.setProcesses(procesos);

  instrucciones = mum.randomInstructions(operaciones);
  listaInstrucciones = instrucciones.split("\\R");
  
  
  
  int respuesta = JOptionPane.showConfirmDialog(
    null,                              // Ventana padre (null = ventana principal)
    "¿Desea guardar las instrucciones generadas?",             // Mensaje
    "Guardar Archivo",                    // Título de la ventana
    JOptionPane.YES_NO_OPTION          // Tipo de botones
  );
  
  if (respuesta == JOptionPane.YES_OPTION) {
    selectOutput("Elige donde guardar el archivo:", "archivoGuardado");
  }
}

void cargar(){ //Boton
  
  selectInput("Selecciona un archivo a cargar:", "archivoSeleccionado");
  
  // ################### TEMPORAL ###################
  // ################### TEMPORAL ###################
  // ################### TEMPORAL ###################
  

  mum.setRandom(semilla);
  mum.setProcesses(procesos);

  instrucciones = mum.randomInstructions(operaciones);
  listaInstrucciones = instrucciones.split("\\R");
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

void inputSemilla(String txt){
  if (txt == null || txt.length() == 0) return;
  
  try {
    float v = float(txt);
    menuSemilla.setText(str(int(v)));
    semilla = (int) v;
  } catch (Exception ex) {
    // error/invalido (?)
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
  
  indice = 0;
  
  ALG.processes = procesos;
  ALG.time = 0;
}

void menuPrincipal(){
  
}
