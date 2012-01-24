//
//  parser.hpp
//  
//
//  Created by Andreas Grosam on 12/6/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _parser_hpp
#define _parser_hpp


#include "json/config.hpp"

#include <boost/iterator/iterator_traits.hpp>
#include <boost/noncopyable.hpp>
#include <boost/static_assert.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/type_traits.hpp>
#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp> 
#include <boost/variant.hpp>

#include <boost/mpl/and.hpp>
#include <boost/mpl/apply.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/mpl/back_inserter.hpp>
#include <boost/mpl/contains.hpp>
#include <boost/mpl/copy.hpp>
#include <boost/mpl/end.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/insert.hpp>
#include <boost/mpl/insert_range.hpp>
#include <boost/mpl/is_sequence.hpp>
#include <boost/mpl/not.hpp>
#include <boost/mpl/or.hpp>
#include <boost/mpl/placeholders.hpp>
#include <boost/mpl/set.hpp>
#include <boost/mpl/vector.hpp>



#include <algorithm>
#include <map>
#include <stdexcept>
#include <stack>
#include <utility>


namespace json { namespace jsonpath {

    namespace internal {
        
        //
        // MPL Helper classes
        //
        
        // mpl helper class to build the union of two sequences
        template<class S0, class S1>
        struct union_as_vector2 
        {
            typedef typename boost::mpl::copy<
                typename boost::mpl::copy<S0, boost::mpl::back_inserter<boost::mpl::vector<> > >::type
              , boost::mpl::back_inserter<typename boost::mpl::copy<S1, boost::mpl::back_inserter<boost::mpl::vector<> > >::type> 
            >::type type;
        };
        
        // mpl helper class to build the union of three sequences
        template<class S0, class S1, class S2>
        struct union_as_vector3 
        {
        private:
            typedef typename boost::mpl::copy<
            typename boost::mpl::copy<S0, boost::mpl::back_inserter<boost::mpl::vector<> > >::type
            , boost::mpl::back_inserter<typename boost::mpl::copy<S1, boost::mpl::back_inserter<boost::mpl::vector<> > >::type> 
            >::type s0_s1_type;
        public:
            typedef typename boost::mpl::copy<
            s0_s1_type
            , boost::mpl::back_inserter<typename boost::mpl::copy<S2, boost::mpl::back_inserter<boost::mpl::vector<> > >::type> 
            >::type type;
        };
        
    }  // namespace internal
        
        
    
    namespace internal {
    
        
        //
        //  FSA Operator tags
        //
        struct operator_tag {};
        struct symbol_tag {};
        struct paren_tag {};
        
        struct open_paren : paren_tag {};
        struct close_paren : paren_tag {};


        //
        // Special symbols:
        //
        
        // Epsilon
        struct epsilon : symbol_tag {
            
            friend inline
            // A symbol shall have defined the insertion operator and the
            // equality operator:
            std::ostream&
            operator<<(std::ostream& os, const epsilon&) {
                os << "\xCE\xB5"; // ε
                return os;
            }
            friend inline
            bool operator==(const epsilon&, const epsilon&) { return true; }
            friend inline
            bool operator<(const epsilon&, const epsilon&) { return false; }
        };
        
        
        // Default Symbol
        // A default symbol can be used to create "default transitions".
        struct default_symbol : symbol_tag {
            
            friend inline
            // A symbol shall have defined the insertion operator and the
            // equality operator:
            std::ostream&
            operator<<(std::ostream& os, const default_symbol&) {
                os << "\xCE\xB5"; // ε
                return os;
            }
            friend inline
            bool operator==(const default_symbol&, const default_symbol&) { return true; }
            friend inline
            bool operator<(const default_symbol&, const default_symbol&) { return false; }
        };

        
        //
        // Special States:
        //
        
        // A special state denoting the error state
        struct error_state {
            friend inline bool operator==(const error_state&, const error_state&) { return true; }
        };
        
        
        
        
        //
        //  Operator Precedence
        //        
        template <int Precedence>
        struct operator_base : operator_tag {
            static int precedence() { return Precedence; }
        };
        
        template <typename T, typename U>    
        inline typename boost::enable_if<
            boost::mpl::and_<
                boost::is_base_and_derived<operator_tag, T>
               ,boost::is_base_and_derived<operator_tag, U>
            >
          , bool
        >::type
        operator< (T const& lhv, U const& rhv) {
            return (lhv.precedence < rhv.precedence);
        }
        
    }  // namespace internal
        
    
    
