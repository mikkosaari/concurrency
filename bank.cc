/* A model of a banking system. Antti Valmari 2017-09-14 */


/* State variables and model parameters */
state_var
  ATM, x,
  CH1, y,
  CH2, w,
  BANK, z, b,
  cash;
const unsigned b_original = 200;  // original balance of the account


/* Prints either the value or two spaces. */
void if_print( bool bb, unsigned val ){
  if( bb ){ std::cout << val; }
  else{ std::cout << "  "; }
}

/* Prints the state in human-readable form on one line. */
void print_state(){
  std::cout
    << "A= " << ATM << ' '; if_print( ATM == 2 || ATM == 4 || ATM == 5, x );
  std::cout
    << "  CH1= ";
       if( CH1 == 1 ){ std::cout << 'q' << y; }
       else if( CH1 == 2 ){ std::cout << 'd' << y; }
       else{ std::cout << "   "; }
  std::cout
    << "  CH2= ";
       if( CH2 == 1 ){ std::cout << 'y' << w; }
       else if( CH2 == 2 ){ std::cout << "no "; }
       else{ std::cout << "   "; }
  std::cout
    << "  B= " << BANK << ' '; if_print( BANK, z );
       std::cout << "  bal= "; std::cout.width(3); std::cout << b
    << "  cash= "; std::cout.width(3); std::cout << cash
    << '\n';
}


/* The channels */

bool send_CH1_q( unsigned val ){
  if( CH1 ){ return false; }
  CH1 = 1; y = val; return true;
}

bool send_CH1_done( unsigned val ){
  if( CH1 ){ return false; }
  CH1 = 2; y = val; return true;
}

bool rec_CH1_q( unsigned &val ){
  if( CH1 == 1 ){ val = y; y = 0; CH1 = 0; return true; }
  return false;
}

bool rec_CH1_done( unsigned &val ){
  if( CH1 == 2 ){ val = y; y = 0; CH1 = 0; return true; }
  return false;
}

bool send_CH2_y( unsigned val ){
  if( CH2 ){ return false; }
  CH2 = 1; w = val; return true;
}

bool send_CH2_n(){
  if( CH2 ){ return false; }
  CH2 = 2; return true;
}

bool rec_CH2_y( unsigned &val ){
  if( CH2 == 1 ){ val = w; w = 0; CH2 = 0; return true; }
  return false;
}

bool rec_CH2_n(){
  if( CH2 == 2 ){ CH2 = 0; return true; }
  return false;
}


/* Verification functions */

#define chk_may_progress
inline bool is_may_progress(){
  return !( ATM || CH1 || CH2 || BANK );
}

#define chk_state
inline const char *check_state(){
  if( !ATM && !BANK && ( b + cash < b_original ) ){
    return "The client lost";
  }
  return 0;
}


/* Illustrative names for transition numbers and their total number */
enum{
  ATM_1, ATM_2, BANK_1, BANK_2, CH1_loss, CH2_loss,
  tr_end_marker
};

/* Sets the original balance and tells the number of transitions. */
unsigned nr_transitions(){
  b = b_original;
  return tr_end_marker;
}

/* Firing of transition tr. The return value tells whether it was enabled. */
bool fire_transition( unsigned tr ){
  unsigned tmp;

  switch( tr ){

    case ATM_1: switch( ATM ){
      case 0: ATM = 1; return true;
      case 1: x = 20; ATM = 2; return true;
      case 2:
        if( send_CH1_q(x) ){ x = 0; ATM = 3; return true; }
        else{ return false; }
      case 3:
        if( rec_CH2_y(tmp) ){ x = tmp; ATM = 4; return true; }
        else
        if( rec_CH2_n() ){ ATM = 6; return true; }
        return false;
      case 4: ATM = 5; return true;
      case 5: if( cash + x < 256 ){ cash += x; } ATM = 8; return true;
      case 6: ATM = 7; return true;
      case 7: ATM = 0; return true;
      case 8:
        if( send_CH1_done(x) ){ x = 0; ATM = 0; return true; }
        else{ return false; }
      default: return false;
    }

    case ATM_2: switch( ATM ){
      case 1: x = 50; ATM = 2; return true;
      case 3: ATM = 7; return true;
      default: return false;
    }

    case BANK_1: switch( BANK ){
      case 0:
        if( rec_CH1_q(tmp) ){ z = tmp; BANK = 1; return true; }
        else{ return false; }
      case 1:
        if( z > b && send_CH2_n() ){ z = 0; BANK = 0; return true; }
        else if( z <= b && send_CH2_y(z) ){ BANK = 2; return true; }
        return false;
      case 2:
        if( rec_CH1_done(tmp) ){ z = tmp; BANK = 3; return true; }
        else{ return false; }
      case 3:
        if( b > z ){ b -= z; }else{ b = 0; }
        z = 0; BANK = 0; return true;
      default: return false;
    }

    case BANK_2:
      if( BANK == 2 ){ BANK = 0; return true; }else{ return false; }

    case CH1_loss:
      if( CH1 ){ CH1 = 0; y = 0; return true; }else{ return false; }

    case CH2_loss:
      if( CH2 ){ CH2 = 0; w = 0; return true; }else{ return false; }

  }
  return false; 
}

