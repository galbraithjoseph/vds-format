// Function declarations: stock clang-format does not add tilde separators.
void prepare();
void run();
void wait();
void step(int index);
bool firstLongReadinessConditionIsSatisfied();
bool secondLongReadinessConditionIsSatisfied();

// Simple if/else: optional braces can be removed.
void
simple(bool ready)
{
	if ( ready )
		run();
	else
		wait();
}

// A multi-statement branch requires braces throughout the chain.
void
mixedBranches(bool ready)
{
	if ( ready )
	{
		prepare();
		run();
	}
	else
	{
		wait();
	}
}

// A wrapped condition requires braces around both single-statement bodies.
void
wrappedCondition()
{
	if ( firstLongReadinessConditionIsSatisfied() && secondLongReadinessConditionIsSatisfied() &&
		 firstLongReadinessConditionIsSatisfied() )
	{
		run();
	}
	else
	{
		wait();
	}
}

// The chain decision includes all else-if branches, including mixed input.
void
chained(bool ready, bool retry)
{
	if ( ready )
	{
		run();
	}
	else if ( retry )
	{
		prepare();
		run();
	}
	else
	{
		wait();
	}
}

// Spacing after ! and inside while parentheses.
void
polling(bool bStopped)
{
	while ( ! bStopped )
	{
		run();
		bStopped = true;
	}
}

// Stock conditional-parenthesis spacing also affects for headers.
void
counting(int count)
{
	for (int index = 0; index < count; ++index)
		step(index);
}

// Constructor initializers: colon on the signature, one initializer per line.
x::x() :
	a(2),
	b(3),
	c(4)
{
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void
xxx::Shutdown()
{
}

template<class X>
blah
xxx<X>::fooy()
{
}

// Short lambdas stay inline when they fit within the column limit.
void
xxx::callbacks()
{
	x([this] {});
	x([this] { run(); });
	m_cvThread.wait(lock, [this] { return m_bThreadExit || m_bSystemTimerReceived; });
}

// Calls stay compact until wrapping is needed, then use one argument per line.
void
arguments()
{
	x(1, 2, 3, 504);
	x(firstLongReadinessConditionIsSatisfied(),
	  secondLongReadinessConditionIsSatisfied(),
	  firstLongReadinessConditionIsSatisfied(),
	  504);
}

void
assertionSpacing()
{
	ASSERT( expression );
	VERIFY( expression );
	x(y);
}

// Nonempty braced initializers have spaces inside the braces.
void
bracedInitializers()
{
	int x{ 0 };
	int values[]{ 1, 2, 3 };
	int empty{};
}

// Parameters stay compact until wrapping is needed, then use one per line.
void shortParameters(int first, int second, int third);
void wrappedParameters(int firstParameterWithALongName,
					   int secondParameterWithALongName,
					   int thirdParameterWithALongName,
					   int fourth);
void
wrappedDefinition(int firstParameterWithALongName,
				  int secondParameterWithALongName,
				  int thirdParameterWithALongName,
				  int fourth)
{
	run();
}
void
functionNameLongEnoughToForceAllParametersOntoTheNextLineWithoutPackingThemTogetherEvenThoughTheyWouldFitThereTogether(
	int a,
	int b,
	int c);
