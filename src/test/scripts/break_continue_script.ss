TEST_SCRIPT(
  for let v = 0; v < 5; v=v+1 {
    if v >= 3 {
      break;
    }
    print v;
  }
  for let v = 0; v < 5; v=v+1 {
    if v < 3 {
      cont;
    }
    print v;
  }
)