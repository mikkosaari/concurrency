/* Antti's State Space Exploration Tool (ASSET)
Antti Valmari 2015-01-09

ASSET can check safety, deadlock, and two kinds of progress properties, called
"may progress" and "must progress". To check safety, the user must define a
function that tells whether the current state is good or bad. To check
deadlocks, the user must define a function that tells whether termination is
allowed in the current state. To check may progress, the user must define a
function that tells whether the current state is a may progress state. By
default, ASSET treats also all terminal states as may progress states, but
this can be switched off. ASSET checks that from every reachable state, a may
progress state is reachable. Checking of must progress is otherwise similar,
but ASSET checks that every cycle and every terminal state contains at least
one must progress state.

ASSET can use the symmetry method and/or the (strong) basic stubborn set
method to reduce the number of states that are constructed. However, their use
requires that the user or a preprocessor tool analyses the model and gives the
necessary symmetry representative function and/or stubborn set obligation
function. Furthermore, the stubborn set method is not guaranteed to find must
progress errors, and the finding of safety and may progress errors is only
guaranteed for "may-terminating" models, that is, models that cannot reach a
state from which no terminal state is reachable. If the stubborn set method
fails to find errors but the result cannot be trusted for the above reasons,
then ASSET gives a warning.

ASSET constructs the states in breadth-first order, to minimize the length of
counterexamples. If a safety or deadlock error is encountered during the
construction of states, ASSET reports the error and terminates. Otherwise, if
the user has defined a may progress function, then ASSET continues by checking
may progress. If that check fails, ASSET reports the error and terminates.
Otherwise, if the user has defined a must progress function, ASSET does the
corresponding check. Finally, if the stubborn set method is used for detecting
other than deadlock errors, ASSET checks whether the model is may-terminating.

As a counterexample to may or must progress, ASSET yields a minimal path to a
state that violates the progress property, followed by a path to a cycle or
terminal state, followed by the cycle or terminal state. With may progress,
the purpose is that by analysing the cycle or terminal state, the user finds a
point where the model cannot do what the user expects it to do.

The structural transitions of the model are known by numbers in the range 0,
1, ..., maximum-1. Even if two transitions are logically distinct, they can be
given the same number, if they are never simultaneously enabled. To improve
the readability of the counterexamples, it is recommended that transitions
that model "unusual" events (such as timeout or loss of a message) are given
bigger numbers than transitions that model the "usual" course of events.

When the stubborn set method is not used, ASSET tries transitions in
decreasing order of their numbers. That is, ASSET tries the "unusual" events
before the "usual" events. The goal is to detect safety and deadlock errors as
quickly as possible. This feature can be switched off. In the case of stubborn
sets, the starting points of stubborn set construction are tried in decreasing
order by default, but the order can be reversed.

The model is represented as a set of C++ functions. The state of the model is
represented via variables of the pre-defined type state_var. Because model
checking is memory-consuming, it is often advantageous to use as few bits for
each state variable as possible. To facilitate this, what looks like the
initial value of a state variable actually specifies the number of bits that
the state variable uses. If a state variable uses b bits, then its value is an
integer in the range 0, ..., 2^b-1. Its initial value is 0. The default number
of bits is 8.

The model must provide the following features.

  unsigned nr_transitions()
Returns the number of structural transitions. May also perform whatever
initialization the model needs before the construction of the state space,
such as setting the state into its initial value. By default, the initial
value is all zero.

  void print_state()
Prints the state in human-readable form, preferably on one line. Must not
change the state.

  bool fire_transition( unsigned tr )
Returns false if transition number tr is not enabled. Otherwise returns true
and changes the state as caused by the occurrence of the transition. Must not
change the state if returns false.

The model may provide the following features. Unless otherwise mentioned, they
must not change the state. To switch the feature on, the model must contain
the corresponding "#define ..." or the same effect must be obtained via other
means, such as compile-time options.

  #define chk_state
  const char *check_state()
Returns 0 if the state is good, and otherwise returns an error message.

  #define chk_deadlock
  const char *check_deadlock()
Returns 0 if it is okay to terminate in the current state, and otherwise
returns an error message.

  #define chk_may_progress
  bool is_may_progress()
Returns true if and only if the state is a user-defined may progress state. By
default, ASSET treats also terminal states as may progress states, but that
can be switched off. An error is reported if a state is found such that no may
progress state is reachable from it.

  #define chk_must_progress
  bool is_must_progress()
Returns true if and only if the state is a user-defined must progress state.
By default, ASSET treats also terminal states as must progress states, but
that can be switched off. An error is reported if a cycle or terminal state is
found that contains no must progress states.

  #define symmetry
  void symmetry_representative()
Employs the symmetry method. This function must map each state to a symmetric
state, by changing the current state. The more states in each symmetry
equivalence class are mapped to the same state, the better for reduction
results.

  #define stubborn
  void next_stubborn( unsigned tr )
Employs the stubborn set method. This function lists the (additional)
transitions that the stubborn set of the current state must contain, if tr is
in it. The list is specified by making at most one call to one of the
following:
  stb( t1, t2, t3 )   // the number of arguments may be from 0 to 4
  stb_all()           // the stubborn set must contain all transitions
Not making a call indicates that additional transitions need not be taken. It
is equivalent to the call stb(). In the case of stb( t1, t2, t3 ), it does not
matter whether tr itself is in the list.

The model may also declare an error by making "err_msg" contain a character
string. The stubborn set method is not guaranteed to find such errors.

The model has access to static bool functions is_initial() and is_zero() that
return true if and only if the current state is the initial state or has all
bits 0.

Model checking takes place by compiling and executing this file. The model is
assumed to be in the file "asset.model". It is a good idea to write a small
script that copies the model to "asset.model", runs the C++ compiler, and runs
the model checker.

Model checking can be controlled with the following additional "#define ..."
or compile-time options:
  only_typical  Prints a typical sequence of events. Does not model check.
  try_forward   Transitions are tried forward in the safety & deadlock stage.
  dl_not_may    Terminal states are not may progress states by default.
  dl_not_must   Terminal states are not must progress states by default.
  no_show_cnt   Does not show running number of states.
  show_cnt      Shows the number of states after each this many states.
  stop_cnt      Aborts state space construction after this many states.
  no_sanity_chk Sanity checks that catch modelling bugs are switched off in
                the most speed-critical parts of ASSET.
  no_progr_chk  Non-progress detection is not executed. Saves memory.
  hash_count    Number of bits used for indexing the hash table.
  size_par      Number that is shown in the analysis results, if no_show_cnt.
*/


