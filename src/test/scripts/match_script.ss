TEST_SCRIPT(
  let v = "hello";
  match v {
    true => {
      print "at all";
    }

    "hello" => print "at hello";
  }
)
