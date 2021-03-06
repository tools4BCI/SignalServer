Hardware Requirements
=====================

- CPU: at least 200 MHz

  (already tested on embedded systems, highly dependent on sampling rate and number of acquired channels)

- RAM: 32 MB

- Run the Signal Server with highest process priority

For networking usage:

- Ethernet min. 100 MBit (1 GBit recommended)

  Needed network connection bandwidth varying by the sampling rate and the number of channels acquired.

  **Recomendation**

  If the Signal Server is used via a network connection it is recommeded to use it behind a router.
  Otherwise traffic from other people could utilize a common network to its capacity and influence
  the transmission time of the Signal Servers data packets.
