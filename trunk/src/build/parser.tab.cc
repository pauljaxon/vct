
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton implementation for Bison LALR(1) parsers in C++
   
      Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007, 2008 Free Software
   Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */


/* First part of user declarations.  */


/* Line 311 of lalr1.cc  */
#line 41 "parser.tab.cc"


#include "parser.tab.hh"

/* User implementation prologue.  */


/* Line 317 of lalr1.cc  */
#line 50 "parser.tab.cc"
/* Unqualified %code blocks.  */

/* Line 318 of lalr1.cc  */
#line 80 "parser.yy"
 // **bison 2.4**

/* First user implementation prologue.  Inserted at start of parser.tab.cc */

#include "lexer.hh"




/* Line 318 of lalr1.cc  */
#line 65 "parser.tab.cc"

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* FIXME: INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#define YYUSE(e) ((void) (e))

/* Enable debugging if requested.  */
#if YYDEBUG

/* A pseudo ostream that takes yydebug_ into account.  */
# define YYCDEBUG if (yydebug_) (*yycdebug_)

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)	\
do {							\
  if (yydebug_)						\
    {							\
      *yycdebug_ << Title << ' ';			\
      yy_symbol_print_ ((Type), (Value), (Location));	\
      *yycdebug_ << std::endl;				\
    }							\
} while (false)

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug_)				\
    yy_reduce_print_ (Rule);		\
} while (false)

# define YY_STACK_PRINT()		\
do {					\
  if (yydebug_)				\
    yystack_print_ ();			\
} while (false)

#else /* !YYDEBUG */

# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_REDUCE_PRINT(Rule)
# define YY_STACK_PRINT()

#endif /* !YYDEBUG */

#define yyerrok		(yyerrstatus_ = 0)
#define yyclearin	(yychar = yyempty_)

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)


/* Line 380 of lalr1.cc  */
#line 1 "[Bison:b4_percent_define_default]"

namespace yy {

/* Line 380 of lalr1.cc  */
#line 134 "parser.tab.cc"
#if YYERROR_VERBOSE

  /* Return YYSTR after stripping away unnecessary quotes and
     backslashes, so that it's suitable for yyerror.  The heuristic is
     that double-quoting is unnecessary unless the string contains an
     apostrophe, a comma, or backslash (other than backslash-backslash).
     YYSTR is taken from yytname.  */
  std::string
  parser::yytnamerr_ (const char *yystr)
  {
    if (*yystr == '"')
      {
        std::string yyr = "";
        char const *yyp = yystr;

        for (;;)
          switch (*++yyp)
            {
            case '\'':
            case ',':
              goto do_not_strip_quotes;

            case '\\':
              if (*++yyp != '\\')
                goto do_not_strip_quotes;
              /* Fall through.  */
            default:
              yyr += *yyp;
              break;

            case '"':
              return yyr;
            }
      do_not_strip_quotes: ;
      }

    return yystr;
  }

#endif

  /// Build a parser object.
  parser::parser (pdriver& driver_yyarg)
    :
#if YYDEBUG
      yydebug_ (false),
      yycdebug_ (&std::cerr),
#endif
      driver (driver_yyarg)
  {
  }

  parser::~parser ()
  {
  }

#if YYDEBUG
  /*--------------------------------.
  | Print this symbol on YYOUTPUT.  |
  `--------------------------------*/

  inline void
  parser::yy_symbol_value_print_ (int yytype,
			   const semantic_type* yyvaluep, const location_type* yylocationp)
  {
    YYUSE (yylocationp);
    YYUSE (yyvaluep);
    switch (yytype)
      {
         default:
	  break;
      }
  }


  void
  parser::yy_symbol_print_ (int yytype,
			   const semantic_type* yyvaluep, const location_type* yylocationp)
  {
    *yycdebug_ << (yytype < yyntokens_ ? "token" : "nterm")
	       << ' ' << yytname_[yytype] << " ("
	       << *yylocationp << ": ";
    yy_symbol_value_print_ (yytype, yyvaluep, yylocationp);
    *yycdebug_ << ')';
  }
#endif

  void
  parser::yydestruct_ (const char* yymsg,
			   int yytype, semantic_type* yyvaluep, location_type* yylocationp)
  {
    YYUSE (yylocationp);
    YYUSE (yymsg);
    YYUSE (yyvaluep);

    YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

    switch (yytype)
      {
  
	default:
	  break;
      }
  }

  void
  parser::yypop_ (unsigned int n)
  {
    yystate_stack_.pop (n);
    yysemantic_stack_.pop (n);
    yylocation_stack_.pop (n);
  }

#if YYDEBUG
  std::ostream&
  parser::debug_stream () const
  {
    return *yycdebug_;
  }

  void
  parser::set_debug_stream (std::ostream& o)
  {
    yycdebug_ = &o;
  }


  parser::debug_level_type
  parser::debug_level () const
  {
    return yydebug_;
  }

  void
  parser::set_debug_level (debug_level_type l)
  {
    yydebug_ = l;
  }
#endif

  int
  parser::parse ()
  {
    /// Lookahead and lookahead in internal form.
    int yychar = yyempty_;
    int yytoken = 0;

    /* State.  */
    int yyn;
    int yylen = 0;
    int yystate = 0;

    /* Error handling.  */
    int yynerrs_ = 0;
    int yyerrstatus_ = 0;

    /// Semantic value of the lookahead.
    semantic_type yylval;
    /// Location of the lookahead.
    location_type yylloc;
    /// The locations where the error started and ended.
    location_type yyerror_range[2];

    /// $$.
    semantic_type yyval;
    /// @$.
    location_type yyloc;

    int yyresult;

    YYCDEBUG << "Starting parse" << std::endl;


    /* Initialize the stacks.  The initial state will be pushed in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystate_stack_ = state_stack_type (0);
    yysemantic_stack_ = semantic_stack_type (0);
    yylocation_stack_ = location_stack_type (0);
    yysemantic_stack_.push (yylval);
    yylocation_stack_.push (yylloc);

    /* New state.  */
  yynewstate:
    yystate_stack_.push (yystate);
    YYCDEBUG << "Entering state " << yystate << std::endl;

    /* Accept?  */
    if (yystate == yyfinal_)
      goto yyacceptlab;

    goto yybackup;

    /* Backup.  */
  yybackup:

    /* Try to take a decision without lookahead.  */
    yyn = yypact_[yystate];
    if (yyn == yypact_ninf_)
      goto yydefault;

    /* Read a lookahead token.  */
    if (yychar == yyempty_)
      {
	YYCDEBUG << "Reading a token: ";
	yychar = yylex (&yylval, driver);
      }


    /* Convert token to internal form.  */
    if (yychar <= yyeof_)
      {
	yychar = yytoken = yyeof_;
	YYCDEBUG << "Now at end of input." << std::endl;
      }
    else
      {
	yytoken = yytranslate_ (yychar);
	YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
      }

    /* If the proper action on seeing token YYTOKEN is to reduce or to
       detect an error, take that action.  */
    yyn += yytoken;
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yytoken)
      goto yydefault;

    /* Reduce or error.  */
    yyn = yytable_[yyn];
    if (yyn <= 0)
      {
	if (yyn == 0 || yyn == yytable_ninf_)
	goto yyerrlab;
	yyn = -yyn;
	goto yyreduce;
      }

    /* Shift the lookahead token.  */
    YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

    /* Discard the token being shifted.  */
    yychar = yyempty_;

    yysemantic_stack_.push (yylval);
    yylocation_stack_.push (yylloc);

    /* Count tokens shifted since error; after three, turn off error
       status.  */
    if (yyerrstatus_)
      --yyerrstatus_;

    yystate = yyn;
    goto yynewstate;

  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact_[yystate];
    if (yyn == 0)
      goto yyerrlab;
    goto yyreduce;

  /*-----------------------------.
  | yyreduce -- Do a reduction.  |
  `-----------------------------*/
  yyreduce:
    yylen = yyr2_[yyn];
    /* If YYLEN is nonzero, implement the default value of the action:
       `$$ = $1'.  Otherwise, use the top of the stack.

       Otherwise, the following line sets YYVAL to garbage.
       This behavior is undocumented and Bison
       users should not rely upon it.  */
    if (yylen)
      yyval = yysemantic_stack_[yylen - 1];
    else
      yyval = yysemantic_stack_[0];

