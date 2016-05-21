#include <SPI.h>
#include <RF24.h>
#include <printf.h>

#define PYRUN_PIPE_HOST 0xF0F0F0F0E1LL
#define PYRUN_PIPE_NODE 0xF0F0F0F0D2LL
#define PYRUN_MAXLENGTH_SIGNAL 32

class pyrun_object {
  public:
  pyrun_object( int id, char type, int amount);

  pyrun_object* m_next;

  int m_ea[8];
  int m_id = 0;
  char m_type;
  int m_amount;
  private:
};

class pyrun {
  public:
    pyrun( int t_CE, int t_CS);

    ~pyrun();

    void setPins( int e1, int e2, int e3, int e4, int e5, int e6, int e7, int e8);

    void setClient() {
      m_radio->openWritingPipe( PYRUN_PIPE_HOST);
      m_radio->openReadingPipe(1, PYRUN_PIPE_NODE);
      m_radio->startListening();
      m_server = false;
    }
    void setServer() {
      m_radio->openWritingPipe( PYRUN_PIPE_NODE);
      m_radio->openReadingPipe(1, PYRUN_PIPE_HOST);
      m_radio->stopListening();
      m_server = true;
    }

    int create( char type, int amount) {
      pyrun_object *t_entity;
      pyrun_object* attach;
      m_number_index++;
      t_entity = new pyrun_object( m_number_index, type, amount);
      Serial.println( "ID erstellt");
      // Baumende suchen
      if( m_tree == NULL)
          m_tree = t_entity;
      else {
          attach = m_tree;
          while( attach->m_next != NULL)
              attach = attach->m_next;
          attach->m_next = t_entity;
      }
      return m_number_index;
    }


    void loop();

    void set( char type,int amount) {
      m_type = type;
      m_amount = amount;
    }
  private:
    RF24 *m_radio;
    bool m_server;
    int m_ea[8];
    int m_ports[8];
    int m_id = 0;
    char m_type;
    int m_amount;
    int m_number_index;
    int m_time;
    pyrun_object* m_tree;

    String inputString;         // a string to hold incoming data
    boolean stringComplete = false;
};



pyrun_object::pyrun_object( int id, char type, int amount)  {
    m_id = id;
    m_type = type;
    m_amount = amount;
    m_ea[0] = 0;
    m_ea[1] = 0;
    m_ea[2] = 0;
    m_ea[3] = 0;
    m_ea[4] = 0;
    m_ea[5] = 0;
    m_ea[6] = 0;
    m_ea[7] = 0;
    m_next = NULL;
}

pyrun::pyrun( int t_CE, int t_CS) {
    // Radio starten
    m_radio = new RF24( t_CE, t_CS);
    // begin
    m_radio->begin();
    //m_radio->setPALevel(RF24_PA_MAX);
    // set to client
    setClient();
    m_radio->enableDynamicPayloads();
    m_radio->setAutoAck( true ) ;
    // Optionally, increase the delay between retries & # of retries
    //m_radio->setRetries(2,15);
    // set speed
    m_radio->setDataRate( RF24_250KBPS );
    m_radio->powerUp();
    m_ea[0] = 0;
    m_ea[1] = 0;
    m_ea[2] = 0;
    m_ea[3] = 0;
    m_ea[4] = 0;
    m_ea[5] = 0;
    m_ea[6] = 0;
    m_ea[7] = 0;

    // tree reset
    m_tree = NULL;
    m_number_index = 0;

    // time
    m_time = 0;

    inputString.reserve(200);
}

pyrun::~pyrun() {
    delete m_radio;
}

void pyrun::setPins( int e1, int e2, int e3, int e4, int e5, int e6, int e7, int e8) {
  m_ports[ 0] = e1;
  pinMode( e1, OUTPUT);
  m_ports[ 1] = e2;
  pinMode( e2, OUTPUT);
  m_ports[ 2] = e3;
  pinMode( e3, OUTPUT);
  m_ports[ 3] = e4;
  pinMode( e4, OUTPUT);
  m_ports[ 4] = e5;
  pinMode( e5, OUTPUT);
  m_ports[ 5] = e6;
  pinMode( e6, OUTPUT);
  m_ports[ 6] = e7;
  pinMode( e7, OUTPUT);
  m_ports[ 7] = e8;
  pinMode( e8, OUTPUT);
  
}

