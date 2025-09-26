using System;
using System.Collections.Generic;
using System.IO;
using System.Text.RegularExpressions;

class Program
{
    static void Main(string[] args)
    {
        if (args.Length == 0)
        {
            Console.WriteLine("Usage: program.exe <filepath>");
            return;
        }

        string filePath = args[0];
        
        if (!File.Exists(filePath))
        {
            Console.WriteLine($"File not found: {filePath}");
            return;
        }

        ProcessFile(filePath);
    }

    static void ProcessFile(string filePath)
    {
        string[] lines = File.ReadAllLines(filePath);
        bool modified = false;

        for (int i = 0; i < lines.Length - 1; i++)
        {
            string currentLine = lines[i];
            string nextLine = lines[i + 1];

            // Check if current line contains ENSURE_BLOCK
            if (ContainsEnsureBlock(currentLine))
            {
                // Check if next line also contains ENSURE_BLOCK
                if (ContainsEnsureBlock(nextLine))
                {
                    string currentIndentation = GetLeadingWhitespace(currentLine);
                    string nextLineContent = nextLine.TrimStart();
                    
                    // Replace the next line with current line's indentation
                    string alignedNextLine = currentIndentation + nextLineContent;
                    
                    if (lines[i + 1] != alignedNextLine)
                    {
                        lines[i + 1] = alignedNextLine;
                        modified = true;
                        Console.WriteLine($"Aligned line {i + 2}: {nextLineContent}");
                    }
                }
                // Check if next line starts with '{'
                else if (StartsWithOpeningBrace(nextLine))
                {
                    string currentIndentation = GetLeadingWhitespace(currentLine);
                    int blockEndIndex = FindMatchingClosingBrace(lines, i + 1);
                    
                    if (blockEndIndex != -1)
                    {
                        if (AlignCodeBlock(lines, i + 1, blockEndIndex, currentIndentation))
                        {
                            modified = true;
                        }
                    }
                }
            }
        }

        if (modified)
        {
            File.WriteAllLines(filePath, lines);
            Console.WriteLine($"File processed and saved: {filePath}");
        }
        else
        {
            Console.WriteLine("No changes needed.");
        }
    }

    static bool ContainsEnsureBlock(string line)
    {
        return Regex.IsMatch(line.TrimStart(), @"^ENSURE_BLOCK\s*\(");
    }

    static bool StartsWithOpeningBrace(string line)
    {
        return line.TrimStart().StartsWith("{");
    }

    static string GetLeadingWhitespace(string line)
    {
        int i = 0;
        while (i < line.Length && char.IsWhiteSpace(line[i]))
        {
            i++;
        }
        return line.Substring(0, i);
    }

    static int FindMatchingClosingBrace(string[] lines, int openBraceIndex)
    {
        int braceCount = 0;
        bool foundOpenBrace = false;

        for (int i = openBraceIndex; i < lines.Length; i++)
        {
            string line = lines[i];
            
            // Parse the line while respecting string literals
            bool inString = false;
            bool inChar = false;
            bool escaped = false;
            
            for (int j = 0; j < line.Length; j++)
            {
                char c = line[j];
                
                if (escaped)
                {
                    escaped = false;
                    continue;
                }
                
                if (c == '\\')
                {
                    escaped = true;
                    continue;
                }
                
                if (c == '"' && !inChar)
                {
                    inString = !inString;
                    continue;
                }
                
                if (c == '\'' && !inString)
                {
                    inChar = !inChar;
                    continue;
                }
                
                // Only count braces when not inside string or char literals
                if (!inString && !inChar)
                {
                    if (c == '{')
                    {
                        braceCount++;
                        foundOpenBrace = true;
                    }
                    else if (c == '}')
                    {
                        braceCount--;
                        
                        if (foundOpenBrace && braceCount == 0)
                        {
                            return i;
                        }
                    }
                }
            }
        }

        return -1;
    }

    static bool AlignCodeBlock(string[] lines, int startIndex, int endIndex, string targetIndentation)
    {
        bool modified = false;
        
        // Align opening brace to target indentation
        string openBraceContent = lines[startIndex].TrimStart();
        string newOpenBraceLine = targetIndentation + openBraceContent;
        
        if (lines[startIndex] != newOpenBraceLine)
        {
            lines[startIndex] = newOpenBraceLine;
            modified = true;
            Console.WriteLine($"Aligned opening brace at line {startIndex + 1}");
        }

        // Align closing brace to target indentation  
        string closeBraceContent = lines[endIndex].TrimStart();
        string newCloseBraceLine = targetIndentation + closeBraceContent;
        
        if (lines[endIndex] != newCloseBraceLine)
        {
            lines[endIndex] = newCloseBraceLine;
            modified = true;
            Console.WriteLine($"Aligned closing brace at line {endIndex + 1}");
        }

        // Analyze the content and rebuild with proper indentation
        string baseIndent = targetIndentation + "    "; // Base content indentation (4 spaces from target)
        
        // Process each line and calculate proper nesting
        Stack<int> braceStack = new Stack<int>();
        int currentNestingLevel = 1; // Start with 1 level inside the outer braces
        
        for (int i = startIndex + 1; i < endIndex; i++)
        {
            string line = lines[i];
            
            // Skip empty lines
            if (string.IsNullOrWhiteSpace(line))
                continue;

            string lineContent = line.TrimStart();
            
            // Check for closing braces at the start of this line
            if (lineContent.StartsWith("}"))
            {
                currentNestingLevel--;
                if (braceStack.Count > 0)
                    braceStack.Pop();
            }
            
            // Calculate indentation for this line
            string newIndentation = targetIndentation + new string(' ', currentNestingLevel * 4);
            string newLine = newIndentation + lineContent;
            
            if (lines[i] != newLine)
            {
                lines[i] = newLine;
                modified = true;
                Console.WriteLine($"Aligned content at line {i + 1}: '{lineContent}' (level {currentNestingLevel})");
            }
            
            // Check for opening braces in this line (after processing the line)
            bool inString = false;
            bool inChar = false;
            bool escaped = false;
            
            foreach (char c in lineContent)
            {
                if (escaped)
                {
                    escaped = false;
                    continue;
                }
                
                if (c == '\\')
                {
                    escaped = true;
                    continue;
                }
                
                if (c == '"' && !inChar)
                {
                    inString = !inString;
                    continue;
                }
                
                if (c == '\'' && !inString)
                {
                    inChar = !inChar;
                    continue;
                }
                
                if (!inString && !inChar && c == '{')
                {
                    currentNestingLevel++;
                    braceStack.Push(currentNestingLevel);
                }
            }
        }

        return modified;
    }
}