    {
      slice<location_type, location_stack_type> slice (yylocation_stack_, yylen);
      YYLLOC_DEFAULT (yyloc, slice, yylen);
    }
    YY_REDUCE_PRINT (yyn);
    switch (yyn)
      {
	  case 2:

/* Line 678 of lalr1.cc  */
#line 190 "parser.yy"
    {driver.result = (yysemantic_stack_[(1) - (1)].nval);}
    break;

  case 3:

/* Line 678 of lalr1.cc  */
#line 194 "parser.yy"
    { (yyval.nval) = (yysemantic_stack_[(2) - (2)].nval); }
    break;

  case 4:

/* Line 678 of lalr1.cc  */
#line 195 "parser.yy"
    { (yyval.nval) = (yysemantic_stack_[(2) - (2)].nval); }
    break;

  case 5:

/* Line 678 of lalr1.cc  */
#line 196 "parser.yy"
    { (yyval.nval) = (yysemantic_stack_[(2) - (2)].nval); }
    break;

  case 6:

/* Line 678 of lalr1.cc  */
#line 206 "parser.yy"
    {
	      // Drop recording of $2 and $3
              (yyval.nval) = (yysemantic_stack_[(7) - (5)].nval);
              (yyval.nval)->kind = z::FDL_FILE;
          }
    break;

  case 10:

/* Line 678 of lalr1.cc  */
#line 220 "parser.yy"
    { (yyval.nval) = new Node(z::SEQ); }
    break;

  case 11:

/* Line 678 of lalr1.cc  */
#line 221 "parser.yy"
    { (yyval.nval) = (yysemantic_stack_[(2) - (1)].nval);  (yyval.nval)->addChild((yysemantic_stack_[(2) - (2)].nval));}
    break;

  case 12:

/* Line 678 of lalr1.cc  */
#line 226 "parser.yy"
    { if ((yysemantic_stack_[(5) - (4)].nval)->kind == z::PENDING)
              (yyval.nval) = new Node(z::DEF_TYPE, * (yysemantic_stack_[(5) - (2)].sval));
          else
              (yyval.nval) = new Node(z::DEF_TYPE, * (yysemantic_stack_[(5) - (2)].sval), (yysemantic_stack_[(5) - (4)].nval));
          delete (yysemantic_stack_[(5) - (2)].sval);
        }
    break;

  case 13:

/* Line 678 of lalr1.cc  */
#line 233 "parser.yy"
    { if ((yysemantic_stack_[(7) - (6)].nval)->kind == z::PENDING)
              (yyval.nval) = new Node(z::DEF_CONST, * (yysemantic_stack_[(7) - (2)].sval), (yysemantic_stack_[(7) - (4)].nval));
          else
              (yyval.nval) = new Node(z::DEF_CONST, * (yysemantic_stack_[(7) - (2)].sval), (yysemantic_stack_[(7) - (4)].nval), (yysemantic_stack_[(7) - (6)].nval));
          delete (yysemantic_stack_[(7) - (2)].sval);
        }
    break;

  case 14:

/* Line 678 of lalr1.cc  */
#line 240 "parser.yy"
    { (yyval.nval) = new Node(z::DECL_VAR, * (yysemantic_stack_[(5) - (2)].sval), (yysemantic_stack_[(5) - (4)].nval));
          delete (yysemantic_stack_[(5) - (2)].sval);
        }
    break;

  case 15:

/* Line 678 of lalr1.cc  */
#line 245 "parser.yy"
    { (yyval.nval) = new Node(z::DECL_FUN,
                        * (yysemantic_stack_[(8) - (2)].sval), 
                        (yysemantic_stack_[(8) - (4)].nval),
                        (yysemantic_stack_[(8) - (7)].nval)); 
          delete (yysemantic_stack_[(8) - (2)].sval);
        }
    break;

  case 16:

/* Line 678 of lalr1.cc  */
#line 252 "parser.yy"
    { (yyval.nval) = new Node(z::DEF_CONST, * (yysemantic_stack_[(5) - (2)].sval), (yysemantic_stack_[(5) - (4)].nval));
          delete (yysemantic_stack_[(5) - (2)].sval);
        }
    break;

  case 17:

/* Line 678 of lalr1.cc  */
#line 258 "parser.yy"
    { (yyval.nval) = new Node(z::SEQ,(yysemantic_stack_[(1) - (1)].nval)); }
    break;

  case 18:

/* Line 678 of lalr1.cc  */
#line 259 "parser.yy"
    { (yyval.nval) = (yysemantic_stack_[(3) - (1)].nval);           
                      (yyval.nval)->addChild((yysemantic_stack_[(3) - (3)].nval)); 
                    }
    break;

  case 19:

/* Line 678 of lalr1.cc  */
#line 275 "parser.yy"
    { (yyval.nval) = (yysemantic_stack_[(2) - (2)].nval);
            (yyval.nval)->addLeftChild(new Node(z::RULE_FAMILY,
                                      "implicit",
                                      new Node(z::SEQ), 
                                      (yysemantic_stack_[(2) - (1)].nval)));
            (yyval.nval)->kind = z::RULE_FILE;
          }
    break;

  case 20:

/* Line 678 of lalr1.cc  */
#line 285 "parser.yy"
    { (yyval.nval) = new Node(z::SEQ); }
    break;

  case 21:

/* Line 678 of lalr1.cc  */
#line 286 "parser.yy"
    { (yyval.nval) = (yysemantic_stack_[(2) - (1)].nval); (yyval.nval)->addChild((yysemantic_stack_[(2) - (2)].nval)); }
    break;

  case 22:

/* Line 678 of lalr1.cc  */
#line 291 "parser.yy"
    { (yyval.nval) = new Node(z::RULE_FAMILY, *(yysemantic_stack_[(6) - (2)].sval), (yysemantic_stack_[(6) - (4)].nval), (yysemantic_stack_[(6) - (6)].nval)); }
    break;

  case 23:

/* Line 678 of lalr1.cc  */
#line 293 "parser.yy"
    { (yyval.nval) = new Node(z::RULE_FAMILY,
                                            *(yysemantic_stack_[(5) - (2)].sval),
                                            new Node(z::SEQ),
                                            (yysemantic_stack_[(5) - (5)].nval));
                            }
    break;

  case 24:

/* Line 678 of lalr1.cc  */
#line 301 "parser.yy"
    { (yyval.nval) = new Node(z::SEQ,(yysemantic_stack_[(1) - (1)].nval)); }
    break;

  case 25:

/* Line 678 of lalr1.cc  */
#line 302 "parser.yy"
    { (yyval.nval) = (yysemantic_stack_[(3) - (1)].nval);  (yyval.nval)->addChild((yysemantic_stack_[(3) - (3)].nval)); }
    break;

  case 26:

/* Line 678 of lalr1.cc  */
#line 306 "parser.yy"
    { (yyval.nval) = new Node(z::SEQ); }
    break;

  case 27:

/* Line 678 of lalr1.cc  */
#line 310 "parser.yy"
    { (yyval.nval) = new Node(z::SEQ);  }
    break;

  case 28:

/* Line 678 of lalr1.cc  */
#line 311 "parser.yy"
    { (yyval.nval) = (yysemantic_stack_[(2) - (1)].nval);   (yyval.nval)->addChild((yysemantic_stack_[(2) - (2)].nval)); }
    break;

  case 29:

/* Line 678 of lalr1.cc  */
#line 316 "parser.yy"
    { (yyval.nval) = (yysemantic_stack_[(7) - (6)].nval); delete (yysemantic_stack_[(7) - (1)].sval); delete (yysemantic_stack_[(7) - (3)].sval);}
    break;

  case 30:

/* Line 678 of lalr1.cc  */
#line 320 "parser.yy"
    { (yyval.nval) = new Node(z::MAY_BE_REPLACED_BY, (yysemantic_stack_[(4) - (1)].nval), (yysemantic_stack_[(4) - (3)].nval));
                        if ((yysemantic_stack_[(4) - (4)].nval)->kind != z::TRUE) (yyval.nval)->addChild((yysemantic_stack_[(4) - (4)].nval));
                      }
    break;

  case 31:

/* Line 678 of lalr1.cc  */
#line 324 "parser.yy"
    { (yyval.nval) = new Node(z::MAY_BE_DEDUCED, (yysemantic_stack_[(2) - (1)].nval)); }
    break;

  case 32:

/* Line 678 of lalr1.cc  */
#line 326 "parser.yy"
    { (yyval.nval) = new Node(z::MAY_BE_DEDUCED, (yysemantic_stack_[(3) - (1)].nval), (yysemantic_stack_[(3) - (3)].nval)); }
    break;

  case 33:

/* Line 678 of lalr1.cc  */
#line 328 "parser.yy"
    { (yyval.nval) = new Node(z::ARE_INTERCHANGEABLE, (yysemantic_stack_[(5) - (1)].nval), (yysemantic_stack_[(5) - (3)].nval));
                        if ((yysemantic_stack_[(5) - (5)].nval)->kind != z::TRUE) (yyval.nval)->addChild((yysemantic_stack_[(5) - (5)].nval));
                      }
    break;

  case 34:

/* Line 678 of lalr1.cc  */
#line 334 "parser.yy"
    { (yyval.nval) = new Node(z::TRUE); }
    break;

  case 35:

/* Line 678 of lalr1.cc  */
#line 335 "parser.yy"
    { (yyval.nval) = (yysemantic_stack_[(2) - (2)].nval); }
    break;

  case 36:

/* Line 678 of lalr1.cc  */
#line 345 "parser.yy"
    { (yyval.nval) = (yysemantic_stack_[(1) - (1)].nval); (yyval.nval)->kind = z::VCG_FILE; }
    break;

  case 37:

/* Line 678 of lalr1.cc  */
#line 351 "parser.yy"
    { (yyval.nval) = (yysemantic_stack_[(1) - (1)].nval); }
    break;

  case 38:

/* Line 678 of lalr1.cc  */
#line 353 "parser.yy"
    { (yyval.nval) = (yysemantic_stack_[(2) - (1)].nval);
                        (yyval.nval)->appendChildren((yysemantic_stack_[(2) - (2)].nval));
                      }
    break;

  case 39:

/* Line 678 of lalr1.cc  */
#line 359 "parser.yy"
    {
       (yyval.nval) = (yysemantic_stack_[(4) - (4)].nval);
       for (int i = 0; i != (yyval.nval)->arity(); i++) {
           Node* c = (yyval.nval)->child(i);
           (c->id).append(" " + (* (yysemantic_stack_[(4) - (2)].sval)));
       }
       delete (yysemantic_stack_[(4) - (2)].sval);
   }
    break;

  case 40:

/* Line 678 of lalr1.cc  */
#line 370 "parser.yy"
    {(yyval.nval) = new Node(z::SEQ,(yysemantic_stack_[(1) - (1)].nval)); }
    break;

  case 41:

/* Line 678 of lalr1.cc  */
#line 371 "parser.yy"
    {(yyval.nval) = (yysemantic_stack_[(2) - (1)].nval); (yyval.nval)->addChild((yysemantic_stack_[(2) - (2)].nval)); }
    break;

  case 42:

/* Line 678 of lalr1.cc  */
#line 376 "parser.yy"
    {
             (yysemantic_stack_[(5) - (3)].nval)->kind = z::HYPS;
             (yysemantic_stack_[(5) - (5)].nval)->kind = z::CONCLS;
             (yyval.nval) = new Node(z::GOAL, * (yysemantic_stack_[(5) - (1)].sval), (yysemantic_stack_[(5) - (3)].nval), (yysemantic_stack_[(5) - (5)].nval));
             delete (yysemantic_stack_[(5) - (1)].sval);
            }
    break;

  case 43:

/* Line 678 of lalr1.cc  */
#line 383 "parser.yy"
    {
             (yyval.nval) = new Node (z::GOAL, * (yysemantic_stack_[(5) - (1)].sval)); 
             delete (yysemantic_stack_[(5) - (1)].sval);
            }
    break;

  case 44:

/* Line 678 of lalr1.cc  */
#line 390 "parser.yy"
    {(yyval.nval) = new Node(z::SEQ); }
    break;

  case 45:

/* Line 678 of lalr1.cc  */
#line 391 "parser.yy"
    {(yyval.nval) = (yysemantic_stack_[(2) - (1)].nval); (yyval.nval)->addChild((yysemantic_stack_[(2) - (2)].nval)); }
    break;

  case 46:

/* Line 678 of lalr1.cc  */
#line 394 "parser.yy"
    {(yyval.nval) = (yysemantic_stack_[(4) - (3)].nval); delete (yysemantic_stack_[(4) - (1)].sval);}
    break;

  case 47:

/* Line 678 of lalr1.cc  */
#line 398 "parser.yy"
    {(yyval.nval) = new Node(z::SEQ); }
    break;

  case 48:

/* Line 678 of lalr1.cc  */
#line 399 "parser.yy"
    {(yyval.nval) = (yysemantic_stack_[(2) - (1)].nval); (yyval.nval)->addChild((yysemantic_stack_[(2) - (2)].nval)); }
    break;

  case 49:

/* Line 678 of lalr1.cc  */
#line 402 "parser.yy"
    {(yyval.nval) = (yysemantic_stack_[(4) - (3)].nval); delete (yysemantic_stack_[(4) - (1)].sval);}
    break;

  case 50:

/* Line 678 of lalr1.cc  */
#line 415 "parser.yy"
    { (yyval.sval) = (yysemantic_stack_[(1) - (1)].sval);}
    break;

  case 51:

/* Line 678 of lalr1.cc  */
#line 416 "parser.yy"
    { (yyval.sval) = (yysemantic_stack_[(1) - (1)].sval);}
    break;

  case 52:

/* Line 678 of lalr1.cc  */
#line 417 "parser.yy"
    { (yyval.sval) = (yysemantic_stack_[(1) - (1)].sval);}
    break;

  case 53:

/* Line 678 of lalr1.cc  */
#line 418 "parser.yy"
    { (yyval.sval) = (yysemantic_stack_[(1) - (1)].sval);}
    break;

  case 54:

/* Line 678 of lalr1.cc  */
#line 419 "parser.yy"
    { (yyval.sval) = (yysemantic_stack_[(1) - (1)].sval);}
    break;

  case 55:

/* Line 678 of lalr1.cc  */
#line 420 "parser.yy"
    { (yyval.sval) = (yysemantic_stack_[(1) - (1)].sval);}
    break;

  case 56:

/* Line 678 of lalr1.cc  */
#line 424 "parser.yy"
    { (yyval.nval) = new Node(z::ID, * (yysemantic_stack_[(1) - (1)].sval)); }
    break;

  case 57:

/* Line 678 of lalr1.cc  */
#line 427 "parser.yy"
    { (yyval.nval) = new Node(z::SEQ,(yysemantic_stack_[(1) - (1)].nval)); }
    break;

  case 58:

/* Line 678 of lalr1.cc  */
#line 428 "parser.yy"
    { (yyval.nval) = (yysemantic_stack_[(3) - (1)].nval);  (yyval.nval)->addChild((yysemantic_stack_[(3) - (3)].nval)); }
    break;

  case 59:

/* Line 678 of lalr1.cc  */
#line 432 "parser.yy"
    { (yyval.nval) = new Node(z::SEQ,(yysemantic_stack_[(1) - (1)].nval)); }
    break;

  case 60:

/* Line 678 of lalr1.cc  */
#line 433 "parser.yy"
    { (yyval.nval) = (yysemantic_stack_[(3) - (1)].nval); (yyval.nval)->addChild((yysemantic_stack_[(3) - (3)].nval)); }
    break;

  case 61:

/* Line 678 of lalr1.cc  */
#line 437 "parser.yy"
    { (yyval.nval) = new Node(z::DECL, * (yysemantic_stack_[(3) - (1)].sval), (yysemantic_stack_[(3) - (3)].nval)); 
                          delete (yysemantic_stack_[(3) - (1)].sval);
                        }
    break;

  case 62:

/* Line 678 of lalr1.cc  */
#line 444 "parser.yy"
    { (yyval.nval) = new Node(z::SEQ);
     for (int i = 0; i != (yysemantic_stack_[(3) - (1)].nval)->arity(); i++) {
         (yyval.nval)->addChild(new Node(z::DECL, (yysemantic_stack_[(3) - (1)].nval)->child(i)->id, (yysemantic_stack_[(3) - (3)].nval)) );
     }
   }
    break;

  case 63:

/* Line 678 of lalr1.cc  */
#line 452 "parser.yy"
    { (yyval.nval) = (yysemantic_stack_[(3) - (2)].nval); }
    break;

  case 64:

/* Line 678 of lalr1.cc  */
#line 456 "parser.yy"
    { (yyval.nval) = new Node(z::SEQ, (yysemantic_stack_[(1) - (1)].nval)); }
    break;

  case 65:

/* Line 678 of lalr1.cc  */
#line 457 "parser.yy"
    { (yyval.nval) = (yysemantic_stack_[(3) - (1)].nval);           (yyval.nval)->addChild((yysemantic_stack_[(3) - (3)].nval)); }
    break;

  case 66:

/* Line 678 of lalr1.cc  */
#line 468 "parser.yy"
    { (yyval.nval) = (yysemantic_stack_[(1) - (1)].nval); }
    break;

  case 67:

/* Line 678 of lalr1.cc  */
#line 469 "parser.yy"
    { (yyval.nval) = (yysemantic_stack_[(3) - (2)].nval); (yyval.nval)->kind = z::ENUM_TY; }
    break;

  case 68:

/* Line 678 of lalr1.cc  */
#line 470 "parser.yy"
    { (yyval.nval) = new Node(z::ARRAY_TY,
                                               (yysemantic_stack_[(6) - (3)].nval),
                                               (yysemantic_stack_[(6) - (6)].nval));
                               }
    break;

  case 69:

/* Line 678 of lalr1.cc  */
#line 474 "parser.yy"
    { (yyval.nval) = (yysemantic_stack_[(3) - (2)].nval); (yyval.nval)->kind = z::RECORD_TY; }
    break;

  case 70:

/* Line 678 of lalr1.cc  */
#line 478 "parser.yy"
    { (yyval.nval) = (yysemantic_stack_[(1) - (1)].nval); }
    break;

  case 71:

/* Line 678 of lalr1.cc  */
#line 480 "parser.yy"
    { (yyval.nval) = (yysemantic_stack_[(3) - (1)].nval);     
            (yyval.nval)->appendChildren((yysemantic_stack_[(3) - (3)].nval)); 
          }
    break;

  case 72:

/* Line 678 of lalr1.cc  */
#line 486 "parser.yy"
    { const string& idstr = (yysemantic_stack_[(1) - (1)].nval)->id;
           if      (idstr == "pending")  (yyval.nval) = new Node(z::PENDING);
           else if (idstr == "integer")  (yyval.nval) = Node::int_ty;
           else if (idstr == "real")     (yyval.nval) = Node::real_ty;
           else if (idstr == "boolean")  (yyval.nval) = Node::bool_ty;
           else  {
               (yysemantic_stack_[(1) - (1)].nval)->kind = z::TYPE_ID;
               (yyval.nval) = (yysemantic_stack_[(1) - (1)].nval);
           }
         }
    break;

  case 73:

/* Line 678 of lalr1.cc  */
#line 506 "parser.yy"
    { (yyval.nval) = new Node(z::FORALL, (yysemantic_stack_[(6) - (3)].nval), (yysemantic_stack_[(6) - (5)].nval)); }
    break;

  case 74:

/* Line 678 of lalr1.cc  */
#line 508 "parser.yy"
    { (yyval.nval) = new Node(z::EXISTS, (yysemantic_stack_[(6) - (3)].nval), (yysemantic_stack_[(6) - (5)].nval)); }
    break;

  case 75:

/* Line 678 of lalr1.cc  */
#line 509 "parser.yy"
    {(yyval.nval) = new Node(z::IMPLIES, (yysemantic_stack_[(3) - (1)].nval), (yysemantic_stack_[(3) - (3)].nval)); }
    break;

  case 76:

/* Line 678 of lalr1.cc  */
#line 510 "parser.yy"
    {(yyval.nval) = new Node(z::IFF, (yysemantic_stack_[(3) - (1)].nval), (yysemantic_stack_[(3) - (3)].nval)); }
    break;

  case 77:

/* Line 678 of lalr1.cc  */
#line 511 "parser.yy"
    {(yyval.nval) = new Node(z::AND, (yysemantic_stack_[(3) - (1)].nval), (yysemantic_stack_[(3) - (3)].nval)); }
    break;

  case 78:

/* Line 678 of lalr1.cc  */
#line 512 "parser.yy"
    {(yyval.nval) = new Node(z::OR, (yysemantic_stack_[(3) - (1)].nval), (yysemantic_stack_[(3) - (3)].nval)); }
    break;

  case 79:

/* Line 678 of lalr1.cc  */
#line 513 "parser.yy"
    {(yyval.nval) = new Node(z::NOT, (yysemantic_stack_[(2) - (2)].nval)); }
    break;

  case 80:

/* Line 678 of lalr1.cc  */
#line 514 "parser.yy"
    {(yyval.nval) = new Node(z::EQ, (yysemantic_stack_[(3) - (1)].nval), (yysemantic_stack_[(3) - (3)].nval)); }
    break;

  case 81:

/* Line 678 of lalr1.cc  */
#line 515 "parser.yy"
    {(yyval.nval) = new Node(z::NOT, new Node(z::EQ, (yysemantic_stack_[(3) - (1)].nval), (yysemantic_stack_[(3) - (3)].nval))); }
    break;

  case 82:

/* Line 678 of lalr1.cc  */
#line 516 "parser.yy"
    {(yyval.nval) = new Node(z::LT, (yysemantic_stack_[(3) - (1)].nval), (yysemantic_stack_[(3) - (3)].nval)); }
    break;

  case 83:

/* Line 678 of lalr1.cc  */
#line 517 "parser.yy"
    {(yyval.nval) = new Node(z::GT, (yysemantic_stack_[(3) - (1)].nval), (yysemantic_stack_[(3) - (3)].nval)); }
    break;

  case 84:

/* Line 678 of lalr1.cc  */
#line 518 "parser.yy"
    {(yyval.nval) = new Node(z::LE, (yysemantic_stack_[(3) - (1)].nval), (yysemantic_stack_[(3) - (3)].nval)); }
    break;

  case 85:

/* Line 678 of lalr1.cc  */
#line 519 "parser.yy"
    {(yyval.nval) = new Node(z::GE, (yysemantic_stack_[(3) - (1)].nval), (yysemantic_stack_[(3) - (3)].nval)); }
    break;

  case 86:

/* Line 678 of lalr1.cc  */
#line 521 "parser.yy"
    {(yyval.nval) = (yysemantic_stack_[(2) - (2)].nval); }
    break;

  case 87:

/* Line 678 of lalr1.cc  */
#line 523 "parser.yy"
    {(yyval.nval) = new Node(z::UMINUS, (yysemantic_stack_[(2) - (2)].nval)); }
    break;

  case 88:

/* Line 678 of lalr1.cc  */
#line 524 "parser.yy"
    {(yyval.nval) = new Node(z::PLUS, (yysemantic_stack_[(3) - (1)].nval), (yysemantic_stack_[(3) - (3)].nval)); }
    break;

  case 89:

/* Line 678 of lalr1.cc  */
#line 525 "parser.yy"
    {(yyval.nval) = new Node(z::MINUS, (yysemantic_stack_[(3) - (1)].nval), (yysemantic_stack_[(3) - (3)].nval)); }
    break;

  case 90:

/* Line 678 of lalr1.cc  */
#line 526 "parser.yy"
    {(yyval.nval) = new Node(z::TIMES, (yysemantic_stack_[(3) - (1)].nval), (yysemantic_stack_[(3) - (3)].nval)); }
    break;

  case 91:

/* Line 678 of lalr1.cc  */
#line 527 "parser.yy"
    {(yyval.nval) = new Node(z::RDIV, (yysemantic_stack_[(3) - (1)].nval), (yysemantic_stack_[(3) - (3)].nval)); }
    break;

  case 92:

/* Line 678 of lalr1.cc  */
#line 528 "parser.yy"
    {(yyval.nval) = new Node(z::MOD, (yysemantic_stack_[(3) - (1)].nval), (yysemantic_stack_[(3) - (3)].nval)); }
    break;

  case 93:

/* Line 678 of lalr1.cc  */
#line 529 "parser.yy"
    {(yyval.nval) = new Node(z::IDIV, (yysemantic_stack_[(3) - (1)].nval), (yysemantic_stack_[(3) - (3)].nval)); }
    break;

  case 94:

/* Line 678 of lalr1.cc  */
#line 530 "parser.yy"
    {(yyval.nval) = new Node(z::EXP, (yysemantic_stack_[(3) - (1)].nval), (yysemantic_stack_[(3) - (3)].nval)); }
    break;

  case 95:

/* Line 678 of lalr1.cc  */
#line 531 "parser.yy"
    {(yyval.nval) = new Node(z::SUBRANGE, (yysemantic_stack_[(3) - (1)].nval), (yysemantic_stack_[(3) - (3)].nval)); }
    break;

  case 96:

/* Line 678 of lalr1.cc  */
#line 532 "parser.yy"
    {(yyval.nval) = (yysemantic_stack_[(3) - (2)].nval); }
    break;

  case 97:

/* Line 678 of lalr1.cc  */
#line 533 "parser.yy"
    {(yyval.nval) = new Node(z::NATNUM, * (yysemantic_stack_[(1) - (1)].sval)); delete (yysemantic_stack_[(1) - (1)].sval); }
    break;

  case 98:

/* Line 678 of lalr1.cc  */
#line 534 "parser.yy"
    {(yyval.nval) = (yysemantic_stack_[(1) - (1)].nval);}
    break;

  case 99:

/* Line 678 of lalr1.cc  */
#line 535 "parser.yy"
    {(yyval.nval) = (yysemantic_stack_[(1) - (1)].nval);}
    break;

  case 100:

/* Line 678 of lalr1.cc  */
#line 537 "parser.yy"
    { string& idstr = * (yysemantic_stack_[(4) - (1)].sval);
        int nargs = (yysemantic_stack_[(4) - (3)].nval)->arity(); // nargs > 0 by def of aexpseq
        if      (idstr == "element" && nargs == 2) {
            (yyval.nval) = (yysemantic_stack_[(4) - (3)].nval);
            (yyval.nval)->kind = z::ARR_ELEMENT;
        }
        else if (idstr == "update" && nargs == 3) {
             (yyval.nval) = (yysemantic_stack_[(4) - (3)].nval);
             (yyval.nval)->kind = z::ARR_UPDATE;
        }
        else if (idstr.size() >= 4
                 && string(idstr, 0, 4) == "mk__") {

            // array or record constructor
            if ((yysemantic_stack_[(4) - (3)].nval)->child(0)->kind == z::ASSIGN
                && (yysemantic_stack_[(4) - (3)].nval)->child(0)->arity() == 1) {
                (yyval.nval) = (yysemantic_stack_[(4) - (3)].nval);
                (yyval.nval)->kind = z::MK_RECORD;
                (yyval.nval)->id = idstr.erase(0,4);
            }
            else {
                (yyval.nval) = (yysemantic_stack_[(4) - (3)].nval);
                (yyval.nval)->kind = z::MK_ARRAY;
                (yyval.nval)->id = idstr.erase(0,4);
            }
        }
        else if (idstr.size() >= 4
                 && string(idstr, 0, 4) == "fld_" 
                 && nargs == 1) 
        {
            (yyval.nval) = (yysemantic_stack_[(4) - (3)].nval);
            (yyval.nval)->kind = z::RCD_ELEMENT;
            (yyval.nval)->id = idstr.erase(0,4);
        }
        else if (idstr.size() >= 4 
                 && string(idstr, 0, 4) == "upf_" 
                 && nargs == 2) {
            (yyval.nval) = (yysemantic_stack_[(4) - (3)].nval);
            (yyval.nval)->kind = z::RCD_UPDATE;
            (yyval.nval)->id = idstr.erase(0,4);
        }
        else if (idstr == "succ") {
            (yyval.nval) = (yysemantic_stack_[(4) - (3)].nval);
            (yyval.nval)->kind = z::SUCC;
        }
        else if (idstr == "pred") {
            (yyval.nval) = (yysemantic_stack_[(4) - (3)].nval);
            (yyval.nval)->kind = z::PRED;
        }
        else if (idstr == "odd") {
            (yyval.nval) = (yysemantic_stack_[(4) - (3)].nval);
            (yyval.nval)->kind = z::ODD;
        }
        else if (idstr == "abs") {
            (yyval.nval) = (yysemantic_stack_[(4) - (3)].nval);
            (yyval.nval)->kind = z::ABS;
        }
        else if (idstr == "sqr") {
            (yyval.nval) = (yysemantic_stack_[(4) - (3)].nval);
            (yyval.nval)->kind = z::SQR;
        }
        else {
            (yyval.nval) = (yysemantic_stack_[(4) - (3)].nval);
            (yyval.nval)->kind = z::FUN_AP;
            (yyval.nval)->id = idstr;
        }

        delete (yysemantic_stack_[(4) - (1)].sval);
      }
    break;

  case 101:

/* Line 678 of lalr1.cc  */
#line 609 "parser.yy"
    { (yyval.nval) = new Node(z::SEQ,(yysemantic_stack_[(1) - (1)].nval)); }
    break;

  case 102:

/* Line 678 of lalr1.cc  */
#line 610 "parser.yy"
    { (yyval.nval) = (yysemantic_stack_[(3) - (1)].nval);           (yyval.nval)->addChild((yysemantic_stack_[(3) - (3)].nval)); }
    break;

  case 103:

/* Line 678 of lalr1.cc  */
#line 614 "parser.yy"
    {(yyval.nval) = (yysemantic_stack_[(1) - (1)].nval); }
    break;

  case 104:

/* Line 678 of lalr1.cc  */
#line 616 "parser.yy"
    {  if ((yysemantic_stack_[(3) - (1)].nval)->arity() == 1) {
              if ((yysemantic_stack_[(3) - (1)].nval)->child(0)->kind == z::ID)
                   (yyval.nval) = new Node(z::ASSIGN, (yysemantic_stack_[(3) - (1)].nval)->child(0)->id, (yysemantic_stack_[(3) - (3)].nval));
              else
                   (yyval.nval) = new Node(z::ASSIGN, (yysemantic_stack_[(3) - (1)].nval)->child(0), (yysemantic_stack_[(3) - (3)].nval));
          }
          else
              (yyval.nval) = new Node(z::ASSIGN, (yysemantic_stack_[(3) - (1)].nval), (yysemantic_stack_[(3) - (3)].nval));
       }
    break;

  case 105:

/* Line 678 of lalr1.cc  */
#line 628 "parser.yy"
    {(yyval.nval) = new Node(z::INDEX_AND,(yysemantic_stack_[(1) - (1)].nval)); }
    break;

  case 106:

/* Line 678 of lalr1.cc  */
#line 630 "parser.yy"
    {
            (yyval.nval) = (yysemantic_stack_[(3) - (1)].nval);
            (yyval.nval)->addChild((yysemantic_stack_[(3) - (3)].nval));
         }
    break;

  case 107:

/* Line 678 of lalr1.cc  */
#line 636 "parser.yy"
    { const string& idstr = (yysemantic_stack_[(1) - (1)].nval)->id;
           if      (idstr == "pending") (yyval.nval) = new Node(z::PENDING);
           else if (idstr == "true")    (yyval.nval) = new Node(z::TRUE);
           else if (idstr == "false")   (yyval.nval) = new Node(z::FALSE);
           else                         (yyval.nval) = (yysemantic_stack_[(1) - (1)].nval);
         }
    break;



/* Line 678 of lalr1.cc  */
#line 1306 "parser.tab.cc"
	default:
          break;
      }
    YY_SYMBOL_PRINT ("-> $$ =", yyr1_[yyn], &yyval, &yyloc);

