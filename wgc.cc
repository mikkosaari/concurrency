/* A model of the wolf, sheep, and cabbage problem. These three and a man are
on the left bank of a river. There is a boat that is only big enough for the
man and one of the three (so the boat may be in five states: empty, contains
the man, or contains the man and one of the three). The man must not leave the
wolf and the sheep together unattended, because otherwise the wolf eats the
sheep. The same applies to the sheep and the cabbage. How can the man get all
three safely to the opposite side of the river?

Try this also with the option try_forward.

Antti Valmari 2014-10-14 */

/*
Encoding of state variables:
  0 = left bank
  1 = boat going right
  2 = right bank
  3 = boat going left
*/
state_var man(2), wolf(2), sheep(2), cabb(2);

void print_place( unsigned pl ){
  if( man   == pl ){ std::cout << 'M'; }else{ std::cout << ' '; }
  if( wolf  == pl ){ std::cout << 'W'; }else{ std::cout << ' '; }
  if( sheep == pl ){ std::cout << 'S'; }else{ std::cout << ' '; }
  if( cabb  == pl ){ std::cout << 'C'; }else{ std::cout << ' '; }
}
void print_state(){
  print_place(0);
  if( man == 1 ){ std::cout << " |> "; print_place(1); std::cout << " >| "; }
  else if( man == 3 )
                { std::cout << " |< "; print_place(3); std::cout << " <| "; }
  else{ std::cout << " |        | "; }
  print_place(2);
  std::cout << '\n';
}

/* This does not test eating but that the solution is ready. */
#define chk_state
const char *check_state(){
  if( wolf != 2 || sheep != 2 || cabb != 2 ){ return 0; }
  return "All on the right bank!";
}

/* From left bank to boat, from boat to right bank and so on. */
void move( state_var &xx ){
  if( xx < 3 ){ ++xx; }else{ xx = 0; }
}

unsigned nr_transitions(){ return 4; }

/*
The enabling conditions are based on the following:
- Whatever is with the man, cannot eat or be eaten.
- Whatever are in different places cannot eat each other.
- The man cannot leave anything alone on the boat.
*/
bool fire_transition( unsigned tr ){
  switch( tr ){

  /* The man moves alone */
  case 0:
    if(
      wolf != sheep && sheep != cabb &&
      !(wolf % 2) && !(sheep % 2) && !(cabb % 2) // (x%2) <=> x is on the boat
    ){
      move( man ); return true;
    }return false;

  /* The man moves with the wolf */
  case 1:
    if( man == wolf && sheep != cabb ){
      move( man ); move( wolf ); return true;
    }return false;

  /* The man moves with the sheep */
  case 2:
    if( man == sheep ){
      move( man ); move( sheep ); return true;
    }return false;

  /* The man moves with the cabbage */
  case 3:
    if( man == cabb && wolf != sheep ){
      move( man ); move( cabb ); return true;
    }return false;

  }
  return false;
}

