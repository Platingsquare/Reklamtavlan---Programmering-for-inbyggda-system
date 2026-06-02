#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// ============================================================================
// KONFIGURATION
// ============================================================================
#define DISPLAY_DURATION_MS 20000  
#define TICK_RATE_MS 1             

#define LCD_RS PD0
#define LCD_EN PD1
#define LCD_D4 PD4
#define LCD_D5 PD5
#define LCD_D6 PD6
#define LCD_D7 PD7

// ============================================================================
// DATASTRUKTURER
// ============================================================================

typedef enum {
    EFFECT_STATIC,
    EFFECT_SCROLL,
    EFFECT_BLINK
} Effect;

// Kundstruktur
typedef struct {
    const char* line1;   
    const char* line2;   
    uint32_t payment;
    uint8_t id;
    uint8_t has_line2;   
} Customer;

// Annonsstruktur (NY: payment tillagd!)
typedef struct {
    const char* line1;
    const char* line2;
    Effect effect;
    uint8_t customer_id;
    uint8_t has_line2;
    uint32_t payment; // <-- Detta saknades tidigare och orsakade felet
} Ad;

// ============================================================================
// KUNDDB (HÄR SKRIVER DU EXAKT VAD SOM SKA STÅ PÅ VARJE RAD)
// ============================================================================
Customer customers[] = {
    // 0: Internal (1000 kr)
    { "Synas har?", "IOT:s Reklambyra", 1000, 0, 1 },
    
    // 1: Harry (5000 kr) - 3 varianter
    { "Kop bil at Harry", "En god bilaffar!", 5000, 1, 1 },
    { "En god bilaffar!", "For Harry!", 5000, 1, 1 },
    { "Hederlige Harrys", "Bilar", 5000, 1, 1 },
    
    // 2: Farmor Anka (3000 kr)
    { "Kop paj hos", "Farmor Anka!", 3000, 2, 1 },
    { "Skynda innan", "Marten at alla!", 3000, 2, 1 },
    
    // 3: Svarte Petter (1500 kr)
    { "Let Petter bygga", "at dig!", 1500, 3, 1 },
    { "Bygga svart?", "Ring Petter!", 1500, 3, 1 },
    
    // 4: Langben (4000 kr)
    { "Mysterier?", "Ring Langben!", 4000, 4, 1 },
    { "Langben fixar", "biffen!", 4000, 4, 1 }
};

#define NUM_SCENARIOS 10 

// ============================================================================
// GLOBALT TILLSTAND
// ============================================================================
static volatile uint32_t system_ticks = 0;
static uint32_t display_start_time = 0;
static Ad current_ad;
static uint8_t last_customer_id = 255;
static uint8_t blink_toggle = 0;
static uint8_t scroll_offset = 0;
static uint8_t scroll_offset2 = 0; // För rad 2 scroll

// ============================================================================
// LCD DRIVRUTIN
// ============================================================================

void lcd_send_nibble(uint8_t nibble) {
    PORTD &= ~((1<<LCD_D4)|(1<<LCD_D5)|(1<<LCD_D6)|(1<<LCD_D7));
    if (nibble & 0x01) PORTD |= (1<<LCD_D4);
    if (nibble & 0x02) PORTD |= (1<<LCD_D5);
    if (nibble & 0x04) PORTD |= (1<<LCD_D6);
    if (nibble & 0x08) PORTD |= (1<<LCD_D7);
    PORTD |= (1<<LCD_EN);
    _delay_us(1);
    PORTD &= ~(1<<LCD_EN);
    _delay_us(50);
}

void lcd_write(uint8_t data, uint8_t rs) {
    if (rs) PORTD |= (1<<LCD_RS);
    else PORTD &= ~(1<<LCD_RS);
    lcd_send_nibble(data >> 4);
    lcd_send_nibble(data & 0x0F);
}

void lcd_command(uint8_t cmd) { lcd_write(cmd, 0); }
void lcd_data(char c) { lcd_write(c, 1); }

void lcd_init(void) {
    DDRD |= (1<<LCD_RS)|(1<<LCD_EN)|(1<<LCD_D4)|(1<<LCD_D5)|(1<<LCD_D6)|(1<<LCD_D7);
    _delay_ms(50);
    lcd_send_nibble(0x02); 
    _delay_ms(5);
    lcd_command(0x28); 
    _delay_us(100);
    lcd_command(0x0C); 
    _delay_us(100);
    lcd_command(0x06); 
    _delay_us(100);
    lcd_command(0x01); 
    _delay_ms(2);
}

void lcd_clear(void) { lcd_command(0x01); _delay_ms(2); }
void lcd_print(const char* str) { while (*str) lcd_data(*str++); }
void lcd_set_cursor(uint8_t col, uint8_t row) {
    uint8_t addr = (row == 0) ? 0x80 : 0xC0;
    lcd_command(addr + col);
}

