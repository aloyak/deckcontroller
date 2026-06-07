# DeckController

Use your SteamDeck as a generic game controller on your PC through Wi-Fi and USB (TODO, hopefully) either in Windows or Linux with low latency and very high battery efficiency.

This project is divided into two parts, the driver that runs on the PC (the `target` folder) and emulates a controller, and the client (the `deck` folder) that runs on the SteamDeck and sends the input data to the driver.

Uses SDL3 for the controller input and network communication, and ImGui for the UI on the PC side.

## Setup

> Recommended to connect the deck to your PC through a hotspot if wifi is bad or unstable, but it should work on the same network as well

These are the steps to take to get it working for any platform (PLEASE READ):

First time:

1. Download the binaries for the steam deck side
2. Add the deck's side as a non-steam game and make it run with proton (tested with proton 9.0)
3. Run the PC side and then run the deck side, in that order or the PC won't find it


* currently working for a faster and nicer way to setup

From that point on:

1. Run the driver on your PC
2. Run the client on your deck

> Using gaming mode is preferred but it works either way

That's it!

## Performance

* **Latency:** From 1-5 ms + input delay up to 20 ms depending on the network, but it should be good enough for most games, consider using a hotspot on the PC

* **Battery:** 6 hours of continuous use by default (check recommended settings)


**Recommended Settings:** battery life goes up to 7 hours if frame limit is set to 10 FPS/Hz, Allow Tearing, Half Rate Shading ON, Set TDP Limit to minimum (3 Watts) and set the GPU clock to 200 Mhz. As tested these settings don't have a noticeable impact on the input latency, but they do reduce the battery consumption a little bit, remember to use per-game profiles!
