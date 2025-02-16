import hid

# Open the HID device (Replace with your actual Vendor ID and Product ID)
dev = hid.device()
dev.open(0xCafe, 0x4004)  # Replace with your device's VID/PID

# Send a Feature Report (Resolution Multiplier)
feature_report = [0x48, 5]  # Report ID 0x48, Set resolution multiplier to 5
dev.send_feature_report(feature_report)

# Close the device
dev.close()
print("Resolution multiplier set to 5")
