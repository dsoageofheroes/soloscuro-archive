using System.Runtime.InteropServices;

public static class GFF
{
  private const string LIBDS = "libds.so";

  [DllImport(LIBDS)]
  public static extern void init();

}