    namespace internal {
        
        //
        //  Internal Visitors 
        // 
        
        // Process a close-paren.
        // In case operator is not a close-paren:
        //  insert it into the output iterator, pop the stack. Return true.
        // Otherwise (if operator is a close-paren) just pop the stack. Return false.
        template <class ContextT>
        struct sub : boost::static_visitor<bool> {
            sub(ContextT& context) : context_(context) {}
            
            bool operator()(open_paren) {
                context_.stack().pop();
                return false;
            }
            
            template <typename T>
            bool operator()(T token) {
                context_.postfix_append(token);
                context_.stack().pop();
                return true;
            }
            
        private:
            ContextT& context_;
        };
        
        
        // Used to compare an operator's precedence with that of the operator on
        // top of stack.
        template <typename OperatorSeq>
        struct precedence : boost::static_visitor<int> 
        {
            template <typename Op>
            typename boost::enable_if<
                typename boost::mpl::contains<OperatorSeq, Op>::type
              , int
            >::type
            operator()(Op op) const {
                return op.precedence();
            }
            
            int operator()(open_paren) const {
                return 0;
            }
        };
        
        template <class ContextT, typename SymbolSeq, typename OperatorSeq>
        struct make_postfix : boost::static_visitor<> 
        {
            make_postfix(ContextT& context) : context_(context) {};
                  
            // Operator: 
            // While the precendence of the operator on top of the stack is higher or
            // equal the current operator insert the operator on top of the stack
            // into the output iterator and pop the stack.
            // Then, push the current operator onto the stack.
            template <typename OperatorT>
            inline typename boost::enable_if<
                typename boost::mpl::contains<OperatorSeq, OperatorT>::type
              , void
            >::type
            operator() (OperatorT op) { 
                precedence<OperatorSeq> prec;
                while ( (not context_.stack().empty()) and (op.precedence() <  boost::apply_visitor(prec, context_.stack().top()) ) ) 
                {
                    context_.postfix_append(context_.stack().top());
                    context_.stack().pop();
                } 
                context_.stack().push(op);
            }

            // Open Paren: push it onto the stack:
            void operator() (open_paren paren) {
                context_.stack().push(paren);
            }
            
            // Close Paren:
            // While the operator on top of the stack is not an Open Paren, insert
            // the operator in the output iterator and pop the stack.
            // Pop the stack in order to remove the Close Paren from the stack.
            void operator() (close_paren paren) {
                sub<ContextT> s(context_);
                while (not context_.stack().empty() and boost::apply_visitor(s, context_.stack().top()) )
                {} 
            }
            
            // Symbol (Operand): push it onto the stack:
            template <typename SymbolT>
            inline typename boost::enable_if<
                typename boost::mpl::contains<SymbolSeq, SymbolT>::type
              , void
            >::type
            operator()(SymbolT const& sym) {
                context_.postfix_append(sym);
            }   
            
            
        private:
            ContextT& context_;
        };
        
        
        template <typename TokenSeq, typename OperatorSeq, typename OutIterator>
        struct context 
        {
            // Note: make_variant_over does not work with mpl::set, thus we need
            // to use mpl:vector for example.
            
            typedef typename union_as_vector2<OperatorSeq, boost::mpl::vector<open_paren> >::type  stack_element_types;
            typedef typename boost::make_variant_over<TokenSeq>::type       token_type;        
            typedef typename boost::make_variant_over<stack_element_types>::type   stack_element_type;
            typedef std::stack<stack_element_type> operator_stack;
            
            context(operator_stack& stack, OutIterator& dest) : stack_(stack), dest_(dest) {}
            
            operator_stack& stack() { return stack_; }
            void postfix_append(token_type const& token) {
                *dest_++ = token;
            }
            
        private:
            operator_stack& stack_;
            OutIterator& dest_;
        };
        
        
    }  // namespace internal
    

#pragma mark - FSA Traits
    
    template <typename Alphabet>
    class fsa_traits
    {        
        BOOST_MPL_ASSERT(( boost::mpl::is_sequence<typename Alphabet::symbol_types> ));
        typedef typename Alphabet::symbol_types alphabet_symbols;
    public:
        
        // An alphabet consists of one or more symbol types (usually one).
        // Each symbol defines the type value_type.
        
        typedef Alphabet                    alphabet;   // note, Alphabet::symbol_types may not contain epsilon
        typedef internal::epsilon           epsilon;
        typedef internal::default_symbol    default_symbol;
        