#include <iostream>
#include <vector>


/* A description of a detected error is given via this. */
const char *err_msg = 0;

/* Data type for state variables */
/* Raw state data is in a vector of unsigned ints. State number i occupies the
  locations i * nr_words ... (i+1) * nr_words - 1. A state variable occupies
  some successive bits inside one location. */
class state_var{

  friend void use_state( unsigned );
  friend void fire_init( unsigned );
  friend unsigned int hash_try( bool );
  friend void store_initial_state();

  static std::vector<unsigned> st_data; // raw state data
  static bool started;      // true after declaring the state variables
  static unsigned nr_words; // number of words used by a state
  static unsigned tot_bits; // number of used bits in most recent word
  static unsigned state_nr; // number of the current state
  unsigned word,            // number of word where the state variable is
    shift, mask;            // for extracting the state var. from inside word

public:

  /* The constructor sets word, shift, and mask according to the used and now
    needed numbers of bits. The default is that one byte is needed. */
  state_var( unsigned nr_bits = 8 ){
    if( started ){
      err_msg = "State variables must not be created after start"; return;
    }
    if( nr_bits > sizeof( unsigned )*8 ){
      err_msg = "Too many bits in a state variable"; return;
    }
    if( tot_bits + nr_bits > sizeof( unsigned )*8 ){
      ++nr_words; tot_bits = 0;
    }
    word = nr_words - 1; shift = tot_bits; tot_bits += nr_bits;
    mask = ((1u << nr_bits) - 1) << shift;
  }

  /* Returns the value of a state variable as an unsigned int. */
  inline operator unsigned() const {
    return (st_data[ state_nr * nr_words + word ] & mask) >> shift;
  }

  /* Inputs the value from an unsigned int. */
  inline unsigned operator =( unsigned val ){
    #ifndef no_sanity_chk
    if( val << shift & ~mask ){
      err_msg = "Assigned an out of range value to a variable";
    }
    #endif
    st_data[ state_nr * nr_words + word ] &= ~mask;
    st_data[ state_nr * nr_words + word ] |= val << shift;
    return val;
  }

