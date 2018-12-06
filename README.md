# Verify
Verify is a simple program that introduces a new file type: .vfy

It is a compressed tarball containing:
* A header file, describing the data
* Any kind of data: .txt, .mp4...
* A signature

This software checks if the given .vfy file is made by someone you trust!
You just put all your trusted senders public keys in a folder, and Verify does the rest for you !

This is still a work in progress: Verify is meant to be part of VLC in the future. Indeed, with Deepfakes making it impossible to distinguish real from fake videos, I believe it is important to make cryptography available to everyone, easily