        // add epsilon to the set if required:
        typedef typename boost::mpl::if_<
              typename boost::mpl::contains<alphabet_symbols, epsilon>::type
            , alphabet_symbols
            , typename boost::mpl::insert<
                  alphabet_symbols
                , typename boost::mpl::end<alphabet_symbols>::type
                , epsilon
            >::type
        >::type                             symbol_types;
                
        // Valid label types used by transitions:
        typedef typename boost::mpl::insert<
              symbol_types
            , typename boost::mpl::end<symbol_types>::type
            , default_symbol
        >  ::type                           label_types;  // type sequence for valid label types
        
        
        
        // regular expression operations:
        struct kleenstar : internal::operator_base<3> {};
        struct concat : internal::operator_base<2> {};
        struct alternation : internal::operator_base<1> {};
        
        typedef internal::open_paren        open_paren;
        typedef internal::close_paren       close_paren;
        
        // operator set
        typedef boost::mpl::vector<
            kleenstar
          , concat
          , alternation
        >                                   operator_types;
        
        typedef boost::mpl::vector<
            open_paren
          , close_paren
        >                                   paren_types;
        
        // The sequence of token types is the union of the symbol types plus 
        // parenthesis and the operator types:
        typedef typename internal::union_as_vector3<
            operator_types, 
            paren_types,
            symbol_types
        >::type                             token_types;
        
        
        // boost variant types
        typedef typename boost::make_variant_over<operator_types>::type  operator_type;
        typedef typename boost::make_variant_over<token_types>::type     token_type;
        typedef typename boost::make_variant_over<symbol_types>::type    symbol_type;
        typedef typename boost::make_variant_over<label_types>::type     label_type;
        
        
        // Convert a sequence of an infix-expression to a postfix-expression
        template <typename InIterator, typename OutIterator> 
        static inline OutIterator to_postfix(InIterator first, InIterator last, OutIterator dest)
        {
            typedef internal::context<token_types, operator_types, OutIterator> context_type;            
            typename context_type::operator_stack stack;

            context_type ctx = context_type(stack, dest);            
            internal::make_postfix<context_type, symbol_types, operator_types> e(ctx);
            std::for_each(first, last, boost::apply_visitor(e));
            // After all tokens of the input infix-expression have been scanned
            // copy the top of stack to the output iterator and and pop the stack
            // until the stack is empty.
            while (not stack.empty()) {
                *dest++ = stack.top();
                stack.pop();
            }            
            return dest;
        }   
        
        
        // Special States:
        typedef internal::error_state       error_state;
    };
    
    
    
    
}}


namespace json { namespace jsonpath { namespace jsonpath_internal {
    
    struct symbol_tag {};
    struct index_tag : symbol_tag {};
    struct key_tag : symbol_tag {};
    
    // TODO:
    // define ctors, operator<< and member function match()
    
    struct array_index : index_tag {};
    struct integral_range : index_tag {};
    struct integral_range_set : index_tag {};
    struct string_literal : key_tag {};
    struct regex : key_tag {};
    
    struct Alphabet {
        typedef boost::mpl::vector<array_index, integral_range, integral_range_set, string_literal, regex>::type symbol_types;
    };
    
    
}}}




namespace json { namespace jsonpath { namespace fsa {
    
    
    // A state is always unique in the context of a FSA. It cannot be copied. 
    // A state can be identified by its id.
    
    
    namespace internal {
        
        //
        //  Internal Visitors
        //
        
        
        // is_less_than_vis Visitor
        // 
        // Returns a < b  for b and a of type T, otherwise
        // returns index_in_variant<A> < index_in_variant<B>  
        // 
        // Requires that for each type in the variant the less_than 
        // operator is defined.
        template <typename Variant>
        struct is_less_than_vis : boost::static_visitor<bool> 
        {
            typedef typename Variant::types types;

            // If we have equal types apply the less_than operator
            template <typename T>
            bool operator()(const T& lhv, const T& rhv) {
                return lhv < rhv;
            }
            
            // If we have different types, we evaluate the distance of both
            // argument types within the type sequence and compare those instead:
            template <typename T, typename U>
            bool operator()(const T&, const U&) {
                typedef typename boost::mpl::begin<types>::type first;
                typedef typename boost::mpl::find<types, T>::type T_iter;
                typedef typename boost::mpl::find<types, U>::type U_iter;
                typedef typename boost::mpl::distance<first, T_iter>::type   dT_type;
                typedef typename boost::mpl::distance<first, U_iter>::type   dU_type;
                
                return dT_type::value < dU_type::value;
            }
        };
        
        
        // is_equal_vis Visitor
        // 
        // Returns a == b  if b and a of type T, 
        // otherwise returns false.
        template <typename Variant> 
        struct is_equal_vis : boost::static_visitor<bool> {
            
