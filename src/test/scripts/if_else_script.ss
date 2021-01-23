TEST_SCRIPT(
  let x = true;

  if x {
    print "true";
  } else {
    print "false";
  }

  x = false;

  if x {
    print "true";
  } else {
    print "false";
  }

  let y = true;

  if x and y {
    print "and";
  } else if x {
    print "x";
  } else if y {
    print "y";
  }

  if x or y {
    print "or";
  } else {
    print "not or";
  }
)