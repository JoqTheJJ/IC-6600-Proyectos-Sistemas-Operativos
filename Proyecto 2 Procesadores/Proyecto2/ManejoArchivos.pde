void archivoSeleccionado(File seleccion) {
  
  
  if (seleccion == null) {
    println("No se seleccionó ningún archivo.");
  } else {
    println("Archivo seleccionado: " + seleccion.getAbsolutePath());
  }
}

void archivoGuardado(File seleccion) {
  
  //instrucciones contiene lo que se guarda
  
  if (seleccion == null) {
    println("No se eligió destino.");
  } else {
    println("Se guardará en: " + seleccion.getAbsolutePath());
  }
}
