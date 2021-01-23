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
    print "same";
  } else if x {
    print "x";
  } else if y {
    print "y";
  }