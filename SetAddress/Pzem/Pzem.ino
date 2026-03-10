/*
 * PZEM-004T Test Code for ESP32 (2-Wire Configuration)
 * 
 * Wiring:
 * PZEM-004T to ESP32:
 * - PZEM TX -> ESP32 GPIO16 (RX2)
 * - PZEM RX -> ESP32 GPIO17 (TX2)
 * - PZEM 5V -> ESP32 5V (VIN)
 * - PZEM GND -> ESP32 GND
 * 
 * AC Load Connection (2-wire):
 * - PZEM Live In -> AC Live
 * - PZEM Live Out -> Load
 * - Load return goes directly back to AC source (no neutral through PZEM)
 */

#include <PZEM004Tv30.h>

// Use Hardware Serial2 on ESP32
// Try different GPIO pins if these don't work
// Common alternatives: RX=16,TX=17 or RX=25,TX=26 or RX=32,TX=33
#define PZEM_RX 16  // Change if needed
#define PZEM_TX 17  // Change if needed

PZEM004Tv30 pzem(Serial2, PZEM_RX, PZEM_TX);

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  
  Serial.println("\n\n========================================");
  Serial.println("PZEM-004T Test Program for ESP32");
  Serial.println("2-Wire Configuration (Live only)");
  Serial.println("========================================");
  Serial.println();
  
  delay(1000);
  
  // Print pin configuration
  Serial.print("Using GPIO pins - RX: ");
  Serial.print(PZEM_RX);
  Serial.print(", TX: ");
  Serial.println(PZEM_TX);
  Serial.println();
  
  // Test communication multiple times
  Serial.println("Testing PZEM communication...");
  Serial.println("Attempting 5 times...");
  
  float voltage = NAN;
  for (int i = 0; i < 5; i++) {
    Serial.print("Attempt ");
    Serial.print(i + 1);
    Serial.print(": ");
    voltage = pzem.voltage();
    if (!isnan(voltage)) {
      Serial.println("Success!");
      break;
    }
    Serial.println("Failed");
    delay(500);
  }
  
  if (isnan(voltage)) {
    Serial.println("❌ ERROR: Cannot communicate with PZEM!");
    Serial.println("\nCheck connections:");
    Serial.println("- PZEM TX -> ESP32 GPIO16 (RX2)");
    Serial.println("- PZEM RX -> ESP32 GPIO17 (TX2)");
    Serial.println("- PZEM 5V -> ESP32 VIN (5V)");
    Serial.println("- PZEM GND -> ESP32 GND");
    Serial.println("\n⚠ COMMON ISSUES:");
    Serial.println("1. TX/RX might be swapped - try reversing them");
    Serial.println("2. PZEM needs 5V power (check if LED on PZEM is lit)");
    Serial.println("3. Try different GPIO pins (25/26 or 32/33)");
    Serial.println("4. Check if PZEM module is damaged");
    Serial.println("\n📝 To try different pins:");
    Serial.println("   Edit PZEM_RX and PZEM_TX at top of code");
  } else {
    Serial.print("✓ SUCCESS: PZEM communication OK! Voltage: ");
    Serial.print(voltage);
    Serial.println("V");
    Serial.println("Starting continuous readings...\n");
  }
  
  delay(2000);
}

void loop() {
  // Read all parameters
  float voltage = pzem.voltage();
  float current = pzem.current();
  float power = pzem.power();
  float energy = pzem.energy();
  float frequency = pzem.frequency();
  float pf = pzem.pf();
  
  // Display header
  Serial.println("========================================");
  Serial.print("Time: ");
  Serial.print(millis() / 1000);
  Serial.println(" seconds");
  Serial.println("========================================");
  
  // Check if readings are valid
  if (isnan(voltage)) {
    Serial.println("❌ ERROR: Failed to read from PZEM!");
    Serial.println("Check AC power and connections.\n");
  } else {
    // Display voltage
    Serial.print("Voltage:   ");
    Serial.print(voltage, 1);
    Serial.println(" V");
    
    // Display current
    Serial.print("Current:   ");
    Serial.print(current, 3);
    Serial.println(" A");
    
    // Display power
    Serial.print("Power:     ");
    Serial.print(power, 1);
    Serial.println(" W");
    
    // Display energy
    Serial.print("Energy:    ");
    Serial.print(energy, 3);
    Serial.println(" kWh");
    
    // Display frequency
    Serial.print("Frequency: ");
    Serial.print(frequency, 1);
    Serial.println(" Hz");
    
    // Display power factor
    Serial.print("PF:        ");
    Serial.println(pf, 2);
    
    Serial.println();
    
    // Status indicators
    if (current > 0.01) {
      Serial.println("✓ Load is connected and drawing current");
    } else {
      Serial.println("⚠ No load detected (current = 0)");
    }
  }
  
  Serial.println("========================================\n");
  
  // Wait 2 seconds before next reading
  delay(2000);
}

/*
 * TROUBLESHOOTING TIPS:
 * 
 * 1. No communication (all NaN):
 *    - Check RX/TX connections (they should be crossed)
 *    - Verify 5V power supply to PZEM
 *    - Try swapping RX and TX pins
 * 
 * 2. Voltage shows but current is 0:
 *    - Make sure load is connected through PZEM
 *    - AC Live must go through PZEM sensor coil
 *    - Turn on the load device
 * 
 * 3. Erratic readings:
 *    - Use shorter wires between ESP32 and PZEM
 *    - Keep PZEM away from high voltage AC lines
 *    - Ensure good ground connection
 * 
 * 4. To reset energy counter:
 *    - Uncomment the line below in setup()
 */

// pzem.resetEnergy();  // Uncomment to reset energy counter on startup