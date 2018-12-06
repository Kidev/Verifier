# Verify
Verify is a simple program that introduces a new file type: .vfy
It is a compressed tarball containing:
    - A header file, describing the data
    - Any kind of data: .txt, .mp4...
    - A signature
This software checks if the given .vfy file is made by someone you trust!
You just have to have all of your trusted senders public key in a folder,
And Verify does the rest for you !

This is still a work in progress: Verify is meant to be part of VLC.
Indeed, with Deepfakes making it impossible to distinguish real from fake videos,
I beleive it is important to make cryptography available to everyone, easily