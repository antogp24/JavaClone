void print_identity_matrix(int rows, int cols) {
    for (int y = 0; y < rows; y++) {
        sout("[ ");
        for (int x = 0; x < cols; x++) {
            sout(x == y ? "1" : "0");
            sout(x < cols - 1 ? ", " : " ");
        }
        soutln("] ");
    }
}

abstract class Math {
    static final double PI = 3.14159;

    static long gcd(long a, long b) {
        if (b > a) {
            long temp = a; a = b; b = temp;
        }
        if (b == 0) return a;
        if (b == 1) return 1;
        return Math.gcd(b, a % b);
    }

    static long lcm(long a, long b) {
        return (a * b) / Math.gcd(a, b);
    }
    
    static long fib(long n) {
        if (n < 2) return n;
        double p = pow(1 + sqrt(5), n) - pow(1 - sqrt(5), n);
        double q = pow(2, n) * sqrt(5);
        return (long)(p / q);
    }

    static boolean is_prime(int n) {
        boolean result = true;
        for (int d = 2; d <= (int)sqrt(n); d++) {
            if (n % d == 0) {
                result = false;
                break;
            }
        }
        return result;
    }
}

// Printing an identity matrix.
print_identity_matrix(4, 4);

// Printing greatest common divisor, and lowest common multiple of 105 and 30.
sout("gcd(105, 30): ");
soutln(Math.gcd(105, 30));
sout("lcm(105, 30): ");
soutln(Math.lcm(105, 30));

// Printing out the first n fibonacci numbers and timing it.
long start = clock(), n = 10;
for (int i = 1; i <= n; i++) {
    sout(Math.fib(i));
    if (i < n) sout(", ");
    else soutln("");
}
long end = clock();
sout("It took ");
sout(end - start);
sout(" milliseconds to print the first ");
sout(n);
soutln(" fibonacci numbers.");

// Checking if a number is prime.
int number = 12;
sout("Is ");
sout(number);
sout(" prime? ");
soutln(Math.is_prime(number) ? "yes" : "no");

// Calling a functions and classes and printing their values.
soutln(Math);
soutln(pow);
// Math();

class Vector2 {
    public double x, y;

    public __init__(double x, double y) {
        this.x = x;
        this.y = y;
    }

    public static Vector2 diagonal(double a) {
        return Vector2(a, a);
    }
    
    public Vector2 add(Vector2 b) {
        return Vector2(this.x + b.x, this.y + b.y);
    }

    public Vector2 sub(Vector2 b) {
        return Vector2(this.x - b.x, this.y - b.y);
    }

    public double getLength() {
        return sqrt(this.x * this.x + this.y + this.y);
    }
    
    public void println() {
        sout(this); sout(": Vector2{"); sout(this.x); sout(", "); sout(this.y); soutln("}");    
    }
}

// Testing classes and instances.
Vector2 v1 = Vector2(randint(-100, 100), randint(-100, 100));
Vector2 v2 = v1;

v1.println();
v2.println();
v1.sub(Vector2.diagonal(10))).println();
Vector2.diagonal(500).println();

sout("The length of the vector is ");
soutln(v1.getLength());