  /* Assignment between state variables must go via unsigned int. */
  inline unsigned operator =( const state_var &sv ){
    unsigned val = sv; *this = val; return val;
  }

  /* Implementations of some commonly used operators. Postfix ++ and -- are
    not implemented because of the need of a dummy variable. */
  inline unsigned operator ++(){
    *this = unsigned( *this ) + 1; return *this;
  }
  inline unsigned operator --(){
    *this = unsigned( *this ) - 1; return *this;
  }
  inline unsigned operator +=( unsigned val ){
    *this = unsigned( *this ) + val; return *this;
  }
  inline unsigned operator -=( unsigned val ){
    *this = unsigned( *this ) - val; return *this;
  }
  inline unsigned operator *=( unsigned val ){
    *this = unsigned( *this ) * val; return *this;
  }
  inline unsigned operator /=( unsigned val ){
    *this = unsigned( *this ) / val; return *this;
  }
  inline unsigned operator %=( unsigned val ){
    *this = unsigned( *this ) % val; return *this;
  }
  inline unsigned operator &=( unsigned val ){
    *this = unsigned( *this ) & val; return *this;
  }
  inline unsigned operator |=( unsigned val ){
    *this = unsigned( *this ) | val; return *this;
  }
  inline unsigned operator ^=( unsigned val ){
    *this = unsigned( *this ) ^ val; return *this;
  }

  /* True, iff the state is the initial state */
  inline static bool is_initial(){ return state_nr == 1; }

  /* True, iff all state variables are zero */
  inline static bool is_zero(){
    unsigned end = (state_nr + 1) * nr_words;
    for( unsigned ii = state_nr * nr_words; ii < end; ++ii ){
      if( st_data[ii] ){ return false; }
    }
    return true;
  }

};
std::vector<unsigned> state_var::st_data;
bool state_var::started(false);
unsigned state_var::nr_words(1);    // the first word is reserved ...
unsigned state_var::tot_bits(0);    // ... but is initially totally unused
unsigned state_var::state_nr(0);


/* Forward declarations of stubborn set obligation functions */
/* These cannot be under the control of "#define stubborn", because it may be
  that it is the model below that switches it on. */
inline void stb();
inline void stb( unsigned );
inline void stb( unsigned, unsigned );
inline void stb( unsigned, unsigned, unsigned );
inline void stb( unsigned, unsigned, unsigned, unsigned );
inline void stb_all();


/* The model under analysis comes from the user's file. */
namespace model{
  #include "asset.model"
}

/* Adjust no_progr_chk. */
#ifndef chk_must_progress
#ifndef chk_may_progress
#ifndef stubborn
#define no_progr_chk
#endif
#ifndef chk_state
#define no_progr_chk
#endif
#endif
#endif
#ifdef only_typical
#undef no_progr_chk
#endif

/* Numerical settings that the user may affect. */
const unsigned
  #ifdef show_cnt       // how often to show the number of states
  show_count = show_cnt,
  #else
  show_count = 1000,
  #endif
  #ifdef stop_cnt       // abort after this many states
  stop_count = stop_cnt,
  #else
  stop_count = 30000000,
  #endif
  #ifdef hash_count     // hash table size is 2^hash_bits
  hash_bits = hash_count;
  #else
  hash_bits = 23;
  #endif

/* Miscellaneous that must be early on in this file. */
unsigned nr_trans = 0;  // number of structural transitions in the model

