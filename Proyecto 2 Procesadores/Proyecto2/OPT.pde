
List<Integer> listaTraducida;


void traducir(String[] listaInstrucciones){
      
  OPT.lastPtr = 0;
  listaTraducida.clear();
  
  String instruccion;
  for (int i = 0; i < listaInstrucciones.length; ++i){
    
    instruccion = listaInstrucciones[i];
    
    int n;
    int openID  = instruccion.indexOf('(');
    int closeID = instruccion.lastIndexOf(')');

    if (instruccion.matches(patronUse)){
      n = Integer.parseInt(instruccion.substring(openID + 1, closeID).trim());
      listaTraducida.add(n);

    } else if (instruccion.matches(patronNew)){
      listaTraducida.add(OPT.lastPtr);
      OPT.lastPtr++;

    } else {
      listaTraducida.add(0);
    }
  }
}












private class OPT extends Pager{
  

  
    OPT(){
      x = 0;
    }
    
    


    
    
    
    @Override
    protected int whoToUnload() {
      println("Holi estoy muerto");
      int ptr = -1;
      // indice (instriccion actual)
      int maxPos = -1;

      println("<INDICE> : "+indice);
      println("<listaTraducida.size()> : "+listaTraducida.size());
      for (int i = indice+1; i < listaTraducida.size(); ++i){
        int currentPagePtr = listaTraducida.get(i);

        //println("holi probando..." + currentPagePtr);
        if (super.isLoaded(currentPagePtr)){
          //La lista no esta vacia (???)          
          //La primera pagina del puntero esta cargada
          int currentPos = nextPos(indice+1, listaTraducida.get(i));
          println("holi sigo vivo, btw: currentPos:" + currentPos);
          
          if (currentPos > maxPos){
            maxPos = currentPos;
            ptr = listaTraducida.get(i);
          }
        }
      }
      println(ptr + " era bromita");
        
      return ptr;
    }
    
    private int nextPos(int start, int page){
      for (int i = start; i < listaTraducida.size(); ++i){
        if (page == listaTraducida.get(i)){
          return i;
        }
      }
      
      return 9999;
    }

}