            template <typename T> 
            bool operator()(const T& lhv, const T& rhv) {
                return lhv == rhv;
            }
            
            template <typename T, typename U>
            bool operator()(const T&, const U&) {
                return false;
            }
            
        };
        
        // less_than Comparator for usage for example in a std::map, whose 
        // argument is a boost::variant
        template <typename Variant>
        struct less_comp
        {
            bool operator()(const Variant& lhv, const Variant& rhv) {
                is_less_than_vis<Variant> vis;
                return boost::apply_visitor(vis, lhv, rhv);
            }
            
        };
        
        // less_than Comparator for usage whose
        // argument is a std::pair<first_variant, second_variant>
        template <typename Pair>
        struct less_pair_comp
        {
            typedef typename Pair::first_type   first_variant;
            typedef typename Pair::second_type  second_variant;
            
            bool operator()(const Pair& lhv, const Pair& rhv) {
                if (boost::apply_visitor(is_less_than_vis<first_variant>(), lhv.first, rhv.first)) {
                    return true;
                }
                else if (boost::apply_visitor(is_equal_vis<first_variant>(), lhv.first, rhv.first))
                {
                    return boost::apply_visitor(is_less_than_vis<second_variant>(), lhv.second, rhv.second);
                }
                else {
                    return false;
                }
            }
            
        };
        

        
        
    }  // namespace internal       
        
    
    namespace internal {
#pragma mark - Transition
        
        
        template <typename Traits, typename StateVariant>
        class transition 
        {
            BOOST_MPL_ASSERT( (boost::mpl::is_sequence<typename StateVariant::types>) );
            BOOST_MPL_ASSERT( (boost::mpl::is_sequence<typename Traits::label_type::types>) );
            
        public:
            // If the state_type of a certain transition equals type self, the 
            // transition targets its origin state.
            // If the state_type value equals NULL, the target state is undefined (invalid).
            // Otherwise it points to another valid state.
            
            
            typedef typename Traits::label_type     label_type; // a boost::variant whose types is a union of alphabet::symbol_types, epsilon and default_symbol
            typedef typename Traits::epsilon        epsilon;
            typedef StateVariant                    state_type; // a boost::variant whose types are <self_state, error_state, state_pointer>
            
            
            transition() 
            : label_(epsilon()), state_(state_type())
            {}
            

            transition(const label_type& label, const state_type& state) 
            : label_(label), state_(state)
            {}
            
            
            label_type      label_;
            state_type      state_;
            
            
            friend
            inline bool operator==(const transition& lhv, const transition& rhv) {
                return boost::apply_visitor(is_equal_vis<label_type>(), lhv.label_, rhv.label_)
                    and boost::apply_visitor(is_equal_vis<state_type>(), lhv.state_, rhv.state_); 
            }
            
            
            friend
            inline bool operator<(const transition& lhv, const transition& rhv) 
            {
                if (boost::apply_visitor(is_less_than_vis<label_type>(), lhv.label_, rhv.label_)) {
                    return true;
                }
                else if (boost::apply_visitor(is_equal_vis<label_type>(), lhv.label_, rhv.label_))
                {
                    return boost::apply_visitor(is_less_than_vis<state_type>(), lhv.state_, rhv.state_);
                }
                else {
                    return false;
                }
            }
            
        };
        
        
        
        
    }  // namespace internal
        
    
#pragma mark - State    
    
    namespace internal {
    
        // state of a non-deterministic FSA-e (NFA)
        template <typename Traits, typename ImpTraits>
        class nfa_e_state : boost::noncopyable {
        public:
            typedef typename Traits::error_state        error_state;
            
            typedef typename Traits::alphabet           alphabet;    // the alphabet for the language
            typedef typename Traits::label_type         label_type; // a boost::variant whose types is a union of alphabet::symbol_types, epsilon and default_symbol
            typedef typename Traits::token_type         token_type;  // a boost::variant whose types are allowed as types for tokens in a regular expression
            
