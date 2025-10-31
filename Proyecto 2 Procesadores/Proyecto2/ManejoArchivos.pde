void archivoSeleccionado(File seleccion) {
  
  
  if (seleccion == null) {
    println("No se seleccionó ningún archivo.");
  } else {
    listaInstrucciones = loadStrings(seleccion);
    for(int i = 0; i < listaInstrucciones.length; i++){
       println(listaInstrucciones[i]);
    }
    println("Archivo seleccionado: " + seleccion.getAbsolutePath());
  }
}

void archivoGuardado(File seleccion) {//Este metodo guarda el archivo a partir de una ejecucion
  if (seleccion == null) {
    println("No se eligió destino.");
  } else {
    // ruta completa del archivo elegido por el usuario
    String ruta = seleccion.getAbsolutePath();

    // si el archivo no tiene extensión, se agrega .txt automáticamente
    if (!ruta.endsWith(".txt")) {
      ruta += ".txt";
    }

    saveStrings(ruta, instrucciones.split("\n"));

    println("Archivo guardado en: " + ruta);
  }
}
