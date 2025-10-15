





void pruebaMUM(){
  MUM mum = new MUM();

  mum.setRandom(811);
  mum.setProcesses(100);

  System.out.println(mum.randomInstructions(5));
}





private final class Pointer {
    private final int pid;
    private final int ptr;

    public Pointer(int pid, int ptr) {
        this.pid = pid;
        this.ptr = ptr;
    }

    public int getPid() {
        return pid;
    }

    public int getPtr() {
        return ptr;
    }
}

public class MUM {
    private int nextPtr = 0;
    private List<Integer> processes = new ArrayList<>();
    private List<Pointer> pointers = new ArrayList<>();
    private Random rand = new Random();

    public void setRandom(long seed) {
        this.rand = new Random(seed);
    }

    public void setProcesses(int quantity) {
        processes.clear();
        for (int i = 0; i < quantity; i++) {
            processes.add(i);
        }
    }

    public boolean isAlive(int pid) {
        return processes.contains(pid);
    }

    public boolean newPtr(int pid) {
        if (!isAlive(pid)) return false;
        pointers.add(new Pointer(pid, nextPtr++));
        return true;
    }

    public void deletePtr(int ptr) {
        for (int i = 0; i < pointers.size(); i++) {
            if (pointers.get(i).getPtr() == ptr) {
                pointers.remove(i);
                break;
            }
        }
    }

    public void killPid(int pid) {
        pointers.removeIf(p -> p.getPid() == pid);
        processes.remove(Integer.valueOf(pid));
    }

    private String randomNew() {
        int pid = rand.nextInt(processes.size());
        pid = processes.get(pid);
        int x = rand.nextInt(10);
        double y = rand.nextDouble();
        int size = 4 + rand.nextInt((int) ((x*4000) + (y*4000)));
        if (newPtr(pid)) {
            return "new(" + pid + ", " + size +")\n";
        }
        return "";
    }

    private String randomUse() {
        int ptr = rand.nextInt(pointers.size());
        ptr = pointers.get(ptr).getPtr();
        return "use(" + ptr + ")\n";
    }

    private String randomDelete() {
        int ptr = rand.nextInt(pointers.size());
        ptr = pointers.get(ptr).getPtr();
        deletePtr(ptr);
        return "delete(" + ptr + ")\n";
    }

    private String randomKill() {
        int pid = rand.nextInt(processes.size());
        pid = processes.get(pid);
        killPid(pid);
        return "kill(" + pid + ")\n";
    }

    private String randomInstruction() {
        String command = "";
        if (processes.isEmpty()) {return command;}
        double chance = rand.nextDouble();
        if (pointers.isEmpty()) {
            if (chance < 0.5) {
                command = randomNew();
            }
            else {
                command = randomKill();
            }
        }
        else {
            if (chance < 0.25) {
                command = randomNew();
            } else if (chance < 0.5) {
                command = randomUse();
            } else if (chance < 0.75) {
                command = randomDelete();
            } else {
                command = randomKill();
            }
        }
        return command;
    }

    public String randomInstructions(int quantity) {
        StringBuilder command = new StringBuilder();
        for (int i = 0; i < quantity; i++) {
            command.append(randomInstruction());
        }
        return command.toString();
    }
}
