#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
#include <cryptopp/integer.h>
#include <cryptopp/nbtheory.h>
#include <cryptopp/osrng.h>//使用crypto++库
#include <chrono>

using namespace CryptoPP;
using namespace std;
using namespace std::chrono;

using Complex = std::complex<double>;
const double PI = acos(-1);

void my_fft(vector<Complex>& a, bool inverse) {//手写fft,true为ifft,false为fft
    int n = a.size();
    if (n == 1) return;

    vector<Complex> a0(n / 2), a1(n / 2);
    for (int i = 0; 2 * i < n; ++i) {
        a0[i] = a[i * 2];
        a1[i] = a[i * 2 + 1];
    }
    my_fft(a0, inverse);
    my_fft(a1, inverse);//fft递归

    double angle = 2 * PI / n * (inverse ? -1 : 1);
    Complex w(1), wn(cos(angle), sin(angle));//fft旋转因子
    for (int i = 0; 2 * i < n; ++i) {
        a[i] = a0[i] + w * a1[i];
        a[i + n / 2] = a0[i] - w * a1[i];
        if (inverse) {
            a[i] /= 2;
            a[i + n / 2] /= 2;//递归层数为k=log2N，进行2^k=N次，实现ifft归一化
        }
        w *= wn;
    }
}

vector<int> split_integer(const Integer& num, int base) {
    vector<int> digits;
    Integer temp = num;
    while (temp > 0) {
        digits.push_back((int)(temp % base));
        temp /= base;
    }
    return digits;//vector储存digits
}

Integer combine_digits(const vector<int>& digits, int base) {
    Integer result = 0;
    Integer multiplier = 1;
    for (int digit : digits) {
        result += digit * multiplier;
        multiplier *= base;
    }
    return result;
}

vector<int> fft_multiply(const vector<int>& a, const vector<int>& b, int base) {
    vector<Complex> fa(a.begin(), a.end()), fb(b.begin(), b.end());
    int n = 1;//size_t
    while (n < a.size() + b.size()) n <<= 1;
    fa.resize(n);
    fb.resize(n);//n=2^((log[lena+lenb]+1)/log2)

    my_fft(fa, false);
    my_fft(fb, false);
    for (int i = 0; i < n; ++i) fa[i] *= fb[i];
    my_fft(fa, true);

    vector<long long> temp(n);
    for (int i = 0; i < n; ++i) temp[i] = static_cast<long long>(round(fa[i].real()));//取整

    vector<int> result(n);
    long long carry = 0;
    for (int i = 0; i < n; ++i) {
        long long sum = temp[i] + carry;
        result[i] = sum % base;
        carry = sum / base;
    }
    while (carry > 0) {
        result.push_back(carry % base);
        carry /= base;
    }//进位
    return result;
}

Integer my_multiply(const Integer& num1, const Integer& num2, int base) {
    vector<int> a = split_integer(num1, base);
    vector<int> b = split_integer(num2, base);
    vector<int> result_digits = fft_multiply(a, b, base);
    return combine_digits(result_digits, base);
}

int main() {
    AutoSeededRandomPool prng;

    Integer num1, num2;
    int digits_1 = 1, digits_2 = 1;
    cout << "Please enter the digit length of the two large random numbers, two positive 'int' divided by space." << endl;
    cin >> digits_1 >> digits_2;
    digits_1 = ((digits_1 > 0) ? digits_1 : 1);
    digits_2 = ((digits_2 > 0) ? digits_2 : 1);
    num1.Randomize(prng, digits_1);
    num2.Randomize(prng, digits_2);

    int base = 32768;//(base^2)<2^31-1，可以考虑使用long long int等扩大 base，提高效率

    auto start_fft = high_resolution_clock::now();
    Integer fft_result = my_multiply(num1, num2, base);
    auto end_fft = high_resolution_clock::now();
    auto fft_duration = duration_cast<microseconds>(end_fft - start_fft);
    auto start_lib = high_resolution_clock::now();
    Integer lib_result = num1 * num2;
    auto end_lib = high_resolution_clock::now();
    auto lib_duration = duration_cast<microseconds>(end_lib - start_lib);

    cout << "Num1: " << num1 << endl << "Num2: " << num2 << endl;
    cout << "FFT Result: " << fft_result << endl << "Library Result: " << lib_result << endl;
    cout << "Results match: " << (fft_result == lib_result) << endl;

    double FFT_t = fft_duration.count(), Lib_t = lib_duration.count();
    cout << "FFT-based multiplication time: " << fft_duration.count() << " us(微秒)" << endl;
    cout << "Library multiplication time: " << lib_duration.count() << " us(微秒)" << endl;
    if (abs(Lib_t - 0) > 10e-6) cout << "FFT/Lib time ratio: " << FFT_t / Lib_t << endl;
    else cout << "Chrono can't even catch the Library multiplication time!" << endl;
    cout << "It turned out that log(FFT/Lib time ratio) is around 2-3.5.(ratio will increase when the digit lengths of num1 and num2 differ significantly)" << endl;
    return 0;
}