/* Stubborn set obligation functions and their shared variables */
unsigned *stb_tr = 0;
bool stb_called = false;
inline void stb(){
  if( stb_called ){ err_msg = "stb called twice for the same transition"; }
  stb_called = true;
}
inline void stb( unsigned t1 ){
  stb();
  if( *stb_tr == ~0u ){ *stb_tr = t1; return; }
  *stb_tr = ~0u;
}
inline void stb( unsigned t1, unsigned t2 ){
  stb();
  if( *stb_tr == ~0u ){ *stb_tr = t1; return; }
  if( *stb_tr == t1 ){ *stb_tr = t2; return; }
  *stb_tr = ~0u;
}
inline void stb( unsigned t1, unsigned t2, unsigned t3 ){
  stb();
  if( *stb_tr == ~0u ){ *stb_tr = t1; return; }
  if( *stb_tr == t1 ){ *stb_tr = t2; return; }
  if( *stb_tr == t2 ){ *stb_tr = t3; return; }
  *stb_tr = ~0u;
}
inline void stb( unsigned t1, unsigned t2, unsigned t3, unsigned t4 ){
  stb();
  if( *stb_tr == ~0u ){ *stb_tr = t1; return; }
  if( *stb_tr == t1 ){ *stb_tr = t2; return; }
  if( *stb_tr == t2 ){ *stb_tr = t3; return; }
  if( *stb_tr == t3 ){ *stb_tr = t4; return; }
  *stb_tr = ~0u;
}
inline void stb_all(){
  stb();
  if( *stb_tr == ~0u ){ *stb_tr = 0; return; }
  if( ++*stb_tr == nr_trans ){ *stb_tr = ~0u; }
}


/* Nodes of the state space */
struct node_type{
  unsigned h_next;      // next in the hash list
  unsigned prev;        // the finding predecessor node of the current node
  #ifndef no_progr_chk
  unsigned e_cnt;       // counts remaining non-progress edges, etc.
  unsigned p_next;      // next in the progress search list
  unsigned ie_end;      // used for counting-sorting incoming edges
  #endif
};
std::vector< node_type > nodes; // (0 = end mark, so location 0 is unused)
unsigned nr_edges = 0;  // number of edges in the state space

/* Start using the state variables of state number ni. */
inline void use_state( unsigned ni ){ state_var::state_nr = ni; }

/* Copy state ni to first unused locations, so that it can be modified. */
inline void fire_init( unsigned ni ){
  unsigned
    jj = nodes.size() * state_var::nr_words,
    end = (ni + 1) * state_var::nr_words;
  for( unsigned ii = ni * state_var::nr_words; ii < end; ++ii, ++jj ){
    state_var::st_data[ jj ] = state_var::st_data[ ii ];
  }
  use_state( nodes.size() );
}


/* The hash table */
unsigned const hash_size = 1u << hash_bits;
unsigned hash_tbl[ hash_size ] = {};

/* Finding a state from or inserting it to the hash table */
/* This function assumes that the state is in the next locations of st_data.
  This function first finds the state in the hash table. If it is there, this
  function returns its index. Otherwise, if no_ins == true, this function
  returns 0. Otherwise, this function makes a new node for the state and
  returns its index. If no_ins == false, hash_was_new tells if the state was
  new. */
bool hash_was_new = false;
unsigned hash_try( bool no_ins ){

  /* Compute the hash value. */
  unsigned
    idx = 0,
    beg = nodes.size() * state_var::nr_words,
    end = beg + state_var::nr_words;
  for( unsigned ii = beg; ii < end; ++ii ){
    idx ^= state_var::st_data[ ii ];
    idx ^= idx >> hash_bits; idx *= 1234567; idx += 5555555;
    idx ^= idx >> hash_bits; idx *= 1234567; idx += 5555555;
  }
  idx &= hash_size-1;

  /* Find the state from the hash list, if it is there. */
  unsigned ni = hash_tbl[ idx ], ii = beg, jj = ni * state_var::nr_words;
  while( ni && ii < end ){
    if( state_var::st_data[ ii ] == state_var::st_data[ jj ] ){ ++ii; ++jj; }
    else{ ni = nodes[ ni ].h_next; ii = beg; jj = ni * state_var::nr_words; }
  }
  if( ni ){ hash_was_new = false; return ni; }
  else if( no_ins ){ return 0; }

  /* Add a node for the state to the hash table, and extend st_data. */
  ni = nodes.size();
  if( ni > stop_count ){
    err_msg = "Maximum number of states exceeded"; return ni;
  }
  nodes.resize( ni+1 );
  state_var::st_data.resize( (ni + 2) * state_var::nr_words );
  #ifndef no_progr_chk
  nodes[ ni ].e_cnt = 0;
  #endif
  nodes[ ni ].h_next = hash_tbl[ idx ]; hash_tbl[ idx ] = ni;
  hash_was_new = true; return ni;

}

inline unsigned hash_find(){ return hash_try( true ); }
inline unsigned hash_insert(){ return hash_try( false ); }


