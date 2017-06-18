public class Bar {
    private int i;
    private String str;
    private Float of;


    public Bar()
    {
        System.out.println("Creating BAR");
    }

    public int getI() {
        return i;
    }

    public void setI(int i) {
        System.out.println("Setting i = " + i);
        this.i = i;
    }

    public String getStr() {
        return str;
    }

    public void setStr(String str) {
        System.out.println("Setting str = " + str);
        this.str = str;
    }

    public Float getOf() {
        return of;
    }

    public void setOf(Float of) {
        System.out.println("Setting of = " + of);
        this.of = of;
    }
}
