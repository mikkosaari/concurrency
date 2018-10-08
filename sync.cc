/* A stubborn set model of the alternating bit protocol with failure and
recovery, with two message types 'N' and 'Y', and big channel capacities
  There are five components: Client, Sender, Data Channel, Receiver, and Ack
Channel. If check_out is on, then Client keeps track of the messages in
transmit and checks that the delivered messages are correct. If check_out is
off, then Client has no activity or memory of its own. There are also two
additional components that model loss of messages in the channels. One of them
can read the first message of non-empty Data Channel at any time, and the
other does the same with Ack Channel.
  Sender communicates with the client via send<N>, send<Y>, ok, and fail.
Receiver communicates with the client via rec<N> and rec<Y>. Sender sends to
Receiver via Data Channel messages of the form N<0>, N<1>, Y<0>, Y<1>, F<0>,
and F<1>. Receiver sends to Sender via Ack channel A<0> and A<1>.
  Instead of retransmission, Sender reports on failure if timeout occurs
before acknowledgement comes. Initially and after each failure, before sending
a new data message (N or Y), Sender sends a flush message (F) and waits for
its acknowledgement. This is to ensure that Sender and Receiver agree on the
value of the alternating bit and so to prevent loss and duplication of
messages.
  Antti Valmari 2015-10-04 */


#define terminate     // makes Sender capable of stopping for good
//#define compress      // channels work atomically (not as a chain of cells)
//#define check_out   // check the delivered messages (makes more states)

#ifdef size_par
const unsigned n=size_par;  // capacity of channels
#else
const unsigned n=4;
#endif

#ifdef compress           // Stubborn set search enters channel transitions
const bool chn = false;   // iff chn = true. If compress is true or channel
#else                     // capacity is 1, there are no channel transitions.
const bool chn = n > 1;
#endif

state_var               // Sm and Rm have: 0=N 1=Y
  Cc(2),                // most recently sent message content 0=none 1=N 2=Y
  CN(1), CY(1),         // because of failures, N and/or Y may be remnant
  Sc(3), Sm(1), Sb(1),  // Sender control, message and alteranting bit
  Rc(2), Rm(1), Rb(1),  // Receiver control, message and alteranting bit
  Dc[n] = 2, Db[n] = 1, // Data channel cont. 0=none 1=N 2=Y 3=F and alt. bit
  Ac[n] = 1, Ab[n] = 1; // Ack channel content and alternating bit

const char mch[] = { ' ', 'N', 'Y', 'F' };

void print_state(){
  std::cout << mch[ Cc ];
  if( CN ){ std::cout << 'N'; }else{ std::cout << ' '; }
  if( CY ){ std::cout << 'Y'; }else{ std::cout << ' '; }
  std::cout << ' ' << Sc << Sb << mch[ Sc && Sc < 4 ? Sm+1 : 0 ] << ' ';
  for( unsigned i = n; i--; ){
    if( Dc[i] ){ std::cout << mch[ Dc[i] ] << Db[i]; }
    else{ std::cout << "  "; }
  }
  std::cout << ' ' << Rc << Rb << mch[ Rc == 1 ? Rm+1 : 0 ] << ' ';
  for( unsigned i = n; i--; ){
    if( Ac[i] ){ std::cout << Ab[i]; }
    else{ std::cout << " "; }
  }
  std::cout << '\n';
}

#define chk_state
inline const char *check_state(){
  #ifdef check_out
  if( Rc != 1 ){ return 0; }
  if( !Cc && !CN && !CY ){ return "Unexpected message"; }
  if( Rm+1 == Cc ){ return 0; }
  if( ( CN && !Rm ) || ( CY && Rm ) ){ return 0; }
  return "Wrong message";
  #else
  return 0;
  #endif
}

#define chk_deadlock
inline const char *check_deadlock(){
  if(
    Sc == 7 && !Sm && !Rc && !Rm &&
    !Dc[0] && !Db[0] && !Ac[0] && !Ab[0] && !Cc
  ){ return 0; }
  return "Unexpected termination";
}

