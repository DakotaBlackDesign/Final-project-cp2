# Aphos App

A mobile interface for generating sound in responce to a users breathing

## Summary

To accompany My thesis project The Aphos App will provide a simple graphical interface on an android device. 
I have hardware that interprets inhaling and exhaling into MIDI BLE. Thae App will interpret the midi and via a synth generate sound. User input will be collected through the accelerometer and a touch interface to modulate this sound and send it via bluetooth to headphones. 


## Component Parts

the hardware is finished and working as an arduino prototype. the project will only be the App. 

## Challenges

I would like to build the app using Jasvascript/ react to leverage my existing knowledge. the challenge will be adapting these languages to run on a mobile device. 

## Timeline

- Week 1: Write proposal
- Week 2: prototype basic functions.
            -recieve Middi ble data
            -digital syth 
            -output sound over bluetooth
- Week 3: prototype user inputs
            -accelerometer
            -touch interactions 
- Week 4: Combine user inputs with synth!
- Week 5: Present!

## References and link

react native documentation
https://facebook.github.io/react-native/docs/tutorial

react native sensor input example
https://medium.com/react-native-training/using-sensors-in-react-native-b194d0ad9167

midi in react
https://www.npmjs.com/package/midi-sounds-react

sample based synthesis
https://www.npmjs.com/package/webaudiofont

android synth tutorial
https://codelabs.developers.google.com/codelabs/making-waves-1-synth/#0

Ble and react native tutorial
https://blog.expo.io/so-you-want-to-build-a-bluetooth-app-with-react-native-and-expo-6ea6a31a151d
