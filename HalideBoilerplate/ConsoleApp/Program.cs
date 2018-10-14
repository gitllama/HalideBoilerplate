using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace ConsoleApp
{
    static class HaildeGenerated
    {
        //EntryPoint = "SetWindowText")
        //[DllImport("Halide.exe", CallingConvention = CallingConvention.Cdecl)]
        //public unsafe static extern void Demosic(IntPtr src, IntPtr dst, int width, int height);

        //[DllImport("HalideGenerated.dll")]
        //public unsafe static extern void Test(IntPtr src, IntPtr dst, int width, int height, int offset);

        //[DllImport("HalideGenerated.dll")]
        //public unsafe static extern int GetInt();

        [DllImport("HalideGenerated.dll")]
        public unsafe static extern int Test();

    }

    class Program
    {
        static void Main(string[] args)
        {

            Console.WriteLine(HaildeGenerated.Test());

            Console.ReadKey();

        //    int w = 4;
        //    int h = 4;
        //    var src = new int[w * h];
        //    var dst = new int[w * h];

        //    unsafe
        //    {
        //        fixed (int* i = src)
        //        fixed (int* j = dst)
        //        {
        //            //var f = HaildeGenerated.GetInt();
        //            HaildeGenerated.Test(new IntPtr(i), new IntPtr(j), w, h, 5);
        //        }
        //    }
        }
    }
}
