public abstract class Pager extends ALG {


    public Pager() {}

    protected void incrementTimes(int seconds, boolean thrashing) {
        for  (Page p : pages) {
            if (p.loaded) {
                p.incrementTime(seconds);
            }
        }

        this.increaseTime(seconds);

        if (thrashing) {
            this.increaseThrashingTime(seconds);
        }
    }

    protected void loadPages(List<Page> pages, boolean isNew, boolean loaded) {
        for (Page page : pages) {
            page.loaded = true;
            page.daddr = 0;
            page.maddr = 0;
            if (isNew && !loaded) {
                this.pages.add(page);
            }
        }

        setMADDRs();

        if (isNew || !loaded) {
            this.setRamKB(pages.size() * 4);
            this.setLoadedPages(pages.size());
        }

        if (!isNew && !loaded) {
            setVirtualRamKB(pages.size() * 4 * -1);
            setUnloadedPages(pages.size() * -1);
        }
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

    public void callNew(int pid, int size) {
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

        // Liberar espacio necesario
        int spaceNeeded = sizeNeeded(newPages);
        int freeMemory = getFreeMemory();
        int hits = Math.min(newPages.size(), freeMemory / 4000);
        int misses = newPages.size() - hits;
        while (freeMemory < spaceNeeded) {
            this.unloadPages(whoToUnload());
            freeMemory = getFreeMemory();
        }

        // Cargar Informacion a las paginas y agregarlas
        loadPages(newPages, true, false);
        this.incrementTimes(hits + (5 * misses), false);
        updateInfo();
    }

    public void callUse(int ptr) {
        // Obtener paginas con el puntero
        List<Page> usePages = getPagesbyPtr(ptr);

        // Liberar espacio necesario
        int hits, misses;
        boolean loaded;
        if (!isLoaded(ptr)) {
            int spaceNeeded = sizeNeeded(usePages);
            int freeMemory = getFreeMemory();
            hits = Math.min(usePages.size(), freeMemory / 4000);
            misses = usePages.size() - hits;
            while (freeMemory < spaceNeeded) {
                this.unloadPages(whoToUnload());
                freeMemory = getFreeMemory();
            }
            loaded = false;
        } else {
            hits = usePages.size();
            misses = 0;
            loaded = true;
        }

        // Cargar Informacion a las paginas llamadas
        loadPages(usePages, false, loaded);
        this.incrementTimes(hits, false);
        this.incrementTimes(5 * misses, 0 < misses);
        updateInfo();
    }

    public void callDelete(int ptr) {
        this.deletePagesStats(this.getPagesbyPtr(ptr));
        pages.removeIf(page -> page.laddr == ptr);
        updateInfo();
    }

    public void callKill(int pid) {
        this.deletePagesStats(this.getPagesbyPid(pid));
        this.processes--;
        pages.removeIf(page -> page.pid == pid);
        updateInfo();
    }

    private boolean isLoaded(int ptr) {
        for  (Page p : pages) {
            if (p.laddr == ptr && p.loaded) {
                return true;
            }
        }
        return false;
    }


    private void deletePagesStats(List<Page> toDelete) {
        for  (Page p : toDelete) {
            if (p.loaded) {
                this.setRamKB(-4);
                this.setLoadedPages(-1);
            } else {
                this.setVirtualRamKB(-4);
                this.setUnloadedPages(-1);
            }
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

    protected List<Page> getPagesbyPid(int pid) {
        List<Page> result = new ArrayList<>();
        for  (Page page : pages) {
            if(page.pid == pid) {
                result.add(page);
            }
        }
        return result;
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

    private int sizeNeeded(List<Page> p) {
        return 4000 *  p.size();
    }

    private void getFragmentation(){
        int fragmentation = 0;
        for (Page page : pages) {
            if (page.loaded) {
                fragmentation += 4000 - page.memoryUsed;
            }
        }
        fragmentation /= 4000;
        setFragmentation(fragmentation);
    }

    public void updateInfo() {
        setRamPercentage();
        setVirtualRamPercentage();
        setThrashingPercentage();
        getFragmentation();
    }

    private void setMADDRs() {
        int nextMADDRs = 0;
        for (Page page : pages) {
            if (page.loaded) {
                page.maddr = ++nextMADDRs;
            }
        }
    }


    private void setDADDRs() {
        int nextDADDRs = 0;
        for (Page page : pages) {
            if (!page.loaded) {
                page.daddr = ++nextDADDRs;
            }
        }
    }

    public void printPages() {
        System.out.println("Size: " + pages.size());
        System.out.print("Loaded: [ ");
        for (Page page : pages) {
            if (page.loaded)
                System.out.print(page.laddr + " ");
        }
        System.out.println("]");
    }

    // devuelve el ptr del proceso que debe dormir (cambia con cada ALG)
    protected abstract int whoToUnload();
    
    void update() {
      this.updateInfo();
    }
}


private class FIFO extends Pager {

    @Override
    protected int whoToUnload() {
        int ptr = -1;
        int maxTime = -1;
        for (Page page : pages) {
            if (page.loaded && maxTime < page.loadedtime) {
                maxTime = page.loadedtime;
                ptr = page.laddr;
            }
        }
        return ptr;
    }

}

private class SC extends Pager {

    @Override
    public void callUse(int ptr) {
        // Obtener paginas con el puntero
        List<Page> usePages = getPagesbyPtr(ptr);

        // Liberar espacio necesario
        int hits, misses;
        boolean loaded;
        if (!super.isLoaded(ptr)) {
            int spaceNeeded = super.sizeNeeded(usePages);
            int freeMemory = super.getFreeMemory();
            hits = Math.min(usePages.size(), freeMemory / 4000);
            misses = usePages.size() - hits;
            while (freeMemory < spaceNeeded) {
                this.unloadPages(whoToUnload());
                freeMemory = super.getFreeMemory();
            }
            loaded = false;
        } else {
            hits = usePages.size();
            misses = 0;
            for (Page page : usePages) {
                page.mark = true;
            }
            loaded = true;
        }

        // Cargar Informacion a las paginas llamadas
        loadPages(usePages, false, loaded);
        this.incrementTimes(hits, false);
        this.incrementTimes(5 * misses, 0 < misses);
        updateInfo();
    }

    @Override
    protected int whoToUnload() {
        Integer page = getInteger();
        if (page != null) return page;
        return whoToUnloadAux();
    }

    private int whoToUnloadAux() {
        Integer page = getInteger();
        if (page != null) return page;
        return -1;
    }

    private Integer getInteger() {
        pages.sort(Comparator.comparingInt((Page p) -> p.loadedtime).reversed());

        for (Page page : pages) {
            if (page.loaded) {
                if (page.mark) {
                    page.mark = false;
                } else {
                    return page.laddr;
                }
            }
        }
        return null;
    }
}

private class MRU extends Pager {
    @Override
    protected void incrementTimes(int seconds, boolean thrashing) {
        for  (Page p : pages) {
            if (p.loaded) {
                p.incrementTime(seconds);
                p.lastCalledTime += seconds;
            }
        }

        this.increaseTime(seconds);

        if (thrashing) {
            this.increaseThrashingTime(seconds);
        }
    }

    @Override
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

    @Override
    protected int whoToUnload() {
        int ptr = -1;
        int minTime = Integer.MAX_VALUE;

        for (Page page : pages) {
            if (page.loaded && page.lastCalledTime < minTime) {
                minTime =  page.lastCalledTime;
                ptr = page.laddr;
            }
        }
        return ptr;
    }
}

private class RND extends Pager {

    private List<Page> getLoadedPages() {
        List<Page> result = new ArrayList<>();
        for (Page page : pages) {
            if (page.loaded) {
                result.add(page);
            }
        }
        return result;
    }

    @Override
    protected int whoToUnload() {
        List<Page> loadedPages = getLoadedPages();
        int rnd = this.rand.nextInt(loadedPages.size());
        return loadedPages.get(rnd).laddr;
    }
}
