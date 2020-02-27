using System;
using System.Runtime.InteropServices;

public interface IGFF
{

}

public class GFF
{
  private const string LIBDS = "libds";

  [DllImport(LIBDS)]
  private static extern void gff_init();

  [DllImport(LIBDS)]
  private static extern void gff_load_directory([MarshalAs(UnmanagedType.LPStr)]string path);

  [DllImport(LIBDS)]
  public static extern int gff_find_index([MarshalAs(UnmanagedType.LPStr)]string name);

  [DllImport(LIBDS)]
  public static extern int get_frame_count(int fileIndex, int resourceId, int resourceType);

  [DllImport(LIBDS)]
  public static extern void gff_print_something();

  public GFF()
  {
    gff_init();
  }

  public void LoadDirectory(string path)
  {
    gff_load_directory(path);
  }
}