            typedef typename ImpTraits::state_id_type   id_type;
            typedef boost::shared_ptr<nfa_e_state>      state_pointer;
            
            
            // A special state used to point to itself
            class self_state : boost::noncopyable {
            private:
                self_state();
                self_state(const nfa_e_state& state) : state_ref_(state) {}
            private:
                nfa_e_state& state_ref_;
            private:
                friend inline bool operator==(const self_state&, const self_state&) { return true; }
                
                template <class State>
                friend self_state::self_state(const nfa_e_state& state);
            };
            
        private:
            
            typedef boost::variant<
                  self_state
                , error_state
                , state_pointer>                        state_type;  // self_state must be the first type!
            
            typedef transition<Traits, state_type>      transition_type;
            
            // The transitions_set contains all transitions whose origin is this state. 
            // Since a std::set can contain only unique values, it is not possible to 
            // have dublicate transition. However, it is allowed to have more than 
            // one transitions whose labels are equal.
            // An epsilon transition to itself shall not be contained.
            typedef typename std::set<transition_type> transitions_set;                
            typedef typename transitions_set::value_type transitions_value_type;                
            typedef typename transitions_set::iterator transitions_iterator;                
            typedef typename transitions_set::const_iterator transitions_const_iterator;                
            
            
            // unary predicates applied on transition's state
            struct equal_state_pointer_pred : boost::static_visitor<bool> 
            {
                equal_state_pointer_pred(const state_type& st) : st_(st) {}
                
                bool operator()(const transition_type& v) const { 
                    return boost::apply_visitor(internal::is_equal_vis<void>(), st_, v.state_);
                }
                
                const state_type& st_;
            };
            
            // unary predicate applied on transition's label:
            struct equal_label_pred : boost::static_visitor<bool> 
            {
                equal_label_pred(const label_type& label) : label_(label) {}
                
                bool operator()(const transition_type& tran) const { 
                    return boost::apply_visitor(internal::is_equal_vis<void>(), label_, tran.label_);
                }
                
                const label_type& label_;
            };
            
            
            struct is_accepting_pred { 
                bool operator()(const transition_type& v) const { return v.second ? *(v.second).is_accepting() : false; }
            };
            
            struct point_to_other_pred { 
                // returns true if v.state_ points to another state, that is it is
                // neither self() nor NULL.
                bool operator()(const transition_type& v) const {                 
                    return not (boost::apply_visitor(internal::is_equal_vis<void>(), self_state(), v.state_) 
                                or boost::apply_visitor(internal::is_equal_vis<void>(), NULL, v.state_));
                }
            };
            
            
        public:
            
            // Constructor
            // Creates a state with no transitions.
            explicit nfa_e_state(bool isAccepting, id_type id = id_type()) 
            : is_accepting_(isAccepting), state_id_(id) 
            {}
            
            
            // Constructor
            // Creates a state with one transition 'transition'.
            nfa_e_state(const transition_type& transition, bool isAccepting, id_type id = id_type()) 
            : is_accepting_(isAccepting), state_id_(id) 
            {
                transitions_.insert(transition);
            }
            
            
            self_state self() const { return self_state(*this); }
            
#if 0            
            // Construct a state from a set-sequence of other pointers to state:
            template <typename Iterator>
            nfa_e_state(Iterator first, Iterator last, id_type id) 
            : states_(first, last), state_id_(id)
            {
                //BOOST_STATIC_ASSERT( boost::is_same<state*, boost::iterator_traits<Iterator>::value_type>::value == true );
                assert(std::find(first, last, this) == last);
                assert(std::find(first, last, NULL) == last);
                
                // If any of the states in the sequence is an accepting state,
                // this state becomes an accepting state, too:
                if (std::find_if(first, last, is_accepting_pred()) != last) {
                    flags_ |= AcceptingState;
                }
            }
#endif            
            
            id_type id() const              { return state_id_; }
            
            bool is_accepting() const       { return is_accepting_; }        
            void set_accepting(bool set)    { is_accepting_ = set; }
            
            bool is_dead_end() const { 
                if (is_accepting())
                    return false;
                // We don't have a dead end, if any of the transitions points to another state:
                transitions_const_iterator iter = std::find_if(transitions_.begin(), transitions_.end(), point_to_other_pred());
                return iter == transitions_.end();  // If true, non of the transitions points to another state, otherwise
                // at least one transition points to another state.
            }
            
            // Adds a transition with a defined label and with a defined target state.
            // Returns true if the transition has been inserted successfully.
            // Otherwise it returns false, e.g. if there is already such a transition 
            // with the same label and the same target state within in the set.
            bool add_transition(const transition_type& transition) {
                bool result = transitions_.insert(transition).second;
                return result;
            } 
            
