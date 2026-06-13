# Virtual filesystem

```mermaid
flowchart
    root(["/ (ramfs)"]) --> home(["users (fat32)"])
    root --> mnt(["volumes"])
    mnt --> ext1(["ext_device (fat32)"])
```
