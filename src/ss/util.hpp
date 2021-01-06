namespace ss
{
  enum class OpCode
  {
    RETURN,
  };

  enum class InterpretResult
  {
    OK,
    COMPILE_ERROR,
    RUNTIME_ERROR,
  };
}  // namespace ss