//#define chk_may_progress
inline bool is_may_progress(){
  #ifdef check_out
  return Rc == 1 && Rm+1 == Cc && !CN && !CY;
  #else
  return Rc == 1;
  #endif
}

//#define chk_must_progress
inline bool is_must_progress(){
  return !Sc;
}

void cmprD(){
  #ifdef compress
  unsigned j = 0;
  while( j < n && Dc[j] ){ ++j; }
  for( unsigned i = j+1; i < n; ++i ){
    if( Dc[i] ){ Dc[j] = Dc[i]; Dc[i] = 0; Db[j] = Db[i]; Db[i] = 0; ++j; }
  }
  #endif
}

void cmprA(){
  #ifdef compress
  unsigned j = 0;
  while( j < n && Ac[j] ){ ++j; }
  for( unsigned i = j+1; i < n; ++i ){
    if( Ac[i] ){ Ac[j] = Ac[i]; Ac[i] = 0; Ab[j] = Ab[i]; Ab[i] = 0; ++j; }
  }
  #endif
}

unsigned nr_transitions(){
  unsigned nr_tr = 2*n + 3;
  std::cout << "ch-capacity = " << n;
  #ifdef compress
  std::cout << " compress";
  #endif
  #ifdef terminate
  std::cout << " terminate"; ++nr_tr;
  #endif
  #ifdef check_out
  std::cout << " check_out";
  #endif
  #ifdef stubborn
  std::cout << " stubborn";
  #endif
  std::cout << '\n';
  return nr_tr;
}

bool fire_transition( unsigned tr ){

  /* Sender */
  if( tr == 0 ){
    switch( Sc ){
    case 0:   // send!N
      #ifdef check_out
      Cc = 1;
      #endif
      Sm = 0; Sc = 1; return true;
    case 1:   // flush!b
      if( Dc[n-1] ){ return false; }
      Dc[n-1] = 3; Db[n-1] = Sb; cmprD(); Sc = 2; return true;
    case 2:   // ack?b'  (if b == b' then swap b; goto 3)
      if( !Ac[0] ){ return false; }
      if( Sb == Ab[0] ){ Sb = !Sb; Sc = 3; }
      Ac[0] = 0; Ab[0] = 0; cmprA(); return true;
    case 3:   // data!m!b
      if( Dc[n-1] ){ return false; }
      Dc[n-1] = Sm+1; Db[n-1] = Sb; cmprD(); Sc = 4; Sm = 0; return true;
    case 4:   // ack?b'  (if b == b' then swap b; goto 5)
      if( !Ac[0] ){ return false; }
      if( Sb == Ab[0] ){ Sb = !Sb; Sc = 5; }
      Ac[0] = 0; Ab[0] = 0; cmprA(); return true;
    case 5:   // ok
      Sc = 6; Cc = 0; CN = 0; CY = 0; return true;
    case 6:   // send!N
      #ifdef check_out
      Cc = 1;
      #endif
      Sm = 0; Sc = 3; return true;
    default:
      return false;
    }
  }
  else if( tr == 1 ){
    switch( Sc ){
    case 0:   // send!Y
      #ifdef check_out
      Cc = 2;
      #endif
      Sm = 1; Sc = 1; return true;
    case 2: case 4:   // fail
      #ifdef check_out
      if( Cc == 1 ){ CN = 1; }else{ CY = 1; }
      #endif
      Cc = 0; Sm = 0; Sc = 0; return true;
    case 6:   // send!Y
      #ifdef check_out
      Cc = 2;
      #endif
      Sm = 1; Sc = 3; return true;
    default:
      return false;
    }
  }

  /* Receiver */
  else if( tr == 2 ){
    switch( Rc ){
    case 0:
      if( !Dc[0] ){ return false; }
      if( Dc[0] < 3 && Db[0] == Rb ){ Rm = Dc[0]-1; Rc = 1; }
      else{ Rb = Db[0]; Rc = 2; }
      Dc[0] = 0; Db[0] = 0; cmprD(); return true;
    case 1:
      #ifdef check_out
      if( ( CN && !Rm ) || ( CY && Rm ) ){ CN = 0; CY = 0; }
      else if( Rm+1 == Cc ){ Cc = 0; CN = 0; CY = 0; }
      else{ err_msg = "This should be impossible"; }
      #endif
      Rc = 2; Rm = 0; return true;
    case 2:
      if( Ac[n-1] ){ return false; }
      Ac[n-1] = 1; Ab[n-1] = Rb; cmprA(); Rc = 0; Rb = !Rb; return true;
    default:
      return false;
    }
  }

  /* Data loser */
  else if( tr == 3 ){
    if( !Dc[0] ){ return false; }
    Dc[0] = 0; Db[0] = 0; cmprD(); return true;
  }

  /* Ack loser */
  else if( tr == 4 ){
    if( !Ac[0] ){ return false; }
    Ac[0] = 0; Ab[0] = 0; cmprA(); return true;
  }

  /* Data channel */
  else if( tr < n+4 ){
    tr -= 4;
    if( !Dc[ tr ] || Dc[ tr-1 ] ){ return false; }
    Dc[ tr-1 ] = Dc[ tr ]; Dc[ tr ] = 0; Db[ tr-1 ] = Db[ tr ]; Db[ tr ] = 0;
    return true;
  }

  /* Ack channel */
  else if( tr < 2*n+3 ){
    tr -= n+3;
    if( !Ac[ tr ] || Ac[ tr-1 ] ){ return false; }
    Ac[ tr-1 ] = Ac[ tr ]; Ac[ tr ] = 0; Ab[ tr-1 ] = Ab[ tr ]; Ab[ tr ] = 0;
    return true;
  }

  #ifdef terminate
  /* Stopper */
  else{
    if( Sc == 0 || Sc == 6 ){ Sc = 7; Sb = 0; return true; }
    return false;
  }
  #endif

  return false;
}

