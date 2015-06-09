# pi-rack
Real time audio effects with a RaspberryPi, with buttons and potentiometers integration.

# Setup

## RaspberryPi
A standard [Raspbian (Debian Wheezy)](http://downloads.raspberrypi.org/raspbian_latest) works just fine.

### USB Audio
Since the Pi doesn't have any audio inputs (and the quality of its output jack is rather bad), a USB audio adapter is required. There are tons of cheap options (under $5), the one I have is listed by `lsusb` as `ID 0d8c:013c C-Media Electronics, Inc. CM108 Audio Controller`.

Making it work properly wasn't easy. First, it must be set up so that it becomes the default soundcard. This is done by running `sudo nano /etc/modprobe.d/alsa-base.conf` and removing the comment that prevents `snd-usb-audio` from loading. It should end up looking like this:

`options snd-usb-audio index=0`

The default audio configuration must also be changed via `sudo nano /etc/asound.conf` to 

```
pcm.!default {
 type plug
 slave {
   pcm "hw:0,0"
 }
}
ctl.!default {
 type hw
 card 0
}
```

You should also update the Pi's firmware. Instructions to do so can be found [here](https://learn.adafruit.com/usb-audio-cards-with-a-raspberry-pi/instructions).

Finally, USB devices should be forced to run in 1.1 mode by running `sudo nano /boot/cmdline.txt` and adding `dwc_otg.speed=1` at the end. **This causes most keyboards to stop working on the RPi, so be sure to set up SSH or VNC remote access beforehand.**

### Pure Data
Simply install via `sudo apt-get install puredata pd-zexy`. Pure Data must be started from the command line, using `pd -lib zexy` (plus whatever other options you might use, such as `-nogui`). `pd-zexy` is required for the `multiplex~` and `demultiplex~` objects.

**Make sure the correct number of input channels is set on Pure Data (Media/Audio Settings).** The CM108 (as well as most cheap soundcards) only has one input channel, but the default value is 2. Failure to correct this setting will prevent Pure Data from properly reading the input signal from your USB interface.

## Headless desktop PC
I finally got tired of struggling to make the Pi's audio (in particular, the microphone inputs) work consistently. It can be made, and I successfully tried all of the effects the few times I got it working, but I don't want to waste any more time on the subject. Since neither size nor weight are constraints to me, I switched platforms to my old desktop PC, which was previously gathering dust in the basement.

It consists of an Asus M2N-MX SE Plus motherboard, and AMD Athlon X2 5200+ (clocked at 2.7 GHz), 2GB of RAM, and a 160GB HDD. I'm using the onboard sound card, and running Linux Mint 17.1 (Rebecca), with Xfce as a desktop environment. This setup is way overkill for the application, but it does leave me headroom to play with some more processor-intensive effects, and decrease system latency.

Getting the project running was very straightforward. Mint automatically installed drivers for the sound card, so all I had to do was clone the repo, install Pure Data and set the correct input and output devices. I expect this to be the case on most modern motherboards.

# Acknowledgements
## [Guitar Extended](https://guitarextended.wordpress.com/)
One of my main sources of inspiration, contained lots of helpful information, links and cool-sounding patches.
## [Do It Now! Laboratories](https://github.com/doitnowlabs/rpieffectbox)
Ben also built one of these, and gave me the idea to reuse an old piece of equipment (in his case, a multimeter) with an LCD screen to display and control the different effects.
