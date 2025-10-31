
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
      listaTraducida.add(-1);
    }
  }
  
  OPT.lastPtr = 0;
}












private class OPT extends Pager{
  

  
    OPT(){
      this.algorythm = Algorythm.OPT;
    }
    
    


    
    
    
    @Override
    protected int whoToUnload() {
      int ptr = -1;
      // indice (instriccion actual)
      int maxPos = -1;

      for (int i = indice+1; i < listaTraducida.size(); ++i){
        int currentPagePtr = listaTraducida.get(i);

        if (currentPagePtr != -1 && super.isLoaded(currentPagePtr)){
          //El puntero no es invalido
          //La primera pagina del puntero esta cargada
          
          int currentPos = nextPos(indice+1, listaTraducida.get(i));
          
          if (currentPos > maxPos){
            maxPos = currentPos;
            ptr = listaTraducida.get(i);
          }
        }
      }
      
      if (ptr == -1){ //Si el puntero es invalido elimina a cualquiera
        for (Page page : pages){
          if (super.isLoaded(page.laddr)){
            return page.laddr;
          }
        }
      }
        
      return ptr;
    }
    
    private int nextPos(int start, int page){
      for (int i = start; i < listaTraducida.size(); ++i){
        if (page == listaTraducida.get(i)){
          return i;
        }
      }
      
      return -1;
    }

}