#ifdef no_show_cnt
const char *clean_eol = "";
#else
const char *clean_eol = "\033[K";
#endif


/* Prints the sequence of states from the initial state (or any state with no
  predecessor) to state number ni. */
void print_history( unsigned ni ){
  if( !ni ){ return; }
  print_history( nodes[ ni ].prev ); use_state( ni ); model::print_state();
}


/* Reports the error that has been found. */
void report_error( unsigned ni, const char *msg ){
  static bool reported = false;
  if( reported ){ return; }
  reported = true;
  std::cout << clean_eol; print_history( ni );
  std::cout << "!!! " << msg;
  if( err_msg && *err_msg ){ std::cout << ": " << err_msg; }
  std::cout << '\n';
}


/* Prints a typical sequence of events from node ni. Affects prev and e_cnt.
  Avoids old states whose e_cnt == 0. */
#ifdef no_progr_chk
void print_typical( unsigned, bool ){}
#else
void print_typical( unsigned ni, bool no_ins ){

  /* Find looping state. */
  unsigned nprev = 0;
  do{
    nodes[ ni ].e_cnt = ~0u; nodes[ ni ].prev = nprev; nprev = ni;
    fire_init( ni );
    for( unsigned tr = 0; tr < nr_trans; ++tr ){
      if( model::fire_transition( tr ) ){
        #ifdef symmetry
        model::symmetry_representative();
        #endif
        if( no_ins ){ ni = hash_find(); }
        else{
          ni = hash_insert();
          if( hash_was_new ){ nodes[ ni ].e_cnt = 1; }
        }
        if( ni && nodes[ ni ].e_cnt ){ tr = nr_trans; }
        else{ ni = nprev; fire_init( ni ); }
      }
      if( err_msg ){ report_error( ni, "Transition firing error" ); return; }
    }
  }while( nodes[ ni ].e_cnt != ~0u );

  /* Print path to loop and the loop. */
  std::cout << clean_eol; print_history( nodes[ ni ].prev );
  std::cout << "----------\n";
  nodes[ ni ].prev = 0; print_history( nprev );

}
#endif


/* Creates the initial state, etc. */
void store_initial_state(){
  state_var::started = true;

  /* Create the sentinel node (node 0) and room for firing transitions. */
  nodes.resize(1); state_var::st_data.resize( 2 * state_var::nr_words );

  /* Initialize the model and put the initial state to the hash table. */
  for(
    unsigned ii = state_var::nr_words; ii < 2 * state_var::nr_words; ++ii
  ){ state_var::st_data[ ii ] = 0; }
  use_state(1);
  nr_trans = model::nr_transitions();
  #ifdef symmetry
  model::symmetry_representative();
  #endif
  hash_insert(); nodes[1].prev = 0;
  if( err_msg ){ report_error( 1, "Initialization error" ); return; }
  if( nr_trans == ~0u ){
    err_msg = ""; report_error( 0, "Too many transitions" ); return;
  }

  /* Check the initial state. */
  #ifdef chk_state
  err_msg = model::check_state();
  if( err_msg ){ report_error( 1, "Safety error" ); return; }
  #endif

}


#ifndef no_progr_chk
bool bss_second = false;    // first or second firing of transitions
unsigned *iedges = 0;       // backward edges
#endif


/* Try to fire a transition. If success, process the resulting state and
  restore the state for firing the next transition. */
inline bool try_transition( unsigned n1, unsigned tr ){

  /* Try it, and just return failure if it was disabled. */
  bool enabled = model::fire_transition( tr );
  #ifndef no_sanity_chk
  if( err_msg ){
    report_error( n1, "Transition firing error" ); return false;
  }
  #endif
  if( !enabled ){ return false; }

  #ifdef symmetry
  model::symmetry_representative();
  #endif

  #ifndef no_progr_chk
  if( bss_second ){
    iedges[ nodes[ hash_find() ].ie_end++ ] = n1;
    fire_init( n1 ); return true;
  }
  #endif

  /* Add or find the state and add the edge to it to data structures. */
  unsigned n2 = hash_insert();
  ++nr_edges;
  #ifndef no_progr_chk
  ++nodes[ n2 ].e_cnt;
  #endif

  /* If the state is new, record its finding predecessor. */
  if( hash_was_new ){
    nodes[ n2 ].prev = n1;

    /* Check that the new state is good. */
    #ifdef chk_state
    err_msg = model::check_state();
    if( err_msg ){ report_error( n2, "Safety error" ); return true; }
    #endif

  }

  /* Restore the state to try the next transition, and report success. */
  fire_init( n1 ); return true;

}


