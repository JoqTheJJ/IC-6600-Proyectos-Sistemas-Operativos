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

    // si el ar+++chivo no tiene extensión, se agrega .txt automáticamente
    if (!ruta.endsWith(".txt")) {
      ruta += ".txt";
    }

    saveStrings(ruta, instrucciones.split("\n"));

    println("Archivo guardado en: " + ruta);
  }
}



//    // Método para guardar archivo
//void guardarArchivoInstrucciones(String contenido, String nombreArchivo){
//  //Ruta personalizada donde guardarás el archivo
//  String rutaBase = "C:/Users/josgf/Desktop/Proyecto2Procesadores/IC-6600-Proyectos-Sistemas-Operativos-main/IC-6600-Proyectos-Sistemas-Operativos-main/Proyecto 2 Procesadores/Proyecto2/ManejoDeArchivos/";
  
//  // Crear la carpeta si no existe
//  File carpeta = new File(rutaBase);
//  if (!carpeta.exists()) carpeta.mkdirs();
  
//  // Guardar el archivo
//  saveStrings(rutaBase + nombreArchivo, contenido.split("\n"));
  
//  println("Archivo guardado en: " + rutaBase + nombreArchivo);
//}
