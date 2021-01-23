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
)