public class Bar {
    public Bar()
    {
        System.out.println("Creating BAR");
    }

    public float get_secret_value()
    {
        return 234.1f;
    }

    public java.lang.String get_secret_string()
    {
        throw new RuntimeException("Fuck!!!!");
    }
}
