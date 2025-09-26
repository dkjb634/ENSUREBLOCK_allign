When a macro like this is used in Rider IDE multiple times in a row
```c++
#define ENSURE_BLOCK(expression)                                    \
	if (expression)                                             \
	{                                                           \
		std::cout << "Error: " << #expression << std::endl; \
	}                                                           \
	else
```
Like this:
```c++
	ENSURE_BLOCK(condition1)
	ENSURE_BLOCK(condition2)
	ENSURE_BLOCK(condition3)
	{
	    ...
	}
```
Invoking reformat from Rider makes indentation to be:
```c++
	ENSURE_BLOCK(false)
		ENSURE_BLOCK(5 > 10)
			ENSURE_BLOCK(this == nullptr)
			{
				...
			}
```
This is by design. Rider builds Abstract Syntax Tree and uses it to express formatting.
However, one might want to avoid this behavior.
Since macro expands to `if(condition) {} else` and the next line expands to the similar code snippet, the code appears to be:
```c++
if(condition1) {} else
    if(condition2) {} else
        if(condition3) {} else
        {
        }
```
As long as `else` and `if` are on different lines, the `if` is logically indented.

### Available options:
Unfortunately, there is no way to make Rider formatter to avoid this behavior. However one can:
* Switch formatter engine to ClangFormat (in *Settings | Editor | Code Style | C/C++ | General*). ClangFormat works differently and doesn't add this indentation for subsequential macro calls (that uses dangling else)
* Or use the File Watcher feature. It is possible to make Rider invoke an external script on file save.

This script looks for multiple usages of ENSURE_BLOCK(condition) macro and alligns indentation for usages starting from second (using the indentation level of the first ENSURE_BLOCK usage met).
It also changes indents for the corresponding code block that follows after several usages of ENSURE_BLOCK macro.

### How To Configure
* First: clone and build the current project, so that you have the executable that you can launch;
* Then follow the steps demonstrated on the screenshot `ENSURE_BLOCK_instruction.png`