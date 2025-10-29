










private class OPT extends ALG{
  
    List<Integer> listaTraducida;
  
    OPT(){
      listaTraducida = new ArrayList<Integer>();
    }
    
    void traducir(String[] listaInstrucciones){
      
      this.lastPtr = 0;
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
          listaTraducida.add(lastPtr);
          lastPtr++;

        } else {
          listaTraducida.add(0);
        }
      }
    }


    








    

    
    void callNew(int pid, int size){
      
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
          
          
      int spaceNeeded = sizeNeeded(newPages);
      int freeMemory = getFreeMemory();
      
      for  (Page page : pages) {
        if (page.loaded) {
          freeMemory -= 4000;
        }
      }
      
      int hits = min(newPages.size(), freeMemory / 4000);
      int misses = newPages.size() - hits;
      
      while (freeMemory < spaceNeeded){
        this.unloadPages(whoToUnload());
        freeMemory = getFreeMemory();
      }
      
      loadPages(newPages, true, false);
      this.incrementTimes(hits + (5 * misses), false);
      updateInfo();
    }

    
    protected void unloadPages(int ptr) {
      List<Page> toUnload = getPagesbyPtr(ptr);
      for  (Page p : toUnload) {
        p.loaded = false;
        p.daddr = 0;
        p.loadedtime = 0;
        p.maddr = 0;
      }

      setDADDRs();

      this.setRamKB(toUnload.size() * 4 * -1);
      this.setLoadedPages(toUnload.size() * -1);

      this.setVirtualRamKB(toUnload.size() * 4);
      this.setUnloadedPages(toUnload.size());
    }
    
    
    
    
    
    
    
    
    protected void loadPages(List<Page> pages, boolean isNew, boolean loaded) {
        for (Page page : pages) {
            page.loaded = true;
            page.daddr = 0;
            page.maddr = 0;
            page.lastCalledTime = 0;
            if (isNew && !loaded) {
                this.pages.add(page);
            }
        }

        super.setMADDRs();

        if (isNew || !loaded) {
            this.setRamKB(pages.size() * 4);
            this.setLoadedPages(pages.size());
        }

        if (!isNew && !loaded) {
            setVirtualRamKB(pages.size() * 4 * -1);
            setUnloadedPages(pages.size() * -1);
        }
    }
    
    protected List<Page> getPagesbyPtr(int ptr) {
      List<Page> result = new ArrayList<>();
      for  (Page page : pages) {
        if(page.laddr == ptr) {
          result.add(page);
        }
      }
      return result;
    }
    
    private void setDADDRs() {
      int nextDADDRs = 0;
      for (Page page : pages) {
        if (!page.loaded) {
          page.daddr = ++nextDADDRs;
        }
      }
    }
    
    private int sizeNeeded(List<Page> p) {
        return 4000 *  p.size();
    }
    
    
    
    
    
    
    
    
    
    
    
    
    
    protected int whoToUnload() {
      int ptr = -1;
      // indice (instriccion actual)
      int maxPos = -1;

      for (int i = indice+1; i < listaTraducida.size(); ++i){
        int currentPos = nextPos(indice+1, listaTraducida.get(i));
        
        if (currentPos > maxPos){
          maxPos = currentPos;
          ptr = listaTraducida.get(i);
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
      
      return 9999;
    }
    
    private int getFreeMemory() {
      int freeMemory = 400000;
      for  (Page page : pages) {
        if (page.loaded) {
          freeMemory -= 4000;
        }
      }
      return freeMemory;
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
    
    
    
    
    

    
    public void callUse(int ptr){
      
    }
    
    public void callDelete(int ptr){
      
    }
    
    public void callKill(int pid){
      
    }


}
