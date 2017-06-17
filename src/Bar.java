public class Bar {
    private int i;
    private String str;


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
}