            void remove_transitions_with_state(nfa_e_state const& st) {
                state_pointer sp = &st == this ? self_state() : &st;
                equal_state_pointer_pred  pred = equal_state_pointer_pred(sp);
                
                transitions_iterator first = transitions_.begin();
                transitions_iterator last = transitions_.end();            
                while (first != last) {
                    if (pred(*first)) {
                        transitions_iterator tmp = first;
                        ++first;
                        transitions_.erase(tmp);                    
                    }
                    else {
                        ++first;
                    }
                }
            }
            
            
            // Copies the states whose label equals 'label' into output iterator dest
            template <typename OutIterator>
            OutIterator copy_transitions_with_label(const label_type& label, OutIterator dest) const {
                typedef transitions_const_iterator iterator;
                equal_label_pred pred(label);
                iterator first = transitions_.begin();
                iterator last = transitions_.end();
                while (first != last) {
                    if (pred(*first)) {
                        *dest++ = *first;
                    }
                }
                return dest;
            }
            
            
            // states are equal, if they have the same transitions
            // and if they both have the same accepting flag.
            
            bool is_equal(nfa_e_state const& other) const {
                return (is_accepting_ == other.is_accepting_ and transitions_ == other.transitions_);
            }
            
            
            inline friend
            bool operator==(const nfa_e_state& lhv, const nfa_e_state& rhv) {
                return lhv.is_equal(rhv);
            }
            
        private:
            
            // default c-tor
            nfa_e_state(); // n.a.
            
            
            
        private:
            id_type             state_id_;
            transitions_set     transitions_;
            bool                is_accepting_;
        };  // class nfa_e_state
    
    
    
    }    // namespace inernal
    
    
                                            
#pragma mark - NFSA                                            
    
    
    namespace internal {
        
        struct fsa_tag {};
    
    
    
    
#pragma mark - Primitive FSA
    
        //
        //  Primitive FSA
        //
        
        template <typename Traits, typename ImpTraits>
        class FSA_single : boost::noncopyable, public fsa_tag
        {
        public:
            
            typedef typename Traits::label_type             label_type;     // a boost::variant whose types is a union 
            // of alphabet::symbol_types, epsilon and default_symbol.        
            
            typedef typename ImpTraits::state_id_type       state_id_type;
            
            typedef nfa_e_state<Traits, ImpTraits>          state;     
            typedef typename Traits::state_type             state_type;    // variant<self, state_pointer>
            typedef typename Traits::state_pointer          state_pointer; // share_ptr<state_type>
            typedef typename Traits::transition_type        transition_type;
            
            
            
            FSA_single(const label_type& label) 
            {
                end_state_ =  state_pointer(new state(1, true));
                start_state_ = state_pointer(new state(transition_type(label, end_state_)));
            }
            
            state_pointer   start_state() const {
                return start_state_;
            }
            
            template <typename OutIterator>
            OutIterator states(OutIterator dest) const  {
                *dest++ = start_state_;
                *dest++ = end_state_;
                return dest;
            }
            
            size_t size() const { return 2; }
            
            
        private:
            state_pointer start_state_;
            state_pointer end_state_;
        };
        
        
        template <typename Traits, typename ImpTraits>
        class FSA_alt : boost::noncopyable, public fsa_tag
        {
        public:
            
            typedef typename Traits::label_type             label_type;     // a boost::variant whose types is a union 
            // of alphabet::symbol_types, epsilon and default_symbol.        
            
            typedef typename ImpTraits::state_id_type       state_id_type;
            
            typedef nfa_e_state<Traits, ImpTraits>          state;     
            typedef typename Traits::state_type             state_type;    // variant<self, state_pointer>
            typedef typename Traits::state_pointer          state_pointer; // share_ptr<state_type>
            typedef typename Traits::transition_type        transition_type;
            
            
            
            FSA_alt(const label_type& label_a, const label_type& label_b) 
            {
                // create the end state (an accepeting state):
                end_state_ = state_pointer(new state(1, true));
                
                // create accepting end state:
                start_state_ = state_pointer(new state(true));
                
                // add two transitions:
                start_state_->add_transition(transition_type(label_a, end_state_));
                start_state_->add_transition(transition_type(label_b, end_state_));
            }
            
            state_pointer   start_state() const {
                return start_state_;
            }
            
            template <typename OutIterator>
            OutIterator states(OutIterator dest) const  {
                *dest++ = start_state_;
                *dest++ = end_state_;
                return dest;
            }
            