void pyrun::loop() {
    // time erhöhen
    m_time++;
    if( m_time> 10000)
    m_time = 0;

    while (Serial.available()) {
      // get the new byte:
      char inChar = (char)Serial.read();
      // add it to the inputString:
      inputString += inChar;
      // if the incoming character is a newline, set a flag
      // so the main loop can do something about it:
      if (inChar == '\n') {
        stringComplete = true;
      }
    }

    if( stringComplete) {
        char msg3[PYRUN_MAXLENGTH_SIGNAL];
        inputString.toCharArray(msg3, PYRUN_MAXLENGTH_SIGNAL);
        m_radio->stopListening();
        m_radio->write( &msg3, strlen(msg3));
        m_radio->startListening();
        Serial.println( inputString);
        inputString = "";
        stringComplete = false;
    }
    
    if( m_server) {
    // Zuhören
    m_radio->startListening();
    delay(100); // bischen warten
    // falls was empfangen
    if( m_radio->available() ) {
      char t_message[PYRUN_MAXLENGTH_SIGNAL];
      int t_len = m_radio->getDynamicPayloadSize();

      // auslesen
      m_radio->read( &t_message, t_len);
      Serial.println( t_message);
      m_radio->stopListening();

      // char array ausanandernehmen
      int i = 0;
      char* t_list;
      char* t_id;
      char* t_type;
      char* t_amount;
      int t_ea[8];
      t_list = strtok( t_message, ";");
      while ( t_list != NULL) {
        i++;
        switch( i) {
            case 1: t_id = t_list; break;
            case 2: t_type = t_list; break;
            case 3: t_amount = t_list; break;
            case 4: t_ea[0] =  atoi(t_list); break;
            case 5: t_ea[1] =  atoi(t_list); break;
            case 6: t_ea[2] =  atoi(t_list); break;
            case 7: t_ea[3] =  atoi(t_list); break;
            case 8: t_ea[4] =  atoi(t_list); break;
            case 9: t_ea[5] =  atoi(t_list); break;
            case 10: t_ea[6] =  atoi(t_list); break;
            case 11: t_ea[7] =  atoi(t_list); break;
        }
        t_list = strtok(NULL, ";");
      }

      if( t_id[0] == '0') {
        Serial.write( "REGIST NEED\n");

        char msg[PYRUN_MAXLENGTH_SIGNAL];

        sprintf( msg,"R;%d;%d;", create( t_type[0], atoi(t_amount)), atoi(t_id));

        m_radio->write( &msg, strlen(msg));
      } else {
        // Senden
        char msg2[PYRUN_MAXLENGTH_SIGNAL];
        sprintf( msg2,"%d;%c;%d;%d;%d;%d;%d;%d;%d;%d;%d;", atoi(t_id), t_type[0], atoi(t_amount), 1, 2, 3, 4, 5 ,6, 7, 8);
        Serial.println( msg2);
        m_radio->write( &msg2, strlen(msg2));
      }
    }

     /*char msg[] = "E;2;0;0;";
     m_radio->write( &msg, strlen(msg));
     delay(10);
     Serial.write( "server ");
     Serial.println( strlen(msg) );
     Serial.write( "\n");*/
    } else {
      if (m_radio->available()) {

        int t_len=0;
        char t_message[PYRUN_MAXLENGTH_SIGNAL];
        t_len = m_radio->getDynamicPayloadSize();
        m_radio->read( &t_message, t_len);
        Serial.write( t_message);
        Serial.write( "\n");

        // auswerten
        int i = 0;
        char* t_list;
        char* t_id;
        char* t_type;
        char* t_amount;
        int t_ea[8] = { 0,0,0,0,0,0,0,0};
        t_list = strtok( t_message, ";");
        while ( t_list != NULL) {
          i++;
          if( i == 1)
            t_id = t_list;
          switch( i) {
            case 1: t_id = t_list; break;
            case 2: t_type = t_list; break;
            case 3: t_amount = t_list; break;
            case 4: t_ea[0] = atoi(t_list); break;
            case 5: t_ea[1] = atoi(t_list); break;
            case 6: t_ea[2] = atoi(t_list); break;
            case 7: t_ea[3] = atoi(t_list); break;
            case 8: t_ea[4] = atoi(t_list); break;
            case 9: t_ea[5] = atoi(t_list); break;
            case 10: t_ea[6] = atoi(t_list); break;
            case 11: t_ea[7] = atoi(t_list); break;
          }
          //Serial.println( t_list);      // Do your processing on each item here, or save it somewhere
          t_list = strtok( NULL, ";");
        }
        // node soll sich neu id bekommen
        if( t_id[0] == 'R' && m_id == atoi(t_amount))
          m_id = atoi(t_type);
        // wert setzen
        if( atoi(t_id) == m_id && t_type[0] == m_type) {
          for( int n = 0; n < 7; n++)
            m_ea[n] = t_ea[n];
        }
     }

     // regist need
     if( m_id == 0 && (m_time == 2000 || m_time == 8000)) {
         // Senden
         m_radio->stopListening();
         char msg2[PYRUN_MAXLENGTH_SIGNAL];
         sprintf( msg2,"%d;%c;%d;%d;%d;%d;%d;%d;%d;%d;%d;", m_id, m_type, m_amount, m_ea[0], m_ea[1], m_ea[2], m_ea[3], m_ea[4], m_ea[5], m_ea[6], m_ea[7]);
         Serial.println( msg2);
         m_radio->write( &msg2, strlen(msg2));
         m_radio->startListening();
       }
       if( m_time == 9999) {
         // Senden
         m_radio->stopListening();
         char msg2[PYRUN_MAXLENGTH_SIGNAL];
         sprintf( msg2,"%d;%c;%d;%d;%d;%d;%d;%d;%d;%d;%d;", m_id, m_type, m_amount, m_ea[0], m_ea[1], m_ea[2], m_ea[3], m_ea[4], m_ea[5], m_ea[6], m_ea[7]);
         Serial.println( msg2);
         //m_radio->write( &msg2, strlen(msg2));
         m_radio->startListening();
       }
       
        for( int i = 0; i < 8; i++) {
          digitalWrite( m_ports[i], m_ea[i]);
        }
 
    }
}


/*  Dont edit!
 *  Bitte nicht ändern
*/
pyrun* radio;

/* Typearten:
 *  I - Eingang
 *  O - Ausgänge
 *  S - Analogeingang
 *  A - Analogausgang
*/
char g_type = 'A';

/*  Anzahl der eingänge/ausgänge
 *  2
 *  4
 *  6
 *  8 (Max)
*/
char g_amount = 2;

void setup() {
  radio = new pyrun( 8, 9);
  radio->set( g_type, g_amount);
  radio->setPins( 2,3,4,5,6,7,11, 12);
  radio->setServer();
  Serial.begin(9600);
  printf_begin();
}

void loop() {
  radio->loop();
}



