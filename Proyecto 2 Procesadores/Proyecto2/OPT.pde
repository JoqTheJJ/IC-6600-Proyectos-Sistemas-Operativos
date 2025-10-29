



private class OPT extends ALG{
  
    OPT(){
      
    }
    
    /**
    
    
    
    
    
    
    void assignMemory(int pid, int size){
      
      int pagesNeeded = size / 4000;
      int lastPageSpace = size % 4000;
      List<Page> newPages = new ArrayList<>();

      // Crear paginas
      for (int i = 0; i < pagesNeeded; ++i) {
          newPages.add(new Page(pid, this.lastPtr, 4000));
      }
      if (lastPageSpace != 0)
          newPages.add(new Page(pid, this.lastPtr++, lastPageSpace));
      else
          ++this.lastPtr;
          
          
      int pagesToFree = pagesNeeded + 1;
      
      */
      
      
      
      
    }
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    void update(String instruccion){
      
      
      
      
      
      
      
      int n;
      int openID  = instruccion.indexOf('(');
      int closeID = instruccion.lastIndexOf(')');

      if (instruccion.matches(patronUse)){
        n = Integer.parseInt(instruccion.substring(openID + 1, closeID).trim());
        callUse(n);

      } else if (instruccion.matches(patronNew)){
        String[] temp = instruccion.substring(openID + 1, closeID).split("\\s*,\\s*");
        n = Integer.parseInt(temp[0]);
        int size = Integer.parseInt(temp[1]);
        callNew(n, size);

      } else if (instruccion.matches(patronDelete)){
        n = Integer.parseInt(instruccion.substring(openID + 1, closeID).trim());
        callDelete(n);

      } else if (instruccion.matches(patronKill)){
        n = Integer.parseInt(instruccion.substring(openID + 1, closeID).trim());
        callKill(n);

      } else {
        println("Angy ¯\\_(=/)_/¯");
      }
    }
    
    
    
    
    
    
    
    public void callNew(int pid, int size){
      
    }
    
    public void callUse(int ptr){
      
    }
    
    public void callDelete(int ptr){
      
    }
    
    public void callKill(int pid){
      
    }


}
