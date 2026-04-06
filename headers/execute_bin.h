
// bin_name: The name of the binary to execute (e.g., "ls").
// args: An array of strings representing the arguments to pass to the binary, including the binary, not including the binary
// args is an pointer to constant pointers to characters
extern void execute_bin(const char** args, const char** output_stream);
