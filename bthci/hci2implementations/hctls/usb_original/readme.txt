This is an implementation of the Bluetooth USB HCTL for the Symbian
USB Host driver interface.

Note that this implementation is based on the classic "H2" 
specification, as such it is not completely compatible with the 
updated version published with the Bluetooth Core Specification 
3.0+HS (and onwards).

There are two distinct components:
	* The HCTL Plugin - this provides the Bluetooth transport 
	  layer adaptation implementation through the USB driver 
	  interface.
	* The FDC Plugin - this is receives the driver information 
	  from the USB Function Driver Framework, it validates the 
	  device before supplying the notifications of device
	  attachment/detachment to the HCTL.

Both these components must be present for the system to be
functional, they can be included in a ROM image with the provided
iby file:
	hctl_usb_original.iby

If the reference CoreHCI is in use then a simple macro definition
is all that is required to include the iby file.  The macro to use
is HCI_USB; below is an example of its use:
	buildrom ... -DHCI_USB

To configure the Bluetooth stack to use this driver, the CoreHCI
implementation must be appropriately configured to use this
implementation.  Typically this is through an ini file in the
ESock data cage - in the Symbian file system this would be located:

	z:\private\101f7989\bluetooth\corehci_symbian.ini

In this file you should indicate the HCTL Plugin implementation
UID (0x200345E7):

	hctl_uid= 0x200345E7

An example configuration is provided in the Symbian reference
CoreHCI:

	corehci_symbian_usboriginal.ini

This should be all that is required to configure the use of the
driver.  The FDF should automatically load the FDC plugin when
a Bluetooth device is attached to the USB bus.  When no device is
attached then behaviour of the Bluetooth stack is the same as if
Bluetooth controllers power had been switched off (i.e. through the
power control API).

