TEST_SCRIPT(
  fn foo(x, y) {
    ret x + y;
  }
  {
    fn bar(x, y) {
      ret x - y;
    }
    print bar(3, 4) - foo(1, 2);
  }
)