            size_t size() const { return 2; }
            
            
        private:
            state_pointer start_state_;
            state_pointer end_state_;
        };
        
        
    } // namesapce internal
    
    
#if 0    
    // "Alphabet" is the type of a set of "characters" defining the alphabet 
    // of a language.
    // "RegularSet" is the type of a regular string whose elements are elements 
    // of Alphabet.
    template <typename Traits, typename ImpTraits>
    class NFSA : boost::noncopyable, public fsa_tag
    {        
    public:
        
        typedef Traits                                  traits;
        
        typename typedef ImpTraits::state_id_type       state_id_type;
        
        typedef nfa_e_state<Traits, ImpTraits>          state;     
        typedef typename Traits::state_type             state_type;    // variant<self, state_pointer>
        typedef typename Traits::state_pointer          state_pointer; // share_ptr<state_type>
        


        typedef typename Traits::alphabet               alphabet;    // the alphabet for the language
        typedef typename Traits::symbol_types           symbol_types;  // type sequence for valid symbol types
        typedef typename Traits::epsilon                epsilon;
        typedef internal::default_symbol                default_symbol;
        
        typedef typename Traits::label_type             label_type;  // a boost::variant whose types is a union 
                                                                     // of alphabet::symbol_types, epsilon and default_symbol.        
        typedef typename Traits::symbol_type            symbol_type;
        
        typedef typename Traits::transition_type        transition_type;
        
        
        typedef std::set<state_type>                    state_set;
        typedef typename Traits::token_type             token_type;  // a variant whose types are allowed as types for tokens in a regular expression
        
    public:
        
        static bool with_epsilon_moves() const {
            const bool result = boost::mpl::contains<typename Traits::label_types, epsilon>::value;
        }
        
        
        
        NFSA() 
        {
            // TODO
        };
        
        // Creates a NFSA with one start state I and one accepting state E connected 
        // by a transition with label 'label'. That is, it represents the regular
        // expression:  "a", whose Alphabet Σ = {a}:
        NFSA(const label_type& label) 
        {
            // create the end state (an accepeting state):
            state_pointer endstate(new state(1, true));
            
            // create the start state with transition t('label', endstate):
            start_state_ = state_pointer(new state(transition_type(label, endstate)));
        }

        
        // Creates a NFSA with one start state I and one accepting state E connected 
        // by two transitions with label 'label_a' and 'label_b' respectively. That is, 
        // it represents the regular expression:  "a|b", whose Alphabet Σ = {a, b}:
        NFSA(const label_type& label_a, const label_type& label_b) 
        {
            // create the end state (an accepeting state):
            state_pointer endstate(new state(1, true));
            
            // create accepting end state:
            start_state_ = state_pointer(new state(true));
            
            // add two transitions:
            start_state_->add_transition(transition(label_a, endsate));
            start_state_->add_transition(transition(label_b, endsate));
        }
        
        
        
        
        state_pointer   start_state() const {
            return start_state_;
        }
        
        
        
        NFSA concat(const NFSA& other) const {
            NFSA result;
            
            return result;
        }
        
        NFSA alternation(const NFSA& other) const {
            NFSA result;
            
            return result;
        }
        
        NFSA kleenstar(const NFSA& other) const {
            NFSA result;
            
            return result;
        }
        
        
    private:
        state_pointer start_state_;
        
        
        
    private:
        
        void push(char_class_type c)
        {
            // Create 2 new states on the heap
            state* s0 = new state(++state_id_);
            state* s1 = new state(++state_id_);
            
            // Add the transition from s0->s1 on input character
            s0->add_transition(c, s1);
            
            // Create a NFA from these 2 states 
            FSA_TABLE NFATable;
            NFATable.push(s0);
            NFATable.push(s1);
            
            // push it onto the operand stack
            m_OperandStack.push(NFATable);
            
            // Add this character to the input character set
            
            m_InputSet.insert(chInput);
        }
    };
#endif    
    

    
    
    template <typename FSA>
    class acceptor 
    {
        typedef typename FSA::traits                traits;
        typedef typename traits::state_type         state_type;     // variant<self_state, error_state, state_pointer>
        typedef typename traits::state_pointer      state_pointer;  // shared_ptr<state_type>
        typedef typename traits::symbol_type        symbol_type;    // variant of valid symbol types
        typedef typename traits::error_state        error_state;
        
    private:
        
        struct is_error_state : boost::static_visitor<bool> {
            
