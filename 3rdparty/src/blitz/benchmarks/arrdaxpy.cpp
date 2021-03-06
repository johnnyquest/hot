// Array DAXPY benchmark

#include <blitz/array.h>
#include <blitz/benchext.h>
#include <blitz/rand-uniform.h>

BZ_USING_NAMESPACE(blitz)

#ifdef BZ_FORTRAN_SYMBOLS_WITH_TRAILING_UNDERSCORES
  #define stencilf stencilf_
#endif

extern "C" {
    void arrdaxpyf(double* A, double* B, int& N, int& iters, double& a);
}

void arrdaxpyFortran77Version(BenchmarkExt<int>& bench);
void arrdaxpyBlitzVersion(BenchmarkExt<int>& bench);

int main()
{
    BenchmarkExt<int> bench("Array DAXPY", 2);

    const int numSizes = 128;

    bench.setNumParameters(numSizes);
    bench.setRateDescription("Mflops/s");

    Vector<int> parameters(numSizes);
    Vector<long> iters(numSizes);
    Vector<double> flops(numSizes);

    for (int i=0; i < numSizes; ++i)
    {
        parameters[i] = (i+1);
        iters[i] = 16*32*8*8*8/(i+1)/(i+1)/(i+1);
        float npoints = parameters[i];
        flops[i] = npoints * npoints * npoints * 2 * 2;
    }

    bench.setParameterVector(parameters);
    bench.setIterations(iters);
    bench.setFlopsPerIteration(flops);

    bench.beginBenchmarking();
    arrdaxpyBlitzVersion(bench);
    arrdaxpyFortran77Version(bench);
    bench.endBenchmarking();

    bench.saveMatlabGraph("arrdaxpy.m");

    return 0;
}

void initializeRandomDouble(double* data, int numElements)
{
    static Random<Uniform> rnd;

    for (int i=0; i < numElements; ++i)
        data[i] = rnd.random();
}

void arrdaxpyBlitzVersion(BenchmarkExt<int>& bench)
{
    bench.beginImplementation("Blitz++");

    while (!bench.doneImplementationBenchmark())
    {
        int N = bench.getParameter();

        cout << "Blitz++: N = " << N << endl;
        cout.flush();

        long iters = bench.getIterations();

        Array<double,3> A(N,N,N), B(N,N,N);
        initializeRandomDouble(A.data(), N*N*N);
        initializeRandomDouble(B.data(), N*N*N);
        TinyVector<int,2> size = N-2;
        double a = 0.34928313;
        double b = - a; 

        bench.start();
        for (long i=0; i < iters; ++i)
        {
            A += a * B;
            A += b * B;
        }
        bench.stop();
    }

    bench.endImplementation();
}

void arrdaxpyFortran77Version(BenchmarkExt<int>& bench)
{
    bench.beginImplementation("Fortran 77");

    while (!bench.doneImplementationBenchmark())
    {
        int N = bench.getParameter();

        cout << "Fortran 77: N = " << N << endl;
        cout.flush();

        int iters = (int)bench.getIterations();

        size_t arraySize = size_t(N) * size_t(N) * N;
       
        double* A = new double[arraySize];
        double* B = new double[arraySize];

        initializeRandomDouble(A, arraySize);
        initializeRandomDouble(B, arraySize);

        bench.start();
        double a = 0.34928313;
        arrdaxpyf(A, B, N, iters, a);
        bench.stop();

        delete [] A;
        delete [] B;
    }

    bench.endImplementation();
}
