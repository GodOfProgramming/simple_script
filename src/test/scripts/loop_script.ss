TEST_SCRIPT(
  let v = 0;
  loop {
    if v > 4 {
      break;
    }
    print v;
    v = v + 1;
  }
)