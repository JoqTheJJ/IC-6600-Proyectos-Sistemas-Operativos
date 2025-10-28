

void mouseWheel(MouseEvent event){
  int mmuMax = max(MMUOPT.size(), ALG.pages.size());
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
    null,
    "¿Desea guardar las instrucciones generadas?",
    "Guardar Archivo",
    JOptionPane.YES_NO_OPTION
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