            bool operator()(error_state) const { return true; }
            
            template <typename T>
            bool operator()(T) const { return false; }
            
        };
        
        struct is_accepting_state : boost::static_visitor<bool> {
            
            bool operator()(error_state) const { return false; }
            bool operator()(self_state) const { return false; }
            
            template <typename T>
            bool operator()(T) const { return false; }
            
        };
        
        
    public:        
        acceptor(const FSA& fsa) : fsa_(fsa), start_state_(fsa.start_state()) {}
        
        
        // Check if the input language (a sequence of symbols) can be accepted
        template <typename InIterator>
        bool accept(InIterator first, InIterator last) 
        {
            BOOST_STATIC_ASSERT( (boost::is_same<symbol_type, typename boost::iterator_value<InIterator>::value_type>::value) );
            
            bool error = false;
            state_type state = NULL;
            while (!error and first != last) {
                state = fsa_.transform(*first);
                error = boost::apply_visitor(is_error_state(), state);
            }
            
            return !error and first == last and boost::apply_visitor(is_accepting_state(), state);
        }
        
    private:
        
        FSA&          fsa_;
        state_pointer start_state_;
        
    private:
        acceptor(); // no defaut c-tor
    };
    
    
    
    
    
    // Creates a NFSA with one start state I and one accepting state E connected 
    // by a transition with label 'label'. That is, it represents the regular
    // expression:  "a", whose Alphabet Σ = {a}:
    
    
}}}   // namespace json::jsonpath::fsa
#endif



        

    
    

#if 0
// NFSA Generator
namespace json { namespace jsonpath { namespace jsonpath_internal {

    // Non-Deterministic Finite State Automaton Generator
    template <typename FSATraits, typename ImpTraits>
    class nfsa_generator 
    {
    public:
        // types        
        typedef class NFSA<FSATraits, ImpTraits>    automaton;
        
        typedef typename FSATraits::alphabet        alphabet;    // the alphabet for the language
        typedef typename FSATraits::symbol_type     symbol_type; // a variant< {FSATraits::alphabet::symbol_types + epsilon} >
        typedef typename FSATraits::token_type      token_type;  // a variant whose types are allowed as types for tokens in a regular expression
        
        typedef typename FSATraits::concat          concat;
        typedef typename FSATraits::alternation     alternation;
        typedef typename FSATraits::kleenstar       kleenstar;
        
        typedef typename FSATraits::open_paren      open_paren;
        typedef typename FSATraits::close_paren     close_paren;
        
        
    public:
        nfsa_generator() {}
        
        
        // Create a NFSA
        // The sequence starting with first shall be a regular expression in
        // postfix notation. This also means, Iterator::value_type is the same 
        // as token_type 
        template <typename Iterator>
        automaton generate(Iterator first, Iterator last) 
        {
            BOOST_STATIC_ASSERT( (boost::is_same<token_type, boost::iterator_value<Iterator>::value_type>::value) );
            
            stack_.clear();
            make_fsa e(stack_);
            while (first != last) {
                boost::apply_visitor(e, *first);
                ++first;
            }
            assert(stack_.size() == 1);
            automaton result = stack_.back();
            stack.pop();
            return result;
        }
        
    private:
        typedef std::stack<automaton> stack_type;

    private:
        
        // Process tokens
        struct make_fsa : boost::static_visitor<void> {
            make_fsa(stack_type& stack) : stack_(stack) {}
            
            void operator() (symbol_type const& sym) {
                stack_.push(automaton(sym));
            }
            
            void operator() (concat) {
                automaton b = stack_.top();
                stack_.pop();
                symbol_type a = stack_.top();
                stack_.pop();
                automaton A(a);
                automaton B(b);
                automaton R = concat(A, B);
                stack_.push(R);
            }
            
            void operator() (alternation) {
                automaton b = stack_.top();
                stack_.pop();
                automaton a = stack_.top();
                stack_.pop();
                automaton A(a);
                automaton B(b);
                automaton R = alternation(A, B);
                stack_.push(R);
            }
            
            void operator() (kleenstar) {
                automaton a = stack_.top();
                stack_.pop();
                automaton A(a);
                automaton R = kleenstar(A);
                stack_.push(R);                
            }
            
            void operator() (open_paren) {
                // TODO:
            }

            void operator() (close_paren) {
                // TODO:
            }

            
        private:
            stack_type& stack_;
        };
        
        
        
    private:
        stack_type  stack_;
    };
    
}}}
#endif   



#endif   // _parser_hpp
