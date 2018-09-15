# yaraEx

## Usage: yaraEx <arg1>, [arg2], [arg3]

`<arg1>`: Rules file to apply. This should be a full path.

`[arg2]`: Start address of the range to apply the rules to. If not specified, the disassembly selection will be used.

`[arg3]`: Size of the range to apply the rules to. When not specified, the whole page will be used.

## Remarks:

- To run a single command, define it in metadata section whose identifer must be "script".
- To run script file, define it in metadata section whose identifer must be "load".
- Support multiple "script" or "load" definitions. Running sequences rely on their defined orders.
- Yara variables like #pattern, @pattern[n], !pattern[n] will be automatically replaced to their real values
in the command text or the script file before running.

The script file path can be relative to the directory where the rules file resides.

All commands including the script file will be put together to run as one script file through DbgScriptLoad function.

`\r` and `\n` will be unescaped in the command text defined by "script" before writing into script file.

## Sample:
```
rule test {
    meta:
        script="log \"Hello yaraEx!\""
        load="d:\yaraEx.yar"

    strings:
        $pattern = ...
    
    condition:
        #pattern
}
```


# yarafind

## Usage: yarafind <arg1>, [arg2], [arg3]

`<arg1>`: The address to start searching from.

`<arg2>`: The byte pattern of yara to search for.

`[arg3]`: Size of the range to apply the rules to. When not specified, the whole page will be used.

>The $result is set to the number of occurrences.


# yarafindall

## Usage: yarafindall <arg1>, [arg2], [arg3]

`<arg1>`: The address to start searching from.

`<arg2>`: The byte pattern of yara to search for.

`[arg3]`: Size of the range to apply the rules to. When not specified, the whole page will be used.

>The $result is set to the number of occurrences.
> 
>The $result1 variable is set to the virtual address of the first address that matches the byte pattern.
> 
>All results will be shown in the gui reference view.
> 
>The $result1 variable is set to the virtual address of the first address that matches the byte pattern.
