public class Bar {
    private int i;
    private String str;
    private Float of;
    private String ostr;
    private Baz bz;
    private Baz obz;


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
        this.str = null;
    }

    public Float getOf() {
        return of;
    }

    public void setOf(Float of) {
        this.of = of;
    }

    public String getOstr() {
        return ostr;
    }

    public void setOstr(String ostr) {
        System.out.println("Setting ostr = " + ostr);
        this.ostr = ostr;
    }

    public Baz getBz() {
        return bz;
    }

    public void setBz(Baz bz) {
        onBz(bz);
        this.bz = bz;
    }

    public Baz getObz() {
        return obz;
    }

    public void setObz(Baz obz) {
        this.obz = obz;
    }

    public void doSomething(double d)
    {
        System.out.println("Doing something: " + d);
    }

    public native void onBz(Baz bz);
}