    yypop_ (yylen);
    yylen = 0;
    YY_STACK_PRINT ();

    yysemantic_stack_.push (yyval);
    yylocation_stack_.push (yyloc);

    /* Shift the result of the reduction.  */
    yyn = yyr1_[yyn];
    yystate = yypgoto_[yyn - yyntokens_] + yystate_stack_[0];
    if (0 <= yystate && yystate <= yylast_
	&& yycheck_[yystate] == yystate_stack_[0])
      yystate = yytable_[yystate];
    else
      yystate = yydefgoto_[yyn - yyntokens_];
    goto yynewstate;

  /*------------------------------------.
  | yyerrlab -- here on detecting error |
  `------------------------------------*/
  yyerrlab:
    /* If not already recovering from an error, report this error.  */
    if (!yyerrstatus_)
      {
	++yynerrs_;
	error (yylloc, yysyntax_error_ (yystate, yytoken));
      }

    yyerror_range[0] = yylloc;
    if (yyerrstatus_ == 3)
      {
	/* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

	if (yychar <= yyeof_)
	  {
	  /* Return failure if at end of input.  */
	  if (yychar == yyeof_)
	    YYABORT;
	  }
	else
	  {
	    yydestruct_ ("Error: discarding", yytoken, &yylval, &yylloc);
	    yychar = yyempty_;
	  }
      }

    /* Else will try to reuse lookahead token after shifting the error
       token.  */
    goto yyerrlab1;


  /*---------------------------------------------------.
  | yyerrorlab -- error raised explicitly by YYERROR.  |
  `---------------------------------------------------*/
  yyerrorlab:

    /* Pacify compilers like GCC when the user code never invokes
       YYERROR and the label yyerrorlab therefore never appears in user
       code.  */
    if (false)
      goto yyerrorlab;

    yyerror_range[0] = yylocation_stack_[yylen - 1];
    /* Do not reclaim the symbols of the rule which action triggered
       this YYERROR.  */
    yypop_ (yylen);
    yylen = 0;
    yystate = yystate_stack_[0];
    goto yyerrlab1;

  /*-------------------------------------------------------------.
  | yyerrlab1 -- common code for both syntax error and YYERROR.  |
  `-------------------------------------------------------------*/
  yyerrlab1:
    yyerrstatus_ = 3;	/* Each real token shifted decrements this.  */

    for (;;)
      {
	yyn = yypact_[yystate];
	if (yyn != yypact_ninf_)
	{
	  yyn += yyterror_;
	  if (0 <= yyn && yyn <= yylast_ && yycheck_[yyn] == yyterror_)
	    {
	      yyn = yytable_[yyn];
	      if (0 < yyn)
		break;
	    }
	}

	/* Pop the current state because it cannot handle the error token.  */
	if (yystate_stack_.height () == 1)
	YYABORT;

	yyerror_range[0] = yylocation_stack_[0];
	yydestruct_ ("Error: popping",
		     yystos_[yystate],
		     &yysemantic_stack_[0], &yylocation_stack_[0]);
	yypop_ ();
	yystate = yystate_stack_[0];
	YY_STACK_PRINT ();
      }

    yyerror_range[1] = yylloc;
    // Using YYLLOC is tempting, but would change the location of
    // the lookahead.  YYLOC is available though.
    YYLLOC_DEFAULT (yyloc, (yyerror_range - 1), 2);
    yysemantic_stack_.push (yylval);
    yylocation_stack_.push (yyloc);

    /* Shift the error token.  */
    YY_SYMBOL_PRINT ("Shifting", yystos_[yyn],
		     &yysemantic_stack_[0], &yylocation_stack_[0]);

    yystate = yyn;
    goto yynewstate;

    /* Accept.  */
  yyacceptlab:
    yyresult = 0;
    goto yyreturn;

    /* Abort.  */
  yyabortlab:
    yyresult = 1;
    goto yyreturn;

  yyreturn:
    if (yychar != yyempty_)
      yydestruct_ ("Cleanup: discarding lookahead", yytoken, &yylval, &yylloc);

    /* Do not reclaim the symbols of the rule which action triggered
       this YYABORT or YYACCEPT.  */
    yypop_ (yylen);
    while (yystate_stack_.height () != 1)
      {
	yydestruct_ ("Cleanup: popping",
		   yystos_[yystate_stack_[0]],
		   &yysemantic_stack_[0],
		   &yylocation_stack_[0]);
	yypop_ ();
      }

    return yyresult;
  }

  // Generate an error message.
  std::string
  parser::yysyntax_error_ (int yystate, int tok)
  {
    std::string res;
    YYUSE (yystate);
#if YYERROR_VERBOSE
    int yyn = yypact_[yystate];
    if (yypact_ninf_ < yyn && yyn <= yylast_)
      {
	/* Start YYX at -YYN if negative to avoid negative indexes in
	   YYCHECK.  */
	int yyxbegin = yyn < 0 ? -yyn : 0;

	/* Stay within bounds of both yycheck and yytname.  */
	int yychecklim = yylast_ - yyn + 1;
	int yyxend = yychecklim < yyntokens_ ? yychecklim : yyntokens_;
	int count = 0;
	for (int x = yyxbegin; x < yyxend; ++x)
	  if (yycheck_[x + yyn] == x && x != yyterror_)
	    ++count;

	// FIXME: This method of building the message is not compatible
	// with internationalization.  It should work like yacc.c does it.
	// That is, first build a string that looks like this:
	// "syntax error, unexpected %s or %s or %s"
	// Then, invoke YY_ on this string.
	// Finally, use the string as a format to output
	// yytname_[tok], etc.
	// Until this gets fixed, this message appears in English only.
	res = "syntax error, unexpected ";
	res += yytnamerr_ (yytname_[tok]);
	if (count < 5)
	  {
	    count = 0;
	    for (int x = yyxbegin; x < yyxend; ++x)
	      if (yycheck_[x + yyn] == x && x != yyterror_)
		{
		  res += (!count++) ? ", expecting " : " or ";
		  res += yytnamerr_ (yytname_[x]);
		}
	  }
      }
    else
#endif
      res = YY_("syntax error");
    return res;
  }


  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
  const short int parser::yypact_ninf_ = -152;
  const short int
  parser::yypact_[] =
  {
        52,     2,  -152,     7,    24,  -152,   -17,  -152,  -152,    46,
      44,  -152,     7,  -152,  -152,  -152,  -152,  -152,    46,  -152,
    -152,  -152,  -152,  -152,  -152,    59,  -152,    41,    70,  -152,
    -152,    64,    46,  -152,    53,    75,  -152,    83,   102,    79,
      75,  -152,    10,    17,   105,    73,  -152,   103,    46,    46,
      46,    46,  -152,   238,   238,  -152,   106,   109,  -152,   238,
     238,   238,    55,  -152,   110,  -152,  -152,   579,  -152,   238,
      46,   -27,  -152,    21,    65,   115,   116,    50,   645,   301,
      46,    46,    46,   407,    61,    61,   238,  -152,   238,   238,
     118,   238,   238,   238,   238,   238,   238,   238,   238,   238,
     238,   238,   238,   238,   238,   238,   238,   238,   112,   391,
     113,   122,  -152,  -152,   101,   101,   101,   101,   101,  -152,
     238,  -152,  -152,    36,   119,   123,  -152,    46,   441,     6,
    -152,    -7,   673,    46,   551,   551,   338,   407,   477,   477,
     477,   477,   477,   477,    27,    27,    61,    61,    61,    61,
    -152,  -152,   238,   238,  -152,   134,  -152,   238,   108,    46,
     138,    46,  -152,   127,  -152,    18,  -152,   135,   137,    91,
     645,   101,    46,   238,   238,  -152,   238,   238,   238,   141,
      51,  -152,   598,   626,  -152,   511,   146,  -152,    33,   101,
    -152,    -1,  -152,   147,   101,  -152,  -152,   238,  -152,  -152,
     320,   372,  -152,   645,   645,   101,  -152,    46,   136,   134,
    -152,  -152,   238,  -152,    67,    46,  -152,   101,  -152,   461,
    -152,  -152,  -152,  -152,  -152,  -152,   530,   125,  -152,   143,
    -152,  -152,   101,  -152,  -152
  };

  /* YYDEFACT[S] -- default rule to reduce with in state S when YYTABLE
     doesn't specify something else to do.  Zero means the default is an
     error.  */
  const unsigned char
  parser::yydefact_[] =
  {
         0,     0,    27,     0,     0,     2,     0,     3,     4,    20,
       0,     5,    36,    37,     1,     8,     9,     7,     0,    55,
      53,    52,    51,    54,    50,    19,    28,     0,     0,    38,
      56,     0,     0,    21,     0,     0,    10,     0,     0,     0,
      39,    40,     0,     0,     0,    44,    41,     0,     0,     0,
       0,     0,    11,     0,     0,    27,     0,     0,    97,     0,
       0,     0,     0,    24,    56,   107,    98,     0,    99,     0,
       0,     0,     6,     0,     0,     0,     0,     0,    64,     0,
      23,     0,     0,    79,    87,    86,     0,    27,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    47,    45,     0,     0,     0,     0,     0,    63,
       0,    96,    57,     0,     0,     0,    25,    22,   103,     0,
     101,     0,    95,     0,    75,    76,    78,    77,    84,    82,
      85,    83,    81,    80,    89,    88,    92,    93,    91,    90,
      94,    29,     0,     0,    31,     0,    43,     0,    42,     0,
       0,     0,    72,     0,    66,     0,    17,     0,     0,     0,
      65,     0,     0,     0,     0,   100,     0,     0,     0,     0,
       0,    59,     0,    34,    32,     0,     0,    48,     0,     0,
      70,     0,    16,     0,     0,    12,    14,     0,    62,    58,
       0,     0,   102,   106,   104,     0,    26,     0,    34,     0,
      30,    46,     0,    67,     0,     0,    69,     0,    18,     0,
      74,    73,    61,    60,    33,    35,     0,     0,    71,     0,
      13,    49,     0,    15,    68
  };

  /* YYPGOTO[NTERM-NUM].  */
  const short int
  parser::yypgoto_[] =
  {
      -152,  -152,  -152,  -152,  -152,  -152,  -152,   -29,  -152,  -152,
    -152,  -152,    78,   -52,  -152,  -152,   -47,  -152,  -152,   153,
    -152,   126,  -152,  -152,  -152,  -152,    -9,   -16,     8,  -152,
     -39,   -81,  -151,  -152,  -102,  -152,  -152,   133,  -152,    -6,
    -152,  -152
  };

  /* YYDEFGOTO[NTERM-NUM].  */
  const short int
  parser::yydefgoto_[] =
  {
        -1,     4,     5,     7,    18,    42,    52,   165,     8,    25,
      33,    62,    63,     9,    26,   108,   210,    11,    12,    13,
      40,    41,    71,   113,   158,   187,    64,    65,   123,   180,
     181,   124,    66,    77,   166,   191,   164,    67,   129,   130,
     131,    68
  };

  /* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule which
     number is the opposite.  If zero, do what YYDEFACT says.  */
  const signed char parser::yytable_ninf_ = -106;
  const short int
  parser::yytable_[] =
  {
        27,   125,    31,    80,   184,     6,   177,    15,    16,    30,
     111,    10,   163,   215,   167,   168,   169,   175,   176,   112,
      19,    17,   216,    37,    14,    53,   178,    54,   114,   193,
     194,   115,    55,    47,    48,   127,    49,    50,    51,    73,
      74,    75,    76,   171,   213,   172,    56,    57,   172,    19,
      28,    34,    20,    21,    22,    23,    24,    58,   225,   119,
     206,   110,   120,   207,    32,   122,   122,    59,    86,   198,
      87,    27,    30,    30,    60,    61,   227,    35,    36,   194,
     190,    20,    21,    22,    23,    24,   103,   104,   105,   106,
      43,   107,   218,    38,    45,     1,     2,     3,   162,   162,
     162,   162,   162,   222,    19,    30,    30,    30,    30,    30,
      39,   159,    69,    44,    70,   229,    81,    72,    27,    82,
      88,   116,   117,   118,   179,   107,   133,   151,   156,   157,
     234,   173,   160,   161,   228,   174,    20,    21,    22,    23,
      24,   192,    53,   122,   186,   122,   189,   197,   205,   195,
      30,   196,    30,   212,   217,   162,   199,   233,   209,   232,
     214,   224,    30,    30,   126,    29,    46,   188,   223,     0,
     202,     0,     0,   162,     0,     0,     0,     0,   162,     0,
      30,     0,     0,     0,     0,    30,    78,    79,     0,   162,
       0,     0,    83,    84,    85,     0,    30,     0,   179,   122,
       0,   162,   109,     0,     0,     0,    30,     0,    30,     0,
       0,     0,     0,     0,     0,     0,   162,     0,     0,     0,
       0,   128,   132,    30,   134,   135,   136,   137,   138,   139,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,    19,     0,     0,     0,     0,    53,     0,    54,     0,
       0,     0,     0,   170,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    56,    57,     0,
       0,     0,     0,    20,    21,    22,    23,    24,    58,     0,
       0,     0,     0,     0,     0,   182,   183,     0,    59,     0,
     185,     0,     0,     0,     0,    60,    61,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   200,   201,     0,   128,
     203,   204,   121,     0,     0,     0,     0,    89,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     219,   220,     0,     0,     0,     0,    89,     0,     0,     0,
       0,     0,     0,     0,     0,   226,     0,    91,    92,    93,
      94,     0,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,     0,   107,    91,    92,    93,    94,
       0,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   221,   107,     0,     0,    94,    89,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,     0,   107,     0,   152,     0,     0,    89,     0,   153,
     154,   155,     0,     0,     0,     0,     0,     0,    91,    92,
      93,    94,     0,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,     0,   107,    91,    92,    93,
      94,     0,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,  -105,   107,     0,    89,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
       0,   107,     0,     0,  -105,   230,     0,    89,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    91,    92,    93,
      94,     0,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,     0,   107,     0,    91,    92,    93,
      94,     0,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,     0,   107,   211,    89,  -106,  -106,
    -106,  -106,  -106,  -106,   101,   102,   103,   104,   105,   106,
       0,   107,     0,     0,     0,   231,    89,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    91,    92,    93,
      94,     0,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,     0,   107,    91,    92,    93,    94,
       0,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,     0,   107,    89,    90,  -106,  -106,    93,
      94,     0,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,    89,   107,     0,     0,     0,   208,
       0,     0,     0,     0,     0,    91,    92,    93,    94,     0,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,    89,   107,    91,    92,    93,    94,   209,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,    89,   107,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    91,    92,    93,    94,     0,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,  -106,
     107,    91,    92,    93,    94,     0,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,     0,   107,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    91,
      92,    93,    94,     0,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,     0,   107
  };

  /* YYCHECK.  */
  const short int
  parser::yycheck_[] =
  {
         9,    82,    18,    55,   155,     3,    13,    24,    25,    18,
      37,     4,   114,    14,   116,   117,   118,    11,    12,    46,
       3,    38,    23,    32,     0,     8,    33,    10,     7,    11,
      12,    10,    15,    23,    24,    87,    26,    27,    28,    48,
      49,    50,    51,     7,    11,    12,    29,    30,    12,     3,
       6,    10,    35,    36,    37,    38,    39,    40,   209,     9,
       9,    70,    12,    12,     5,    81,    82,    50,    13,   171,
      15,    80,    81,    82,    57,    58,     9,     7,    14,    12,
     161,    35,    36,    37,    38,    39,    59,    60,    61,    62,
       7,    64,   194,    40,    15,    43,    44,    45,   114,   115,
     116,   117,   118,   205,     3,   114,   115,   116,   117,   118,
      35,    10,     7,    11,    41,   217,    10,    14,   127,    10,
      10,    56,     7,     7,   133,    64,     8,    15,    15,     7,
     232,    12,    31,    32,   215,    12,    35,    36,    37,    38,
      39,    14,     8,   159,    36,   161,     8,    56,     7,    14,
     159,    14,   161,     7,     7,   171,   172,    14,    22,    34,
     189,   208,   171,   172,    86,    12,    40,   159,   207,    -1,
     176,    -1,    -1,   189,    -1,    -1,    -1,    -1,   194,    -1,
     189,    -1,    -1,    -1,    -1,   194,    53,    54,    -1,   205,
      -1,    -1,    59,    60,    61,    -1,   205,    -1,   207,   215,
      -1,   217,    69,    -1,    -1,    -1,   215,    -1,   217,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   232,    -1,    -1,    -1,
      -1,    88,    89,   232,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,     3,    -1,    -1,    -1,    -1,     8,    -1,    10,    -1,
      -1,    -1,    -1,   120,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    29,    30,    -1,
      -1,    -1,    -1,    35,    36,    37,    38,    39,    40,    -1,
      -1,    -1,    -1,    -1,    -1,   152,   153,    -1,    50,    -1,
     157,    -1,    -1,    -1,    -1,    57,    58,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   173,   174,    -1,   176,
     177,   178,    11,    -1,    -1,    -1,    -1,    16,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     197,    11,    -1,    -1,    -1,    -1,    16,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   212,    -1,    46,    47,    48,
      49,    -1,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    -1,    64,    46,    47,    48,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    11,    64,    -1,    -1,    49,    16,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    -1,    64,    -1,    13,    -1,    -1,    16,    -1,    18,
      19,    20,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,
      48,    49,    -1,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    -1,    64,    46,    47,    48,
      49,    -1,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    13,    64,    -1,    16,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      -1,    64,    -1,    -1,    33,    14,    -1,    16,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,    48,
      49,    -1,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    -1,    64,    -1,    46,    47,    48,
      49,    -1,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    -1,    64,    15,    16,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      -1,    64,    -1,    -1,    -1,    15,    16,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,    48,
      49,    -1,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    -1,    64,    46,    47,    48,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    -1,    64,    16,    17,    46,    47,    48,
      49,    -1,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    16,    64,    -1,    -1,    -1,    21,
      -1,    -1,    -1,    -1,    -1,    46,    47,    48,    49,    -1,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    16,    64,    46,    47,    48,    49,    22,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    16,    64,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    46,    47,    48,    49,    -1,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    16,
      64,    46,    47,    48,    49,    -1,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    -1,    64,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    46,
      47,    48,    49,    -1,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    -1,    64
  };

  /* STOS_[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
  const unsigned char
  parser::yystos_[] =
  {
         0,    43,    44,    45,    66,    67,     3,    68,    73,    78,
       4,    82,    83,    84,     0,    24,    25,    38,    69,     3,
      35,    36,    37,    38,    39,    74,    79,    91,     6,    84,
      91,    92,     5,    75,    10,     7,    14,    91,    40,    35,
      85,    86,    70,     7,    11,    15,    86,    23,    24,    26,
      27,    28,    71,     8,    10,    15,    29,    30,    40,    50,
      57,    58,    76,    77,    91,    92,    97,   102,   106,     7,
      41,    87,    14,    91,    91,    91,    91,    98,   102,   102,
      78,    10,    10,   102,   102,   102,    13,    15,    10,    16,
      17,    46,    47,    48,    49,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    64,    80,   102,
      91,    37,    46,    88,     7,    10,    56,     7,     7,     9,
      12,    11,    92,    93,    96,    96,    77,    78,   102,   103,
     104,   105,   102,     8,   102,   102,   102,   102,   102,   102,
     102,   102,   102,   102,   102,   102,   102,   102,   102,   102,
     102,    15,    13,    18,    19,    20,    15,     7,    89,    10,
      31,    32,    92,    99,   101,    72,    99,    99,    99,    99,
     102,     7,    12,    12,    12,    11,    12,    13,    33,    91,
      94,    95,   102,   102,    97,   102,    36,    90,    93,     8,
      96,   100,    14,    11,    12,    14,    14,    56,    99,    92,
     102,   102,   104,   102,   102,     7,     9,    12,    21,    22,
      81,    15,     7,    11,    72,    14,    23,     7,    99,   102,
      11,    11,    99,    95,    81,    97,   102,     9,    96,    99,
      14,    15,    34,    14,    99
  };

#if YYDEBUG
  /* TOKEN_NUMBER_[YYLEX-NUM] -- Internal symbol number corresponding
     to YYLEX-NUM.  */
  const unsigned short int
  parser::yytoken_number_[] =
  {
         0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319
  };
