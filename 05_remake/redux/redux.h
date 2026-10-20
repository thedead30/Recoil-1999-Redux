// redux.h - Recoil Redux: optional enhancements on top of Recoil Classic (REDUX.md, 03_re/ledger/redux_backlog.csv).
// Every feature is a switch in redux.ini (the game folder). With every switch off nothing here touches the game: no
// function is replaced and the remake is Recoil Classic, byte for byte (prove_identity still holds for the exe).
#pragma once

namespace redux {

// Reads <game folder>\redux.ini and installs the enabled features. Call once, in the game folder, before the game starts.
// Messages ("redux: ...") go to stderr (boot.log in a player install). Returns the number of features enabled.
int Init();
bool InitHdVideo();   // RX-36 (redux_video.cpp)

}  // namespace redux