/* Constructs the state space, detecting safety and deadlock errors. */
void build_state_space(){
  #ifndef no_show_cnt
  const char *progress_msg = " states constructed\n\033[F";
  #ifndef no_progr_chk
  if( bss_second ){ progress_msg = " states backwards-processed\n\033[F"; }
  #endif
  #endif

  /* Variables for finding stubborn sets */
  #ifdef stubborn
  unsigned
    *stub_try = new unsigned[ nr_trans ],   // transition tried in the node
    *stub_found = new unsigned[ nr_trans ], stub_nr = -1u,  // found "bit"
    *stub_dfs = new unsigned[ nr_trans ], dfs_cnt = 0,  // DFS stack
    *stub_scc = new unsigned[ nr_trans ], scc_cnt = 0,  // Tarjan's SCC stack
    *stub_min = new unsigned[ nr_trans ];   // backward-propagated node number
  #endif

  /* Investigate states in breadth-first order until the queue is empty. */
  for( unsigned q_first = 1; q_first < nodes.size(); ++q_first ){
    unsigned old_edges = nr_edges;  // for detecting terminal states

    /* Occasionally show the number of processed states. */
    #ifndef no_show_cnt
    if( q_first % show_count == 0 ){
      std::cout << q_first << progress_msg; std::cout.flush();
    }
    #endif

    /* Employ an "enabled was found" bit and a fresh "found"-number. */
    #ifdef stubborn
    bool some_fired = false;
    ++stub_nr;
    if( !stub_nr ){
      for( unsigned tr = 0; tr < nr_trans; ++tr ){ stub_found[ tr ] = 0; }
      ++stub_nr;
    }
    #endif

    /* Try all transitions as such or as starting points of a stubborn set. */
    fire_init( q_first );
    #ifdef try_forward
    for( unsigned tr = 0; tr < nr_trans; ++tr ){
    #else
    for( unsigned tr = nr_trans; tr--; ){
    #endif

      #ifndef stubborn
      try_transition( q_first, tr );
      if( err_msg ){ return; }

      #else

      /* Reject already tried transitions. */
      if( stub_found[ tr ] == stub_nr ){ continue; }

      /* Find a closed set of transitions that have not yet been tried. */
      stub_found[ tr ] = stub_nr; stub_try[ tr ] = ~0u; stub_min[ tr ] = 0;
      stub_dfs[0] = stub_scc[0] = tr; dfs_cnt = scc_cnt = 1;
      unsigned watchdog = 0;
      while( dfs_cnt ){

        /* Extract the transition and its next obligation neighbour. */
        unsigned t1 = stub_dfs[ dfs_cnt-1 ], sm1 = stub_min[ t1 ];
        stb_tr = &stub_try[ t1 ]; stb_called = false;
        model::next_stubborn( t1 );
        if( err_msg ){
          report_error( q_first, "Stubborn set error" ); return;
        }
        unsigned t2 = stub_try[ t1 ];

        /* If no more neighbours, then backtrack. */
        if( t2 == ~0u ){
          watchdog = 0; --dfs_cnt;

          /* If strong component is ready, mark and try its transitions. */
          if( stub_scc[ sm1 ] == t1 ){
            for( unsigned ii = sm1; ii < scc_cnt; ++ii ){
              stub_min[ stub_scc[ ii ] ] = ~0u;
              some_fired |= try_transition( q_first, stub_scc[ ii ] );
              if( err_msg ){ return; }
            }
            if( some_fired ){ break; }
            scc_cnt = sm1;
          }

          /* Otherwise backward-propagate the minimum index, if possible. */
          else if( dfs_cnt ){
            t2 = stub_dfs[ dfs_cnt-1 ];
            if( stub_min[ t2 ] > sm1 ){ stub_min[ t2 ] = sm1; }
          }

        }

        /* If the neighbour has not been seen, enter it. */
        else if( stub_found[ t2 ] < stub_nr ){
          watchdog = 0; stub_found[ t2 ] = stub_nr; stub_try[ t2 ] = ~0u;
          stub_min[ t2 ] = scc_cnt;
          stub_dfs[ dfs_cnt++ ] = stub_scc[ scc_cnt++ ] = t2;
        }

        /* Otherwise just backward-propagate the minimum index. */
        else{
          if( ++watchdog > nr_trans ){
            err_msg = "Same element twice in stb list";
            report_error( q_first, "Stubborn set error" ); return;
          }
          if( sm1 > stub_min[ t2 ] ){ stub_min[ t1 ] = stub_min[ t2 ]; }
        }

      }
      if( some_fired ){ break; }
      #endif

    }

    /* Check the state against deadlock errors. */
    #ifndef no_progr_chk
    if( !bss_second ){
    #endif
      if( old_edges == nr_edges ){
        #ifdef chk_deadlock
        use_state( q_first ); err_msg = model::check_deadlock();
        if( err_msg ){ report_error( q_first, "Illegal deadlock" ); return; }
        #endif
      }
    #ifndef no_progr_chk
    }
    #endif

  }

  /* Wipe out displayed running state count and clean up memory. */
  std::cout << clean_eol;
  #ifdef stubborn
  delete [] stub_try; delete [] stub_found; delete [] stub_dfs;
  delete [] stub_scc; delete [] stub_min;
  #endif

}


/* Define dummy or real non-progress detection functions, as needed. */
#ifdef no_progr_chk
void construct_input_edges(){}
void verify_progress( unsigned ){}
#else

/* Constructs a data structure for incoming edges of each node. */
void construct_input_edges(){
  iedges = new unsigned[ nr_edges ];

  /* Count the indices of incoming edges, using already computed counts. */
  nodes[0].ie_end = nodes[1].ie_end = 0;
  for( unsigned ni = 2; ni < nodes.size(); ++ni ){
    nodes[ ni ].ie_end = nodes[ ni-1 ].ie_end + nodes[ ni-1 ].e_cnt;
  }

  /* Re-generate the edges, putting them in iedges. */
  bss_second = true; build_state_space(); return;

}

/* Finds non-progress errors. */
void verify_progress( unsigned round ){

  /* Compute initial values of non-progress numbers depending on round. */
  for( unsigned ni = 1; ni < nodes.size(); ++ni ){ nodes[ ni ].e_cnt = 0; }
  if( round == 1 ){     // npn = the number of the output edges of the node.
    for( unsigned ei = 0; ei < nr_edges; ++ei ){
      ++nodes[ iedges[ ei ] ].e_cnt;
    }
  }else{                // npn = 0 if node has no output edges, otherwise 1.
    for( unsigned ei = 0; ei < nr_edges; ++ei ){
      nodes[ iedges[ ei ] ].e_cnt = 1;
    }
  }

  /* Make npn:s 0 for progress states and 1 for non-progress deadlocks. */
  if( round == 0 ){
    for( unsigned ni = 1; ni < nodes.size(); ++ni ){
      #ifdef dl_not_may
      if( !nodes[ ni ].e_cnt ){ nodes[ ni ].e_cnt = 1; }
      #endif
      if( nodes[ ni ].e_cnt ){
        use_state( ni );
        #ifdef chk_may_progress
        if( model::is_may_progress() )
        #endif
          { nodes[ ni ].e_cnt = 0; }
        if( err_msg ){
          report_error( ni, "may_progress modelling error" ); return;
        }
      }
    }
  }else if( round == 1 ){
    for( unsigned ni = 1; ni < nodes.size(); ++ni ){
      #ifdef dl_not_must
      if( !nodes[ ni ].e_cnt ){ nodes[ ni ].e_cnt = 1; }
      #endif
      if( nodes[ ni ].e_cnt ){
        use_state( ni );
        #ifdef chk_must_progress
        if( model::is_must_progress() )
        #endif
          { nodes[ ni ].e_cnt = 0; }
        if( err_msg ){
          report_error( ni, "must_progress modelling error" ); return;
        }
      }
    }
  }

  /* Backwards-propagate the information that a node with npn = 0 is reachable
    via all / at least one output edge. */
  unsigned p_list = 0;
  for( unsigned ni = 1; ni < nodes.size(); ++ni ){
    if( !nodes[ ni ].e_cnt ){ nodes[ ni ].p_next = p_list; p_list = ni; }
  }
  for( ; p_list; p_list = nodes[ p_list ].p_next ){
    for( unsigned
      ei = nodes[ p_list-1 ].ie_end; ei < nodes[ p_list ].ie_end; ++ei
    ){
      unsigned ni = iedges[ ei ];
      if( nodes[ ni ].e_cnt ){
        --nodes[ ni ].e_cnt;
        if( !nodes[ ni ].e_cnt ){
          nodes[ ni ].p_next = nodes[ p_list ].p_next;
          nodes[ p_list ].p_next = ni;
        }
      }
    }
  }

  /* Find and report a non-progress error, if exists. */
  for( unsigned ni = 1; ni < nodes.size(); ++ni ){
    if( nodes[ ni ].e_cnt ){

      /* Print history up to entering the state that violates progress. */
      std::cout << clean_eol; print_history( nodes[ ni ].prev );
      std::cout << "==========\n";

      /* Print a typical sequence of events starting at the error node. */
      print_typical( ni, true );

      /* Report the error. */
      err_msg = "";
      if( round == 0 ){
        report_error( 0, "May-type non-progress error" ); return;
      }else if( round == 1 ){
        report_error( 0, "Must-type non-progress error" ); return;
      }else if( round == 2 ){
        report_error(
          0, "State was reached from which termination is unreachable"
        );
        return;
      }

    }
  }

}
#endif


/* The main program */
/* Some calls to report_error ensure that every error is reported even if
  subroutines accidentally fail to report it. */
int main(){

  /* Catch errors in the declaration of the model. */
  if( err_msg ){ report_error( 0, "Cannot start model checking" ); return 0; }
  #ifndef chk_state
  #ifndef chk_deadlock
  #ifndef chk_must_progress
  #ifndef chk_may_progress
  err_msg = "No error detection function \"My_...\" defined";
  report_error( 0, "Will not start model checking" ); return 0;
  #endif
  #endif
  #endif
  #endif

  /* Initialize the model. */
  store_initial_state();
  if( err_msg ){ report_error( 0, "Initialization error" ); return 0; }

  #ifdef only_typical

  /* Print a typical execution to a cycle or terminal state. */
  print_typical( 1, false );
  if( err_msg ){ report_error( 0, "Error" ); }
  std::cout << nodes.size()-1 << " states\n"; return 0;

  #else

  /* Do the requested model checking tasks. */
  build_state_space();
  if( !err_msg ){ construct_input_edges(); }
  #ifdef chk_may_progress
  if( !err_msg ){ verify_progress(0); }
  #endif
  #ifdef chk_must_progress
  if( !err_msg ){ verify_progress(1); }
  #endif
  #ifdef stubborn
  if( !err_msg ){ verify_progress(2); }
  #ifdef chk_must_progress
  if( !err_msg ){
    err_msg = "Must progress is unreliable with stubborn sets";
  }
  #endif
  #endif

  #ifndef chk_deadlock
  #ifndef chk_must_progress
  #ifndef chk_may_progress
  if( !err_msg ){
    err_msg = "Nothing was defined to test that the model makes progress";
  }
  #endif
  #endif
  #endif

  /* Print the results. */
  std::cout << nodes.size()-1 << " states, " << nr_edges << " edges\n";
  if( err_msg ){ report_error( 0, "Error" ); }

  /* Print the most important analysis settings. */
  #ifdef no_show_cnt
  std::cout << "Settings:";
  #ifdef size_par
  std::cout << " size=" << size_par;
  #endif
  #ifdef chk_state
  std::cout << " state";
  #endif
  #ifdef chk_deadlock
  std::cout << " dl";
  #endif
  #ifdef chk_may_progress
  std::cout << " may";
  #ifdef dl_not_may
  std::cout << "!dl";
  #endif
  #endif
  #ifdef chk_must_progress
  std::cout << " must";
  #ifdef dl_not_must
  std::cout << "!dl";
  #endif
  #endif
  #ifdef stubborn
  std::cout << " stubb";
  #endif
  #ifdef symmetry
  std::cout << " symm";
  #endif
  #ifdef try_forward
  std::cout << " forw";
  #endif
  #ifdef no_sanity_chk
  std::cout << " no_s_s";
  #endif
  std::cout << " hash_bits=" << hash_bits << '\n';
  #endif

  #endif

}
