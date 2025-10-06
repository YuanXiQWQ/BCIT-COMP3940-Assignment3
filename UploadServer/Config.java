import java.io.File;

public final class Config {
    private static final Config INSTANCE = new Config();
    private final File imagesDir = new File("images");

    private Config() {}

    public static Config getInstance() {return INSTANCE;}

    public File ensureImagesDir()
    {
        if(!imagesDir.exists())
        {
            imagesDir.mkdirs();
        }
        return imagesDir;
    }
}
