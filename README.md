# EldenRingStutterFix

This is an unofficial patch for Elden Ring on PC which aims to fix the stuttering that a lot of players have experienced. It implements these VKD3D fixes natively in D3D12:

* https://github.com/HansKristian-Work/vkd3d-proton/commit/54fbadcc9405841cc018210ba6d1045e71f405c0
* https://github.com/HansKristian-Work/vkd3d-proton/commit/9817c52d2441bc6231afd2e5c472c7e7210a72c9
* https://github.com/HansKristian-Work/vkd3d-proton/commit/f39ece9a7ce579a56196455933a200ec04a9d8eb

# Installation

1. Download the [latest release](https://github.com/soupstream/EldenRingStutterFix/releases/latest)
2. Move `d3d12.dll` into your Elden Ring game folder (probably `C:\Program Files (x86)\Steam\steamapps\common\ELDEN RING\Game`)
3. Launch the game with EAC disabled
   * To do this, create the file `steam_appid.txt` with the text `1245620` in your game folder and run `eldenring.exe` directly

To uninstall, just delete `d3d12.dll` from your game folder.

# Testing

On my PC, the game didn't stutter in the first place so I haven't actually verified that this fix helps. In theory it should decrease stuttering, but won't necessarily improve performance otherwise.

For now, I logged some stats to make sure everything was working as intended. These were the stats after 15 minutes of playing:

```
Command allocators created:        241
Command allocator cache hits:      50276
CreatePipelineLibrary overrides:   1
CreateCommittedResource overrides: 109013
```

When running the game after a driver update, the CreatePipelineLibrary workaround is triggered once. The command allocator cache and CreateCommittedResource fix also look to be working, which saves on a lot of expensive memory operations.
