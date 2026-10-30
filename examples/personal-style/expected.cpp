// Function declarations do not receive separators.
void prepare();
void run();
void wait();
void step(int index);
bool firstLongReadinessConditionIsSatisfied();
bool secondLongReadinessConditionIsSatisfied();

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Simple if/else: optional braces can be removed.
void
simple(bool ready)
{
	if ( ready )
		run();
	else
		wait();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// A wrapped condition requires braces around both single-statement bodies.
void
wrappedCondition()
{
	if ( firstLongReadinessConditionIsSatisfied() &&
		 secondLongReadinessConditionIsSatisfied() &&
		 firstLongReadinessConditionIsSatisfied() )
	{
		run();
	}
	else
	{
		wait();
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Stock conditional-parenthesis spacing also affects for headers.
void
counting(int count)
{
	for (int index = 0; index < count; ++index)
		step(index);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
template<class X>
blah
xxx<X>::fooy()
{
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Short lambdas stay inline when they fit within the column limit.
void
xxx::callbacks()
{
	x([this] {});
	x([this] { run(); });
	m_cvThread.wait(lock, [this] { return m_bThreadExit || m_bSystemTimerReceived; });
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void
assertionSpacing()
{
	ASSERT( expression );
	VERIFY( expression );
	x(y);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void
switchSpacing(int value)
{
	switch (value)
	{
	case 0:
		ASSERT( value );
		break;
	default:
		VERIFY( value );
		break;
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void
switchBlocks(int value)
{
	switch (value)
	{
	case 0:
	case 1:
		{
			ASSERT( value );
			break;
		}
	case 2:
		{
			switch (value)
			{
			case 3:
				{
					VERIFY( value );
					break;
				}
			default:
				break;
			}
			break;
		}
	default:
		{
			VERIFY( value );
		}
		break;
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void
shortEnum()
{
	enum { TIMEOUT_INTERVAL = 4 };
}

// Access specifiers align with the class braces; members indent one level.
class AccessExample
{
public:
	void run();

protected:
	int value;

private:
	void reset();
};

// Trailing comments keep their original tabs and spaces.
int firstComment;	// First.
int secondComment;		// Second.
int mixedComment; 	  // Mixed spacing.

// Namespace closing braces do not receive generated comments.
namespace Example
{
	int first;
	int second;
	int third;
}

// Lambda parameters have a space after the capture list.
auto capturedLambda = [value] (int argument) { return value + argument; };
auto emptyLambda = [] () {};
auto genericLambda = []<class T> (T value) { return value; };

// Multiline enum braces use Allman placement.
enum
{
	RUNDOWN_RECV_EOF = (1 << 0),
	RUNDOWN_SENT_EOF = (1 << 1),
	RUNDOWN_CLOSED = (1 << 2),
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void
shortChunkEnum()
{
	enum { CHUNK_SIZE = 64 };
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// C-style casts have a space before the expression.
void
castSpacing(void* foo)
{
	consume((char**) foo);
	consume((int) 42);
}

// Wrapped concepts align each requirement with the first.
template<class T> concept
LsaString = std::same_as<T, STRING> ||
			std::same_as<T, UNICODE_STRING> ||
			std::same_as<T, LSA_STRING> ||
			std::same_as<T, LSA_UNICODE_STRING>;
template<class T> concept Small = std::same_as<T, int> || std::same_as<T, char>;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Wrapped logical expressions put each operand on its own line.
void
logicalWrapping()
{
	if ( firstLongReadinessConditionIsSatisfied() &&
		 secondLongReadinessConditionIsSatisfied() &&
		 firstLongReadinessConditionIsSatisfied() )
	{
		run();
	}
}

// Requires blocks expand before the concept header needs to wrap.
template<class T> concept HasOperations = requires(T value)
{
	value.foo();
	value.bar();
};

// Prefer wrapping after concept before splitting logical requirements.
template<class T> concept
MediumLengthConceptWithAdditionalDescription = std::same_as<T, UNICODE_STRING> || std::same_as<T, LSA_UNICODE_STRING>;

// Requires expressions after logical operators return to declaration indentation.
template<class T, class C> concept CompatibleLsaString = LsaString<T> &&
														 StringCharacter<C> &&
requires(T& value)
{
	{ value.Buffer } -> std::same_as<C*&>;
};
template<class S> concept StringLike = requires(const S& source)
{
	source.data();
	source.size();
};
