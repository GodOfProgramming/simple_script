fn foo() {
  print "in func";
}

{
  fn bar() {
    print "in local func";
  }

  bar();
}

foo();