# Error codes

ProtOS's syscalls and function declarations follow a standardized integer response format. Usually, the first digit indicates the category of the error (e.g: filesystem, IPC, etc.) and the second digit indicates the actual error inside of the section.

Here are the different error codes, separated by their respective categories:

### General

| Index | Name                | Description   | Type                   |
| ----- | ------------------- | ------------- | ---------------------- |
| 0     | `PROTO_OK`          | Success       | `PROTO_STATUS_SUCCESS` |
| 1     | `PROTO_ERR_UNKNOWN` | Unknown error | `PROTO_STATUS_ERROR`   |

### Filesystem

| Index | Name                             | Description                   | Type                 |
| ----- | -------------------------------- | ----------------------------- | -------------------- |
| 11    | `PROTO_ERR_FILE_NOT_FOUND`       | File cannot be found          | `PROTO_STATUS_ERROR` |
| 12    | `PROTO_ERR_NOT_A_DIRECTORY`      | File is not a directory       | `PROTO_STATUS_ERROR` |
| 13    | `PROTO_ERR_IS_A_DIRECTORY`       | File is a directory           | `PROTO_STATUS_ERROR` |
| 14    | `PROTO_ERR_ALREADY_EXISTS`       | File already exists           | `PROTO_STATUS_ERROR` |
| 15    | `PROTO_ERR_FILE_UNAUTHORIZED_OP` | Unauthorized operation        | `PROTO_STATUS_ERROR` |
| 16    | `PROTO_ERR_MAX_FD_REACHED`       | File descriptor limit reached | `PROTO_STATUS_ERROR` |
| 17    | `PROTO_ERR_INVALID_FD`           | Invalid file descriptor index | `PROTO_STATUS_ERROR` |

### Misc.

| Index | Name                              | Description              | Type                    |
| ----- | --------------------------------- | ------------------------ | ----------------------- |
| 21    | `PROTO_ERR_INIT_FAILED`           | Initialization failed    | `PROTO_STATUS_CRITICAL` |
| 22    | `PROTO_ERR_OUT_OF_MEMORY`         | Out of memory            | `PROTO_STATUS_ERROR`    |
| 23    | `PROTO_ERR_INVALID_ARGUMENT`      | Invalid argument         | `PROTO_STATUS_ERROR`    |
| 24    | `PROTO_ERR_SYSCALL_OUT_OF_BOUNDS` | Syscall ID out of bounds | `PROTO_STATUS_ERROR`    |
| 25    | `PROTO_ERR_INVALID_CONTEXT`       | Invalid context          | `PROTO_STATUS_ERROR`    |

### ELF Loading

| Index | Name                        | Description                | Type                 |
| ----- | --------------------------- | -------------------------- | -------------------- |
| 31    | `PROTO_ERR_ELF_INVALID_HDR` | Invalid ELF Header         | `PROTO_STATUS_ERROR` |
| 32    | `PROTO_ERR_ELF_UNSUPPORTED` | Unsupported platform       | `PROTO_STATUS_ERROR` |
| 33    | `PROTO_ERR_ELF_CORRUPTED`   | Corrupted ELF executable   | `PROTO_STATUS_ERROR` |
| 34    | `PROTO_ERR_ELF_CANNOT_LOAD` | Cannot load ELF executable | `PROTO_STATUS_ERROR` |

### Processes

| Index | Name                          | Description       | Type                 |
| ----- | ----------------------------- | ----------------- | -------------------- |
| 41    | `PROTO_ERR_PROCESS_NOT_FOUND` | Process not found | `PROTO_STATUS_ERROR` |
