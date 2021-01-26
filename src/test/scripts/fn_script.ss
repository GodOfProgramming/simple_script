TEST_SCRIPT(
  fn foo(x, y) {
    ret x + y;
  }
  {
    fn bar(x, y) {
      fn inner(x) {
        ret x;
      }
      {
        fn inner2(y) {
          ret y;
        }

        ret inner(x) - inner2(y);
      }
    }
    print bar(3, 4) - foo(1, 2);
  }
)