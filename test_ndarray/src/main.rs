use ndarray::array;
use ndarray::Array1;
use num::complex::Complex;

fn main() {

    // Make an array?
    let mut x = array![1.0, 2.0];
    let y = array![3.0, 4.0];

    // Print elements?
    println!("x = {:?}", x);
    println!("y = {:?}", y);

    // Add elements?
    x += &y;
    println!("x = {:?}", x);

    // Multiply elements?
    let c = multiply_arrays(&x, &y);
    println!("c = {:?}", c);

    // Multiply arrays without taking ownership
    multiply_arrays_inplace(&mut x, &y);
    println!("x = {:?}", x);

    // Create a complex-valued array?
    let z = array![Complex::new(1.0, 2.0), Complex::new(3.0, 4.0)];
    println!("z = {:?}", z);

}

fn multiply_arrays(a: &Array1<f64>, b: &Array1<f64>) -> Array1<f64>
{
    let c = a * b;
    c // wtf we dont have return in rust
}

fn multiply_arrays_inplace(a: &mut Array1<f64>, b: &Array1<f64>) {
    *a *= b;
}