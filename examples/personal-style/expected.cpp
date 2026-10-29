// Function declarations: stock clang-format does not add tilde separators.
void prepare();
void run();
void wait();
void step(int index);
bool firstLongReadinessConditionIsSatisfied();
bool secondLongReadinessConditionIsSatisfied();

// Simple if/else: optional braces can be removed.
void simple(bool ready)
{
  if ( ready )
    run();
  else
    wait();
}

// A multi-statement branch does not cause insertion in an unbraced else.
void mixedBranches(bool ready)
{
  if ( ready )
  {
    prepare();
    run();
  }
  else
    wait();
}

// A wrapped condition does not cause insertion of missing braces.
void wrappedCondition()
{
  if ( firstLongReadinessConditionIsSatisfied() &&
       secondLongReadinessConditionIsSatisfied() )
    run();
  else
    wait();
}

// Spacing after ! and inside while parentheses.
void polling(bool bStopped)
{
  while ( ! bStopped )
  {
    run();
    bStopped = true;
  }
}

// Stock conditional-parenthesis spacing also affects for headers.
void counting(int count)
{
  for ( int index = 0; index < count; ++index )
    step(index);
}