#endif

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
  const unsigned char
  parser::yyr1_[] =
  {
         0,    65,    66,    67,    67,    67,    68,    69,    69,    69,
      70,    70,    71,    71,    71,    71,    71,    72,    72,    73,
      74,    74,    75,    75,    76,    76,    77,    78,    78,    79,
      80,    80,    80,    80,    81,    81,    82,    83,    83,    84,
      85,    85,    86,    86,    87,    87,    88,    89,    89,    90,
      91,    91,    91,    91,    91,    91,    92,    93,    93,    94,
      94,    95,    96,    97,    98,    98,    99,    99,    99,    99,
     100,   100,   101,   102,   102,   102,   102,   102,   102,   102,
     102,   102,   102,   102,   102,   102,   102,   102,   102,   102,
     102,   102,   102,   102,   102,   102,   102,   102,   102,   102,
     102,   103,   103,   104,   104,   105,   105,   106
  };

  /* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
  const unsigned char
  parser::yyr2_[] =
  {
         0,     2,     1,     2,     2,     2,     7,     1,     1,     1,
       0,     2,     5,     7,     5,     8,     5,     1,     3,     2,
       0,     2,     6,     5,     1,     3,     5,     0,     2,     7,
       4,     2,     3,     5,     0,     2,     1,     1,     2,     4,
       1,     2,     5,     5,     0,     2,     4,     0,     2,     4,
       1,     1,     1,     1,     1,     1,     1,     1,     3,     1,
       3,     3,     3,     3,     1,     3,     1,     3,     6,     3,
       1,     3,     1,     6,     6,     3,     3,     3,     3,     2,
       3,     3,     3,     3,     3,     3,     2,     2,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     1,     1,     1,
       4,     1,     3,     1,     3,     1,     3,     1
  };

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
  /* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
     First, the terminals, then, starting at \a yyntokens_, nonterminals.  */
  const char*
  const parser::yytname_[] =
  {
    "FILE_END", "error", "$undefined", "TITLE", "FOR", "RULE_FAMILY",
  "GOAL_ORIGINS", "COLON", "LSB", "RSB", "LPAREN", "RPAREN", "COMMA",
  "AMPERSAND", "SEMIC", "DOT", "DOTDOT", "REQUIRES", "MAY_BE_REPLACED_BY",
  "MAY_BE_DEDUCED", "MAY_BE_DEDUCED_FROM", "ARE_INTERCHANGEABLE", "IF",
  "END", "FUNCTION", "PROCEDURE", "TYPE", "VAR", "CONST", "FOR_SOME",
  "FOR_ALL", "ARRAY", "RECORD", "ASSIGN", "OF", "SUBPROG_ID", "CONCL_ID",
  "HYP_ID", "TASK_TYPE", "ID", "NATNUM", "TRIPLESTAR", "TRIPLEBANG",
  "START_FDL_FILE", "START_RULE_FILE", "START_VCG_FILE", "IMPLIES", "IFF",
  "OR", "AND", "NOT", "LE", "LT", "GE", "GT", "NE", "EQ", "MINUS", "PLUS",
  "MOD", "DIV", "SLASH", "STAR", "UMINUS", "STARSTAR", "$accept", "top",
  "file", "fdl_file", "program_kind", "fdl_decls", "fdl_decl", "types",
  "rule_file", "rule_families", "rule_family", "typeassums", "typeassum",
  "rules", "rule", "rule_body", "rule_condition", "vcg_file", "goalsets",
  "goalset", "goals", "goal", "hyps", "hyp", "concls", "concl", "id_str",
  "id", "ids", "typedecls", "typedecl", "multidecl", "expseq", "expseq1",
  "type", "recordtypedecls", "type_id", "exp", "aexpseq", "aexp",
  "indexset", "exp_id", 0
  };
