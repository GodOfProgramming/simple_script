let REPS = 100;

fn fib(n) {
  if n <= 1 {
    ret n;
  } else {
    ret fib(n - 2) + fib(n - 1);
  }
}

fn fib_it(n) {
  let prev = 0;
  let curr = 1;
  for let i = 0; i <= n; i = i + 1 {
    let tmp = curr;
    curr = curr + prev;
    prev = tmp;
  }
  ret prev;
}

let result;
let start = clock();
for let i = 0; i < REPS; i = i + 1 {
  result = fib(20);
}
let stop = clock();
print result;
print "took " + ((stop - start) / REPS) + " seconds";
