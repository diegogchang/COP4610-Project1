fn app_sum(xs: &[i64]) -> i64 {
    xs.iter().sum()
}

fn main() {
    let data = [1, 2, 3, 4, 5];
    let s = app_sum(&data);
    println!("[proj2] example run ok. sum={}", s);
}