#endif

#if YYDEBUG
  /* YYRHS -- A `-1'-separated list of the rules' RHS.  */
  const parser::rhs_number_type
  parser::yyrhs_[] =
  {
        66,     0,    -1,    67,    -1,    43,    68,    -1,    44,    73,
      -1,    45,    82,    -1,     3,    69,    92,    14,    70,    23,
      14,    -1,    38,    -1,    24,    -1,    25,    -1,    -1,    70,
      71,    -1,    26,    91,    56,    99,    14,    -1,    28,    91,
       7,    99,    56,   102,    14,    -1,    27,    91,     7,    99,
      14,    -1,    24,    91,    10,    72,    11,     7,    99,    14,
      -1,    24,    91,     7,    99,    14,    -1,    99,    -1,    72,
      12,    99,    -1,    78,    74,    -1,    -1,    74,    75,    -1,
       5,    91,     7,    76,    15,    78,    -1,     5,    91,     7,
      15,    78,    -1,    77,    -1,    76,    13,    77,    -1,   102,
      17,     8,    94,     9,    -1,    -1,    78,    79,    -1,    91,
      10,    40,    11,     7,    80,    15,    -1,   102,    18,   102,
      81,    -1,   102,    19,    -1,   102,    20,    97,    -1,   102,
      13,   102,    21,    81,    -1,    -1,    22,    97,    -1,    83,
      -1,    84,    -1,    83,    84,    -1,     4,     6,     7,    85,
      -1,    86,    -1,    85,    86,    -1,    35,    15,    87,    46,
      89,    -1,    35,    15,    41,    91,    15,    -1,    -1,    87,
      88,    -1,    37,     7,   102,    15,    -1,    -1,    89,    90,
      -1,    36,     7,   102,    15,    -1,    39,    -1,    37,    -1,
      36,    -1,    35,    -1,    38,    -1,     3,    -1,    91,    -1,
      92,    -1,    93,    12,    92,    -1,    95,    -1,    94,    12,
      95,    -1,    91,     7,    99,    -1,    93,     7,    99,    -1,
       8,    98,     9,    -1,   102,    -1,    98,    12,   102,    -1,
     101,    -1,    10,    93,    11,    -1,    31,     8,    72,     9,
      34,    99,    -1,    32,   100,    23,    -1,    96,    -1,   100,
      14,    96,    -1,    92,    -1,    30,    10,    96,    12,   102,
      11,    -1,    29,    10,    96,    12,   102,    11,    -1,   102,
      46,   102,    -1,   102,    47,   102,    -1,   102,    49,   102,
      -1,   102,    48,   102,    -1,    50,   102,    -1,   102,    56,
     102,    -1,   102,    55,   102,    -1,   102,    52,   102,    -1,
     102,    54,   102,    -1,   102,    51,   102,    -1,   102,    53,
     102,    -1,    58,   102,    -1,    57,   102,    -1,   102,    58,
     102,    -1,   102,    57,   102,    -1,   102,    62,   102,    -1,
     102,    61,   102,    -1,   102,    59,   102,    -1,   102,    60,
     102,    -1,   102,    64,   102,    -1,   102,    16,   102,    -1,
      10,   102,    11,    -1,    40,    -1,    97,    -1,   106,    -1,
      91,    10,   103,    11,    -1,   104,    -1,   103,    12,   104,
      -1,   102,    -1,   105,    33,   102,    -1,   102,    -1,   105,
      13,   102,    -1,    92,    -1
  };

  /* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
     YYRHS.  */
  const unsigned short int
  parser::yyprhs_[] =
  {
         0,     0,     3,     5,     8,    11,    14,    22,    24,    26,
      28,    29,    32,    38,    46,    52,    61,    67,    69,    73,
      76,    77,    80,    87,    93,    95,    99,   105,   106,   109,
     117,   122,   125,   129,   135,   136,   139,   141,   143,   146,
     151,   153,   156,   162,   168,   169,   172,   177,   178,   181,
     186,   188,   190,   192,   194,   196,   198,   200,   202,   206,
     208,   212,   216,   220,   224,   226,   230,   232,   236,   243,
     247,   249,   253,   255,   262,   269,   273,   277,   281,   285,
     288,   292,   296,   300,   304,   308,   312,   315,   318,   322,
     326,   330,   334,   338,   342,   346,   350,   354,   356,   358,
     360,   365,   367,   371,   373,   377,   379,   383
  };

  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
  const unsigned short int
  parser::yyrline_[] =
  {
         0,   190,   190,   194,   195,   196,   205,   214,   215,   216,
     220,   221,   225,   232,   239,   244,   251,   258,   259,   274,
     285,   286,   290,   292,   301,   302,   306,   310,   311,   315,
     319,   323,   325,   327,   334,   335,   345,   351,   353,   358,
     370,   371,   375,   382,   390,   391,   394,   398,   399,   402,
     415,   416,   417,   418,   419,   420,   424,   427,   428,   432,
     433,   437,   443,   452,   456,   457,   468,   469,   470,   474,
     478,   479,   486,   505,   507,   509,   510,   511,   512,   513,
     514,   515,   516,   517,   518,   519,   520,   522,   524,   525,
     526,   527,   528,   529,   530,   531,   532,   533,   534,   535,
     536,   609,   610,   614,   615,   628,   629,   636
  };

  // Print the state stack on the debug stream.
  void
  parser::yystack_print_ ()
  {
    *yycdebug_ << "Stack now";
    for (state_stack_type::const_iterator i = yystate_stack_.begin ();
	 i != yystate_stack_.end (); ++i)
      *yycdebug_ << ' ' << *i;
    *yycdebug_ << std::endl;
  }

  // Report on the debug stream that the rule \a yyrule is going to be reduced.
  void
  parser::yy_reduce_print_ (int yyrule)
  {
    unsigned int yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    /* Print the symbols being reduced, and their result.  */
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1
	       << " (line " << yylno << "):" << std::endl;
    /* The symbols being reduced.  */
    for (int yyi = 0; yyi < yynrhs; yyi++)
      YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
		       yyrhs_[yyprhs_[yyrule] + yyi],
		       &(yysemantic_stack_[(yynrhs) - (yyi + 1)]),
		       &(yylocation_stack_[(yynrhs) - (yyi + 1)]));
  }