// ============================================================================
// TIMER
// ============================================================================
ISR(TIMER1_COMPA_vect) { system_ticks++; }

void timer_init(void) {
    TCCR1A = 0;
    TCCR1B = (1 << WGM12); 
    TCCR1B |= (1 << CS11) | (1 << CS10); 
    OCR1A = 249; 
    TIMSK1 |= (1 << OCIE1A);
    sei();
}

// ============================================================================
// LOGIK
// ============================================================================

uint32_t get_total_weight(void) {
    uint32_t total = 0;
    for (uint8_t i = 0; i < NUM_SCENARIOS; i++) total += customers[i].payment;
    return total;
}

uint8_t select_scenario(void) {
    uint32_t total = get_total_weight();
    uint32_t rand_val = (uint32_t)rand() % total;
    uint32_t acc = 0;
    
    for (uint8_t i = 0; i < NUM_SCENARIOS; i++) {
        if (customers[i].id == last_customer_id) continue; 
        acc += customers[i].payment;
        if (rand_val < acc) return i;
    }
    
    for (uint8_t i = 0; i < NUM_SCENARIOS; i++) {
        if (customers[i].id != last_customer_id) return i;
    }
    return 0;
}

void pick_next_ad(void) {
    uint8_t idx = select_scenario();
    Customer* c = &customers[idx];
    
    current_ad.line1 = c->line1;
    current_ad.line2 = c->line2;
    current_ad.has_line2 = c->has_line2;
    current_ad.customer_id = c->id;
    current_ad.payment = c->payment; // Nu fungerar detta!
    
    // Bestäm effekt
    if (c->id == 3) { // Petter
        uint16_t minutes = (system_ticks / 1000) / 60;
        if (minutes % 2 == 0) current_ad.effect = EFFECT_SCROLL;
        else current_ad.effect = EFFECT_STATIC;
    } else {
        if (c->id == 0) current_ad.effect = EFFECT_STATIC;
        else {
            uint8_t eff = rand() % 3;
            current_ad.effect = (Effect)eff;
        }
    }
    
    last_customer_id = c->id;
    display_start_time = system_ticks;
    scroll_offset = 0;
    scroll_offset2 = 0;
    blink_toggle = 0;
}

// ============================================================================
// VISUALISERING
// ============================================================================

void render_display(void) {
    lcd_clear();
    
    // --- RAD 1 ---
    lcd_set_cursor(0, 0);
    
    if (current_ad.effect == EFFECT_BLINK) {
        blink_toggle = !blink_toggle;
        if (blink_toggle) {
            lcd_print(current_ad.line1);
        } else {
            for(int i=0; i<16; i++) lcd_data(' ');
        }
    } else if (current_ad.effect == EFFECT_SCROLL) {
        uint8_t len = strlen(current_ad.line1);
        if ((system_ticks % 300) == 0) {
            scroll_offset = (scroll_offset + 1) % (len > 16 ? len : 1);
        }
        for (uint8_t i = 0; i < 16; i++) {
            if (scroll_offset + i < len) lcd_data(current_ad.line1[scroll_offset + i]);
            else lcd_data(' ');
        }
    } else {
        lcd_print(current_ad.line1);
    }

    // --- RAD 2 ---
    lcd_set_cursor(0, 1);
    
    if (!current_ad.has_line2) {
        for(int i=0; i<16; i++) lcd_data(' ');
        return;
    }

    if (current_ad.effect == EFFECT_BLINK) {
        if (!blink_toggle) {
            for(int i=0; i<16; i++) lcd_data(' ');
            return;
        }
        lcd_print(current_ad.line2);
    } else if (current_ad.effect == EFFECT_SCROLL) {
        uint8_t len = strlen(current_ad.line2);
        if ((system_ticks % 300) == 0) {
            scroll_offset2 = (scroll_offset2 + 1) % (len > 16 ? len : 1);
        }
        for (uint8_t i = 0; i < 16; i++) {
            if (scroll_offset2 + i < len) lcd_data(current_ad.line2[scroll_offset2 + i]);
            else lcd_data(' ');
        }
    } else {
        lcd_print(current_ad.line2);
    }
}

// ============================================================================
// HUVUDPROGRAM
// ============================================================================

void run_billboard_logic(void) {
    timer_init();
    lcd_init();
    srand(system_ticks); 
    pick_next_ad();
    
    while (1) {
        if (system_ticks - display_start_time >= DISPLAY_DURATION_MS) {
            pick_next_ad();
        }
        render_display();
        _delay_ms(100);
    }
}

void setup() { run_billboard_logic(); }
void loop() {}