void next_stubborn( unsigned tr ){

  if( tr == 0 ){
    switch( Sc ){
    case 0: case 6:
      #ifdef terminate
      stb(1,2,2*n+3); return;
      #else
      stb(1,2); return;
      #endif
    case 1: case 3:
      if( Dc[n-1] ){ if( chn ){ stb(n+3); }else{ stb(2,3); } }
      return;
    case 2: case 4:
      if( Ac[0] ){ stb(1,4); }
      else if( chn ){ stb(n+4); }else{ stb(2); }
      return;
    case 5:
      stb(2); return;
    case 7: return;
    }
  }

  else if( tr == 1 ){
    switch( Sc ){
    case 0: case 2: case 4: case 6:
      stb(0,2); return;
    case 1: case 3: case 5: case 7:
      stb(0); return;
    }
  }

  else if( tr == 2 ){
    switch( Rc ){
    case 0:
      if( Dc[0] ){ stb(3); }
      else if( chn ){ stb(5); }else{ stb(0); }
      return;
    case 1:
      stb(0,1); return;
    case 2:
      if( Ac[n-1] ){ if( chn ){ stb(2*n+2); }else{ stb(0,4); } }
      return;
    }
  }

  else if( tr == 3 ){
    if( Dc[0] ){ stb(2); }
    else if( chn ){ stb(5); }else{ stb(0); }
    return;
  }

  else if( tr == 4 ){
    if( Ac[0] ){ stb(0); }
    else if( chn ){ stb(n+4); }else{ stb(2); }
    return;
  }

  else if( tr < n+4 ){
    if( !Dc[ tr-4 ] ){ if( tr < n+3 ){ stb(tr+1); }else{ stb(0); } }
    else if( Dc[ tr-5 ] ){ if( tr > 5 ){ stb(tr-1); }else{ stb(2,3); } }
    return;
  }

  else if( tr < 2*n+3 ){
    if( !Ac[ tr-n-3 ] ){ if( tr < 2*n+2 ){ stb(tr+1); }else{ stb(2); } }
    else if( Ac[ tr-n-4 ] ){ if( tr > n+4 ){ stb(tr-1); }else{ stb(0,4); } }
    return;
  }

  else{
    stb(0);
  }

}