#endif // YYDEBUG

  /* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
  parser::token_number_type
  parser::yytranslate_ (int t)
  {
    static
    const token_number_type
    translate_table[] =
    {
           0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64
    };
    if ((unsigned int) t <= yyuser_token_number_max_)
      return translate_table[t];
    else
      return yyundef_token_;
  }

  const int parser::yyeof_ = 0;
  const int parser::yylast_ = 737;
  const int parser::yynnts_ = 42;
  const int parser::yyempty_ = -2;
  const int parser::yyfinal_ = 14;
  const int parser::yyterror_ = 1;
  const int parser::yyerrcode_ = 256;
  const int parser::yyntokens_ = 65;

  const unsigned int parser::yyuser_token_number_max_ = 319;
  const parser::token_number_type parser::yyundef_token_ = 2;


/* Line 1054 of lalr1.cc  */
#line 1 "[Bison:b4_percent_define_default]"

} // yy

/* Line 1054 of lalr1.cc  */
#line 2041 "parser.tab.cc"


/* Line 1056 of lalr1.cc  */
#line 645 "parser.yy"

/* 
==========================================================================
Additional Code
==========================================================================
*/
void
yy::parser::error (const yy::parser::location_type& l,
                   const std::string& m)
{
    driver.error (m);  // Ignore location, since not maintained in lexer
}




