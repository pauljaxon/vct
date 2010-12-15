
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
#line 188 "parser.yy"
    {driver.result = (yysemantic_stack_[(1) - (1)].nval);}
    break;

  case 3:

/* Line 678 of lalr1.cc  */
#line 192 "parser.yy"
    { (yyval.nval) = (yysemantic_stack_[(1) - (1)].nval); }
    break;

  case 4:

/* Line 678 of lalr1.cc  */
#line 193 "parser.yy"
    { (yyval.nval) = (yysemantic_stack_[(1) - (1)].nval); }
    break;

  case 5:

/* Line 678 of lalr1.cc  */
#line 194 "parser.yy"
    { (yyval.nval) = (yysemantic_stack_[(1) - (1)].nval); }
    break;

  case 6:

/* Line 678 of lalr1.cc  */
#line 204 "parser.yy"
    {
	      // Drop recording of $2 and $3
              (yyval.nval) = (yysemantic_stack_[(7) - (5)].nval);
              (yyval.nval)->kind = z::FDL_FILE;
          }
    break;

  case 10:

/* Line 678 of lalr1.cc  */
#line 218 "parser.yy"
    { (yyval.nval) = new Node(z::SEQ); }
    break;

  case 11:

/* Line 678 of lalr1.cc  */
#line 219 "parser.yy"
    { (yyval.nval) = (yysemantic_stack_[(2) - (1)].nval);  (yyval.nval)->addChild((yysemantic_stack_[(2) - (2)].nval));}
    break;

  case 12:

/* Line 678 of lalr1.cc  */
#line 224 "parser.yy"
    { if ((yysemantic_stack_[(5) - (4)].nval)->kind == z::PENDING)
              (yyval.nval) = new Node(z::DEF_TYPE, * (yysemantic_stack_[(5) - (2)].sval));
          else
              (yyval.nval) = new Node(z::DEF_TYPE, * (yysemantic_stack_[(5) - (2)].sval), (yysemantic_stack_[(5) - (4)].nval));
          delete (yysemantic_stack_[(5) - (2)].sval);
        }
    break;

  case 13:

/* Line 678 of lalr1.cc  */
#line 231 "parser.yy"
    { if ((yysemantic_stack_[(7) - (6)].nval)->kind == z::PENDING)
              (yyval.nval) = new Node(z::DEF_CONST, * (yysemantic_stack_[(7) - (2)].sval), (yysemantic_stack_[(7) - (4)].nval));
          else
              (yyval.nval) = new Node(z::DEF_CONST, * (yysemantic_stack_[(7) - (2)].sval), (yysemantic_stack_[(7) - (4)].nval), (yysemantic_stack_[(7) - (6)].nval));
          delete (yysemantic_stack_[(7) - (2)].sval);
        }
    break;

  case 14:

/* Line 678 of lalr1.cc  */
#line 238 "parser.yy"
    { (yyval.nval) = new Node(z::DECL_VAR, * (yysemantic_stack_[(5) - (2)].sval), (yysemantic_stack_[(5) - (4)].nval));
          delete (yysemantic_stack_[(5) - (2)].sval);
        }
    break;

  case 15:

/* Line 678 of lalr1.cc  */
#line 243 "parser.yy"
    { (yyval.nval) = new Node(z::DECL_FUN,
                        * (yysemantic_stack_[(8) - (2)].sval), 
                        (yysemantic_stack_[(8) - (4)].nval),
                        (yysemantic_stack_[(8) - (7)].nval)); 
          delete (yysemantic_stack_[(8) - (2)].sval);
        }
    break;

  case 16:

/* Line 678 of lalr1.cc  */
#line 250 "parser.yy"
    { (yyval.nval) = new Node(z::DEF_CONST, * (yysemantic_stack_[(5) - (2)].sval), (yysemantic_stack_[(5) - (4)].nval));
          delete (yysemantic_stack_[(5) - (2)].sval);
        }
    break;

  case 17:

/* Line 678 of lalr1.cc  */
#line 256 "parser.yy"
    { (yyval.nval) = new Node(z::SEQ,(yysemantic_stack_[(1) - (1)].nval)); }
    break;

  case 18:

/* Line 678 of lalr1.cc  */
#line 257 "parser.yy"
    { (yyval.nval) = (yysemantic_stack_[(3) - (1)].nval);           
                      (yyval.nval)->addChild((yysemantic_stack_[(3) - (3)].nval)); 
                    }
    break;

  case 19:

/* Line 678 of lalr1.cc  */
#line 270 "parser.yy"
    { (yyval.nval) = (yysemantic_stack_[(1) - (1)].nval);
                               (yyval.nval)->kind = z::RLS_FILE;
                             }
    break;

  case 20:

/* Line 678 of lalr1.cc  */
#line 276 "parser.yy"
    { (yyval.nval) = new Node(z::SEQ,(yysemantic_stack_[(1) - (1)].nval)); }
    break;

  case 21:

/* Line 678 of lalr1.cc  */
#line 277 "parser.yy"
    { (yyval.nval) = (yysemantic_stack_[(2) - (1)].nval); (yyval.nval)->addChild((yysemantic_stack_[(2) - (2)].nval)); }
    break;

  case 22:

/* Line 678 of lalr1.cc  */
#line 282 "parser.yy"
    { (yyval.nval) = new Node(z::RULE_FAMILY, (yysemantic_stack_[(6) - (4)].nval), (yysemantic_stack_[(6) - (6)].nval)); }
    break;

  case 23:

/* Line 678 of lalr1.cc  */
#line 284 "parser.yy"
    { (yyval.nval) = new Node(z::RULE_FAMILY,
                                            new Node(z::SEQ),
                                            (yysemantic_stack_[(5) - (5)].nval));
                            }
    break;

  case 24:

/* Line 678 of lalr1.cc  */
#line 291 "parser.yy"
    { (yyval.nval) = new Node(z::SEQ,(yysemantic_stack_[(1) - (1)].nval)); }
    break;

  case 25:

/* Line 678 of lalr1.cc  */
#line 292 "parser.yy"
    { (yyval.nval) = (yysemantic_stack_[(3) - (1)].nval);  (yyval.nval)->addChild((yysemantic_stack_[(3) - (3)].nval)); }
    break;

  case 26:

/* Line 678 of lalr1.cc  */
#line 296 "parser.yy"
    { (yyval.nval) = new Node(z::SEQ); }
    break;

  case 27:

/* Line 678 of lalr1.cc  */
#line 300 "parser.yy"
    { (yyval.nval) = new Node(z::SEQ);  }
    break;

  case 28:

/* Line 678 of lalr1.cc  */
#line 301 "parser.yy"
    { (yyval.nval) = (yysemantic_stack_[(2) - (1)].nval);   (yyval.nval)->addChild((yysemantic_stack_[(2) - (2)].nval)); }
    break;

  case 29:

/* Line 678 of lalr1.cc  */
#line 306 "parser.yy"
    { (yyval.nval) = (yysemantic_stack_[(7) - (6)].nval); delete (yysemantic_stack_[(7) - (1)].sval); delete (yysemantic_stack_[(7) - (3)].sval);}
    break;

  case 30:

/* Line 678 of lalr1.cc  */
#line 310 "parser.yy"
    { (yyval.nval) = new Node(z::MAY_BE_REPLACED_BY, (yysemantic_stack_[(4) - (1)].nval), (yysemantic_stack_[(4) - (3)].nval));
                        if ((yysemantic_stack_[(4) - (4)].nval)->kind != z::TRUE) (yyval.nval)->addChild((yysemantic_stack_[(4) - (4)].nval));
                      }
    break;

  case 31:

/* Line 678 of lalr1.cc  */
#line 314 "parser.yy"
    { (yyval.nval) = new Node(z::MAY_BE_DEDUCED, (yysemantic_stack_[(2) - (1)].nval)); }
    break;

  case 32:

/* Line 678 of lalr1.cc  */
#line 316 "parser.yy"
    { (yyval.nval) = new Node(z::MAY_BE_DEDUCED, (yysemantic_stack_[(3) - (1)].nval), (yysemantic_stack_[(3) - (3)].nval)); }
    break;

  case 33:

/* Line 678 of lalr1.cc  */
#line 318 "parser.yy"
    { (yyval.nval) = new Node(z::ARE_INTERCHANGEABLE, (yysemantic_stack_[(5) - (1)].nval), (yysemantic_stack_[(5) - (3)].nval));
                        if ((yysemantic_stack_[(5) - (5)].nval)->kind != z::TRUE) (yyval.nval)->addChild((yysemantic_stack_[(5) - (5)].nval));
                      }
    break;

  case 34:

/* Line 678 of lalr1.cc  */
#line 324 "parser.yy"
    { (yyval.nval) = new Node(z::TRUE); }
    break;

  case 35:

/* Line 678 of lalr1.cc  */
#line 325 "parser.yy"
    { (yyval.nval) = (yysemantic_stack_[(2) - (2)].nval); }
    break;

  case 36:

/* Line 678 of lalr1.cc  */
#line 335 "parser.yy"
    { (yyval.nval) = (yysemantic_stack_[(1) - (1)].nval); (yyval.nval)->kind = z::VCG_FILE; }
    break;

  case 37:

/* Line 678 of lalr1.cc  */
#line 341 "parser.yy"
    { (yyval.nval) = (yysemantic_stack_[(1) - (1)].nval); }
    break;

  case 38:

/* Line 678 of lalr1.cc  */
#line 343 "parser.yy"
    { (yyval.nval) = (yysemantic_stack_[(2) - (1)].nval);
                        (yyval.nval)->appendChildren((yysemantic_stack_[(2) - (2)].nval));
                      }
    break;

  case 39:

/* Line 678 of lalr1.cc  */
#line 349 "parser.yy"
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
#line 360 "parser.yy"
    {(yyval.nval) = new Node(z::SEQ,(yysemantic_stack_[(1) - (1)].nval)); }
    break;

  case 41:

/* Line 678 of lalr1.cc  */
#line 361 "parser.yy"
    {(yyval.nval) = (yysemantic_stack_[(2) - (1)].nval); (yyval.nval)->addChild((yysemantic_stack_[(2) - (2)].nval)); }
    break;

  case 42:

/* Line 678 of lalr1.cc  */
#line 366 "parser.yy"
    {
             (yysemantic_stack_[(5) - (3)].nval)->kind = z::HYPS;
             (yysemantic_stack_[(5) - (5)].nval)->kind = z::CONCLS;
             (yyval.nval) = new Node(z::GOAL, * (yysemantic_stack_[(5) - (1)].sval), (yysemantic_stack_[(5) - (3)].nval), (yysemantic_stack_[(5) - (5)].nval));
             delete (yysemantic_stack_[(5) - (1)].sval);
            }
    break;

  case 43:

/* Line 678 of lalr1.cc  */
#line 373 "parser.yy"
    {
             (yyval.nval) = new Node (z::GOAL, * (yysemantic_stack_[(5) - (1)].sval)); 
             delete (yysemantic_stack_[(5) - (1)].sval);
            }
    break;

  case 44:

/* Line 678 of lalr1.cc  */
#line 380 "parser.yy"
    {(yyval.nval) = new Node(z::SEQ); }
    break;

  case 45:

/* Line 678 of lalr1.cc  */
#line 381 "parser.yy"
    {(yyval.nval) = (yysemantic_stack_[(2) - (1)].nval); (yyval.nval)->addChild((yysemantic_stack_[(2) - (2)].nval)); }
    break;

  case 46:

/* Line 678 of lalr1.cc  */
#line 384 "parser.yy"
    {(yyval.nval) = (yysemantic_stack_[(4) - (3)].nval); delete (yysemantic_stack_[(4) - (1)].sval);}
    break;

  case 47:

/* Line 678 of lalr1.cc  */
#line 388 "parser.yy"
    {(yyval.nval) = new Node(z::SEQ); }
    break;

  case 48:

/* Line 678 of lalr1.cc  */
#line 389 "parser.yy"
    {(yyval.nval) = (yysemantic_stack_[(2) - (1)].nval); (yyval.nval)->addChild((yysemantic_stack_[(2) - (2)].nval)); }
    break;

  case 49:

/* Line 678 of lalr1.cc  */
#line 392 "parser.yy"
    {(yyval.nval) = (yysemantic_stack_[(4) - (3)].nval); delete (yysemantic_stack_[(4) - (1)].sval);}
    break;

  case 50:

/* Line 678 of lalr1.cc  */
#line 405 "parser.yy"
    { (yyval.sval) = (yysemantic_stack_[(1) - (1)].sval);}
    break;

  case 51:

/* Line 678 of lalr1.cc  */
#line 406 "parser.yy"
    { (yyval.sval) = (yysemantic_stack_[(1) - (1)].sval);}
    break;

  case 52:

/* Line 678 of lalr1.cc  */
#line 407 "parser.yy"
    { (yyval.sval) = (yysemantic_stack_[(1) - (1)].sval);}
    break;

  case 53:

/* Line 678 of lalr1.cc  */
#line 408 "parser.yy"
    { (yyval.sval) = (yysemantic_stack_[(1) - (1)].sval);}
    break;

  case 54:

/* Line 678 of lalr1.cc  */
#line 409 "parser.yy"
    { (yyval.sval) = (yysemantic_stack_[(1) - (1)].sval);}
    break;

  case 55:

/* Line 678 of lalr1.cc  */
#line 413 "parser.yy"
    { (yyval.nval) = new Node(z::ID, * (yysemantic_stack_[(1) - (1)].sval)); }
    break;

  case 56:

/* Line 678 of lalr1.cc  */
#line 416 "parser.yy"
    { (yyval.nval) = new Node(z::SEQ,(yysemantic_stack_[(1) - (1)].nval)); }
    break;

  case 57:

/* Line 678 of lalr1.cc  */
#line 417 "parser.yy"
    { (yyval.nval) = (yysemantic_stack_[(3) - (1)].nval);  (yyval.nval)->addChild((yysemantic_stack_[(3) - (3)].nval)); }
    break;

  case 58:

/* Line 678 of lalr1.cc  */
#line 421 "parser.yy"
    { (yyval.nval) = new Node(z::SEQ,(yysemantic_stack_[(1) - (1)].nval)); }
    break;

  case 59:

/* Line 678 of lalr1.cc  */
#line 422 "parser.yy"
    { (yyval.nval) = (yysemantic_stack_[(3) - (1)].nval); (yyval.nval)->addChild((yysemantic_stack_[(3) - (3)].nval)); }
    break;

  case 60:

/* Line 678 of lalr1.cc  */
#line 426 "parser.yy"
    { (yyval.nval) = new Node(z::DECL, * (yysemantic_stack_[(3) - (1)].sval), (yysemantic_stack_[(3) - (3)].nval)); 
                          delete (yysemantic_stack_[(3) - (1)].sval);
                        }
    break;

  case 61:

/* Line 678 of lalr1.cc  */
#line 433 "parser.yy"
    { (yyval.nval) = new Node(z::SEQ);
     for (int i = 0; i != (yysemantic_stack_[(3) - (1)].nval)->arity(); i++) {
         (yyval.nval)->addChild(new Node(z::DECL, (yysemantic_stack_[(3) - (1)].nval)->child(i)->id, (yysemantic_stack_[(3) - (3)].nval)) );
     }
   }
    break;

  case 62:

/* Line 678 of lalr1.cc  */
#line 441 "parser.yy"
    { (yyval.nval) = (yysemantic_stack_[(3) - (2)].nval); }
    break;

  case 63:

/* Line 678 of lalr1.cc  */
#line 445 "parser.yy"
    { (yyval.nval) = new Node(z::SEQ, (yysemantic_stack_[(1) - (1)].nval)); }
    break;

  case 64:

/* Line 678 of lalr1.cc  */
#line 446 "parser.yy"
    { (yyval.nval) = (yysemantic_stack_[(3) - (1)].nval);           (yyval.nval)->addChild((yysemantic_stack_[(3) - (3)].nval)); }
    break;

  case 65:

/* Line 678 of lalr1.cc  */
#line 457 "parser.yy"
    { (yyval.nval) = (yysemantic_stack_[(1) - (1)].nval); }
    break;

  case 66:

/* Line 678 of lalr1.cc  */
#line 458 "parser.yy"
    { (yyval.nval) = (yysemantic_stack_[(3) - (2)].nval); (yyval.nval)->kind = z::ENUM_TY; }
    break;

  case 67:

/* Line 678 of lalr1.cc  */
#line 459 "parser.yy"
    { (yyval.nval) = new Node(z::ARRAY_TY,
                                               (yysemantic_stack_[(6) - (3)].nval),
                                               (yysemantic_stack_[(6) - (6)].nval));
                               }
    break;

  case 68:

/* Line 678 of lalr1.cc  */
#line 463 "parser.yy"
    { (yyval.nval) = (yysemantic_stack_[(3) - (2)].nval); (yyval.nval)->kind = z::RECORD_TY; }
    break;

  case 69:

/* Line 678 of lalr1.cc  */
#line 467 "parser.yy"
    { (yyval.nval) = (yysemantic_stack_[(1) - (1)].nval); }
    break;

  case 70:

/* Line 678 of lalr1.cc  */
#line 469 "parser.yy"
    { (yyval.nval) = (yysemantic_stack_[(3) - (1)].nval);     
            (yyval.nval)->appendChildren((yysemantic_stack_[(3) - (3)].nval)); 
          }
    break;

  case 71:

/* Line 678 of lalr1.cc  */
#line 475 "parser.yy"
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

  case 72:

/* Line 678 of lalr1.cc  */
#line 495 "parser.yy"
    { (yyval.nval) = new Node(z::FORALL, (yysemantic_stack_[(6) - (3)].nval), (yysemantic_stack_[(6) - (5)].nval)); }
    break;

  case 73:

/* Line 678 of lalr1.cc  */
#line 497 "parser.yy"
    { (yyval.nval) = new Node(z::EXISTS, (yysemantic_stack_[(6) - (3)].nval), (yysemantic_stack_[(6) - (5)].nval)); }
    break;

  case 74:

/* Line 678 of lalr1.cc  */
#line 498 "parser.yy"
    {(yyval.nval) = new Node(z::IMPLIES, (yysemantic_stack_[(3) - (1)].nval), (yysemantic_stack_[(3) - (3)].nval)); }
    break;

  case 75:

/* Line 678 of lalr1.cc  */
#line 499 "parser.yy"
    {(yyval.nval) = new Node(z::IFF, (yysemantic_stack_[(3) - (1)].nval), (yysemantic_stack_[(3) - (3)].nval)); }
    break;

  case 76:

/* Line 678 of lalr1.cc  */
#line 500 "parser.yy"
    {(yyval.nval) = new Node(z::AND, (yysemantic_stack_[(3) - (1)].nval), (yysemantic_stack_[(3) - (3)].nval)); }
    break;

  case 77:

/* Line 678 of lalr1.cc  */
#line 501 "parser.yy"
    {(yyval.nval) = new Node(z::OR, (yysemantic_stack_[(3) - (1)].nval), (yysemantic_stack_[(3) - (3)].nval)); }
    break;

  case 78:

/* Line 678 of lalr1.cc  */
#line 502 "parser.yy"
    {(yyval.nval) = new Node(z::NOT, (yysemantic_stack_[(2) - (2)].nval)); }
    break;

  case 79:

/* Line 678 of lalr1.cc  */
#line 503 "parser.yy"
    {(yyval.nval) = new Node(z::EQ, (yysemantic_stack_[(3) - (1)].nval), (yysemantic_stack_[(3) - (3)].nval)); }
    break;

  case 80:

/* Line 678 of lalr1.cc  */
#line 504 "parser.yy"
    {(yyval.nval) = new Node(z::NOT, new Node(z::EQ, (yysemantic_stack_[(3) - (1)].nval), (yysemantic_stack_[(3) - (3)].nval))); }
    break;

  case 81:

/* Line 678 of lalr1.cc  */
#line 505 "parser.yy"
    {(yyval.nval) = new Node(z::LT, (yysemantic_stack_[(3) - (1)].nval), (yysemantic_stack_[(3) - (3)].nval)); }
    break;

  case 82:

/* Line 678 of lalr1.cc  */
#line 506 "parser.yy"
    {(yyval.nval) = new Node(z::GT, (yysemantic_stack_[(3) - (1)].nval), (yysemantic_stack_[(3) - (3)].nval)); }
    break;

  case 83:

/* Line 678 of lalr1.cc  */
#line 507 "parser.yy"
    {(yyval.nval) = new Node(z::LE, (yysemantic_stack_[(3) - (1)].nval), (yysemantic_stack_[(3) - (3)].nval)); }
    break;

  case 84:

/* Line 678 of lalr1.cc  */
#line 508 "parser.yy"
    {(yyval.nval) = new Node(z::GE, (yysemantic_stack_[(3) - (1)].nval), (yysemantic_stack_[(3) - (3)].nval)); }
    break;

  case 85:

/* Line 678 of lalr1.cc  */
#line 510 "parser.yy"
    {(yyval.nval) = (yysemantic_stack_[(2) - (2)].nval); }
    break;

  case 86:

/* Line 678 of lalr1.cc  */
#line 512 "parser.yy"
    {(yyval.nval) = new Node(z::UMINUS, (yysemantic_stack_[(2) - (2)].nval)); }
    break;

  case 87:

/* Line 678 of lalr1.cc  */
#line 513 "parser.yy"
    {(yyval.nval) = new Node(z::PLUS, (yysemantic_stack_[(3) - (1)].nval), (yysemantic_stack_[(3) - (3)].nval)); }
    break;

  case 88:

/* Line 678 of lalr1.cc  */
#line 514 "parser.yy"
    {(yyval.nval) = new Node(z::MINUS, (yysemantic_stack_[(3) - (1)].nval), (yysemantic_stack_[(3) - (3)].nval)); }
    break;

  case 89:

/* Line 678 of lalr1.cc  */
#line 515 "parser.yy"
    {(yyval.nval) = new Node(z::TIMES, (yysemantic_stack_[(3) - (1)].nval), (yysemantic_stack_[(3) - (3)].nval)); }
    break;

  case 90:

/* Line 678 of lalr1.cc  */
#line 516 "parser.yy"
    {(yyval.nval) = new Node(z::RDIV, (yysemantic_stack_[(3) - (1)].nval), (yysemantic_stack_[(3) - (3)].nval)); }
    break;

  case 91:

/* Line 678 of lalr1.cc  */
#line 517 "parser.yy"
    {(yyval.nval) = new Node(z::MOD, (yysemantic_stack_[(3) - (1)].nval), (yysemantic_stack_[(3) - (3)].nval)); }
    break;

  case 92:

/* Line 678 of lalr1.cc  */
#line 518 "parser.yy"
    {(yyval.nval) = new Node(z::IDIV, (yysemantic_stack_[(3) - (1)].nval), (yysemantic_stack_[(3) - (3)].nval)); }
    break;

  case 93:

/* Line 678 of lalr1.cc  */
#line 519 "parser.yy"
    {(yyval.nval) = new Node(z::EXP, (yysemantic_stack_[(3) - (1)].nval), (yysemantic_stack_[(3) - (3)].nval)); }
    break;

  case 94:

/* Line 678 of lalr1.cc  */
#line 520 "parser.yy"
    {(yyval.nval) = new Node(z::SUBRANGE, (yysemantic_stack_[(3) - (1)].nval), (yysemantic_stack_[(3) - (3)].nval)); }
    break;

  case 95:

/* Line 678 of lalr1.cc  */
#line 521 "parser.yy"
    {(yyval.nval) = (yysemantic_stack_[(3) - (2)].nval); }
    break;

  case 96:

/* Line 678 of lalr1.cc  */
#line 522 "parser.yy"
    {(yyval.nval) = new Node(z::NATNUM, * (yysemantic_stack_[(1) - (1)].sval)); delete (yysemantic_stack_[(1) - (1)].sval); }
    break;

  case 97:

/* Line 678 of lalr1.cc  */
#line 523 "parser.yy"
    {(yyval.nval) = (yysemantic_stack_[(1) - (1)].nval);}
    break;

  case 98:

/* Line 678 of lalr1.cc  */
#line 524 "parser.yy"
    {(yyval.nval) = (yysemantic_stack_[(1) - (1)].nval);}
    break;

  case 99:

/* Line 678 of lalr1.cc  */
#line 526 "parser.yy"
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

  case 100:

/* Line 678 of lalr1.cc  */
#line 598 "parser.yy"
    { (yyval.nval) = new Node(z::SEQ,(yysemantic_stack_[(1) - (1)].nval)); }
    break;

  case 101:

/* Line 678 of lalr1.cc  */
#line 599 "parser.yy"
    { (yyval.nval) = (yysemantic_stack_[(3) - (1)].nval);           (yyval.nval)->addChild((yysemantic_stack_[(3) - (3)].nval)); }
    break;

  case 102:

/* Line 678 of lalr1.cc  */
#line 603 "parser.yy"
    {(yyval.nval) = (yysemantic_stack_[(1) - (1)].nval); }
    break;

  case 103:

/* Line 678 of lalr1.cc  */
#line 605 "parser.yy"
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

  case 104:

/* Line 678 of lalr1.cc  */
#line 617 "parser.yy"
    {(yyval.nval) = new Node(z::INDEX_AND,(yysemantic_stack_[(1) - (1)].nval)); }
    break;

  case 105:

/* Line 678 of lalr1.cc  */
#line 619 "parser.yy"
    {
            (yyval.nval) = (yysemantic_stack_[(3) - (1)].nval);
            (yyval.nval)->addChild((yysemantic_stack_[(3) - (3)].nval));
         }
    break;

  case 106:

/* Line 678 of lalr1.cc  */
#line 625 "parser.yy"
    { const string& idstr = (yysemantic_stack_[(1) - (1)].nval)->id;
           if      (idstr == "pending") (yyval.nval) = new Node(z::PENDING);
           else if (idstr == "true")    (yyval.nval) = new Node(z::TRUE);
           else if (idstr == "false")   (yyval.nval) = new Node(z::FALSE);
           else                         (yyval.nval) = (yysemantic_stack_[(1) - (1)].nval);
         }
    break;



/* Line 678 of lalr1.cc  */
#line 1294 "parser.tab.cc"
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
  const short int parser::yypact_ninf_ = -197;
  const short int
  parser::yypact_[] =
  {
       122,    78,     9,    -4,    18,  -197,  -197,  -197,    35,  -197,
    -197,    41,  -197,  -197,  -197,  -197,  -197,  -197,    62,  -197,
    -197,  -197,    78,    68,  -197,  -197,  -197,   391,  -197,    82,
      76,   404,   404,  -197,   108,   113,  -197,   404,   404,   404,
      95,  -197,   123,  -197,  -197,   485,  -197,  -197,   117,    76,
    -197,    28,   548,   185,    78,    78,    78,   222,    73,    73,
     404,  -197,   404,   404,   127,   404,   404,   404,   404,   404,
     404,   404,   404,   404,   404,   404,   404,   404,   404,   404,
     404,   404,    44,    96,  -197,  -197,   404,  -197,  -197,   126,
    -197,    29,   132,   136,  -197,    78,   319,    40,  -197,    -8,
     573,    78,   460,   460,   -42,   222,   288,   288,   288,   288,
     288,   288,    48,    48,    73,    73,    73,    73,  -197,   124,
      78,    78,    78,    78,  -197,    78,     2,   548,   110,    -9,
      78,   404,   404,  -197,   404,   404,   404,   145,    47,  -197,
    -197,    51,   100,   147,   148,   141,   150,  -197,  -197,   158,
      78,   151,    78,  -197,  -197,  -197,  -197,   204,   253,  -197,
     548,   548,    -9,  -197,    78,    -9,    -9,    -9,    -9,    -9,
    -197,   404,   134,   164,    90,    -9,  -197,    21,  -197,  -197,
    -197,  -197,   144,   119,  -197,   159,   166,   128,   420,   176,
    -197,   404,  -197,    86,    78,  -197,  -197,   177,    -9,  -197,
    -197,   404,  -197,   404,   171,   272,   153,  -197,    -9,  -197,
     339,   439,  -197,   404,   404,  -197,   180,    -9,   175,  -197,
    -197,   504,   529,  -197,  -197,  -197,   168,   180,  -197,  -197,
    -197
  };

  /* YYDEFACT[S] -- default rule to reduce with in state S when YYTABLE
     doesn't specify something else to do.  Zero means the default is an
     error.  */
  const unsigned char
  parser::yydefact_[] =
  {
         0,     0,     0,     0,     0,     2,     3,     4,    19,    20,
       5,    36,    37,    53,    52,    51,    54,    50,     0,     8,
       9,     7,     0,     0,     1,    21,    38,     0,    55,     0,
       0,     0,     0,    27,     0,     0,    96,     0,     0,     0,
       0,    24,    55,   106,    97,     0,    98,    10,     0,    39,
      40,     0,    63,     0,    23,     0,     0,    78,    86,    85,
       0,    27,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    44,    41,    62,     0,    95,    28,     0,
      56,     0,     0,     0,    25,    22,   102,     0,   100,     0,
      94,     0,    74,    75,    77,    76,    83,    81,    84,    82,
      80,    79,    88,    87,    91,    92,    90,    89,    93,     0,
       0,     0,     0,     0,    11,     0,     0,    64,     0,     0,
       0,     0,     0,    99,     0,     0,     0,     0,     0,    58,
       6,     0,     0,     0,     0,     0,     0,    47,    45,     0,
       0,     0,     0,    71,    61,    65,    57,     0,     0,   101,
     105,   103,     0,    26,     0,     0,     0,     0,     0,     0,
      43,     0,    42,     0,     0,     0,    69,     0,    73,    72,
      60,    59,     0,     0,    17,     0,     0,     0,     0,     0,
      48,     0,    66,     0,     0,    68,    16,     0,     0,    12,
      14,     0,    46,     0,     0,     0,     0,    70,     0,    18,
       0,     0,    29,     0,     0,    31,     0,     0,     0,    13,
      49,     0,    34,    32,    67,    15,    34,     0,    30,    33,
      35
  };

  /* YYPGOTO[NTERM-NUM].  */
  const short int
  parser::yypgoto_[] =
  {
      -197,  -197,  -197,  -197,  -197,  -197,  -197,    16,  -197,  -197,
     184,  -197,   135,   133,  -197,  -197,   -28,  -197,  -197,   188,
    -197,   154,  -197,  -197,  -197,  -197,    -1,    10,    50,  -197,
      42,   -53,  -196,  -197,  -105,  -197,  -197,    11,  -197,    71,
    -197,  -197
  };

  /* YYDEFGOTO[NTERM-NUM].  */
  const short int
  parser::yydefgoto_[] =
  {
        -1,     4,     5,     6,    22,    82,   124,   183,     7,     8,
       9,    40,    41,    54,    88,   204,   228,    10,    11,    12,
      49,    50,   126,   148,   172,   190,    42,    43,    91,   138,
     139,    92,    44,    51,   184,   177,   155,    45,    97,    98,
      99,    46
  };

  /* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule which
     number is the opposite.  If zero, do what YYDEFACT says.  */
  const signed char parser::yytable_ninf_ = -105;
  const short int
  parser::yytable_[] =
  {
        18,   150,    23,    93,    68,   135,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    24,    81,
     223,    28,   151,   152,   154,   136,    13,    14,    15,    16,
      17,   230,    29,    19,    20,   194,   129,    85,     1,   146,
      86,   130,    52,    53,   195,   147,     3,    21,    57,    58,
      59,   133,   134,    89,    28,    28,   163,   180,   165,   164,
     182,   166,   185,   186,   187,    90,    90,   119,   120,    27,
     121,   122,   123,    96,   100,    30,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   209,    89,   206,    47,   127,   198,   176,
     137,   192,   130,   218,    77,    78,    79,    80,    60,    81,
      61,    48,   224,    13,    14,    15,    16,    17,    55,   141,
     142,   143,   144,    56,   145,     1,     2,     3,    28,    28,
     197,   198,    83,    62,    81,   101,   128,   125,   140,   153,
     156,   207,   157,   158,   131,    96,   160,   161,   132,    28,
     149,    28,   162,   167,   168,   169,   170,   171,   196,   175,
      90,    28,    90,   137,    28,    28,    28,    28,    28,   173,
     189,   191,   153,   199,    28,   153,   153,   153,   153,   153,
     200,   201,   188,   203,   208,   153,   212,   217,    31,   225,
     227,   193,    25,    28,    95,    94,    87,    28,   229,    26,
     174,    63,   205,    84,    90,   159,   181,    28,   153,     0,
       0,     0,   210,     0,   211,   178,    28,     0,   153,     0,
      63,     0,     0,     0,   221,   222,     0,   153,    65,    66,
      67,    68,     0,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,     0,    81,    65,    66,    67,
      68,     0,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,   179,    81,     0,     0,     0,    63,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,     0,    81,     0,   213,     0,     0,    63,     0,
     214,   215,   216,     0,     0,     0,    65,    66,    67,    68,
       0,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,     0,    81,    65,    66,    67,    68,     0,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,  -104,    81,     0,    63,  -105,  -105,  -105,  -105,
    -105,  -105,    75,    76,    77,    78,    79,    80,     0,    81,
       0,     0,  -104,   219,     0,    63,     0,     0,     0,     0,
       0,     0,    65,    66,    67,    68,     0,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,     0,
      81,     0,    65,    66,    67,    68,     0,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    31,
      81,    32,     0,     0,     0,     0,    33,     0,     0,     0,
       0,     0,    31,     0,    32,     0,     0,     0,     0,     0,
      34,    35,     0,     0,     0,     0,    13,    14,    15,    16,
      17,    36,     0,    34,    35,   202,    63,     0,    37,    13,
      14,    15,    16,    17,    36,    38,    39,     0,     0,     0,
       0,    37,     0,     0,   220,    63,     0,     0,    38,    39,
       0,     0,     0,    65,    66,    67,    68,     0,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
       0,    81,    65,    66,    67,    68,     0,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,     0,
      81,    63,    64,  -105,  -105,    67,    68,     0,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      63,    81,     0,     0,     0,   226,     0,     0,    65,    66,
      67,    68,     0,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    63,    81,    65,    66,    67,
      68,   227,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    63,    81,     0,     0,     0,     0,
       0,     0,    65,    66,    67,    68,     0,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,  -105,
      81,    65,    66,    67,    68,     0,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,     0,    81,
       0,     0,     0,     0,     0,     0,    65,    66,    67,    68,
       0,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,     0,    81
  };

  /* YYCHECK.  */
  const short int
  parser::yycheck_[] =
  {
         1,    10,     6,    56,    46,    13,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,     0,    61,
     216,    22,    31,    32,   129,    33,    35,    36,    37,    38,
      39,   227,    22,    24,    25,    14,     7,     9,     3,    37,
      12,    12,    31,    32,    23,    43,     5,    38,    37,    38,
      39,    11,    12,    54,    55,    56,     9,   162,     7,    12,
     165,    10,   167,   168,   169,    55,    56,    23,    24,     7,
      26,    27,    28,    62,    63,     7,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,   198,    95,     9,    14,    86,    12,   152,
     101,    11,    12,   208,    56,    57,    58,    59,    13,    61,
      15,    35,   217,    35,    36,    37,    38,    39,    10,   120,
     121,   122,   123,    10,   125,     3,     4,     5,   129,   130,
      11,    12,    15,    10,    61,     8,    10,    41,    14,   129,
     130,   194,   131,   132,    12,   134,   135,   136,    12,   150,
      40,   152,     7,    53,     7,     7,    15,     7,    14,     8,
     150,   162,   152,   164,   165,   166,   167,   168,   169,    11,
      36,     7,   162,    14,   175,   165,   166,   167,   168,   169,
      14,    53,   171,     7,     7,   175,    15,    34,     8,    14,
      22,   175,     8,   194,    61,    60,    11,   198,   226,    11,
     150,    16,   191,    49,   194,   134,   164,   208,   198,    -1,
      -1,    -1,   201,    -1,   203,    11,   217,    -1,   208,    -1,
      16,    -1,    -1,    -1,   213,   214,    -1,   217,    43,    44,
      45,    46,    -1,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    -1,    61,    43,    44,    45,
      46,    -1,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    11,    61,    -1,    -1,    -1,    16,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    -1,    61,    -1,    13,    -1,    -1,    16,    -1,
      18,    19,    20,    -1,    -1,    -1,    43,    44,    45,    46,
      -1,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    -1,    61,    43,    44,    45,    46,    -1,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    13,    61,    -1,    16,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    -1,    61,
      -1,    -1,    33,    14,    -1,    16,    -1,    -1,    -1,    -1,
      -1,    -1,    43,    44,    45,    46,    -1,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    -1,
      61,    -1,    43,    44,    45,    46,    -1,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,     8,
      61,    10,    -1,    -1,    -1,    -1,    15,    -1,    -1,    -1,
      -1,    -1,     8,    -1,    10,    -1,    -1,    -1,    -1,    -1,
      29,    30,    -1,    -1,    -1,    -1,    35,    36,    37,    38,
      39,    40,    -1,    29,    30,    15,    16,    -1,    47,    35,
      36,    37,    38,    39,    40,    54,    55,    -1,    -1,    -1,
      -1,    47,    -1,    -1,    15,    16,    -1,    -1,    54,    55,
      -1,    -1,    -1,    43,    44,    45,    46,    -1,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      -1,    61,    43,    44,    45,    46,    -1,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    -1,
      61,    16,    17,    43,    44,    45,    46,    -1,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      16,    61,    -1,    -1,    -1,    21,    -1,    -1,    43,    44,
      45,    46,    -1,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    16,    61,    43,    44,    45,
      46,    22,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    16,    61,    -1,    -1,    -1,    -1,
      -1,    -1,    43,    44,    45,    46,    -1,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    16,
      61,    43,    44,    45,    46,    -1,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    -1,    61,
      -1,    -1,    -1,    -1,    -1,    -1,    43,    44,    45,    46,
      -1,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    -1,    61
  };

  /* STOS_[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
  const unsigned char
  parser::yystos_[] =
  {
         0,     3,     4,     5,    63,    64,    65,    70,    71,    72,
      79,    80,    81,    35,    36,    37,    38,    39,    88,    24,
      25,    38,    66,     6,     0,    72,    81,     7,    88,    89,
       7,     8,    10,    15,    29,    30,    40,    47,    54,    55,
      73,    74,    88,    89,    94,    99,   103,    14,    35,    82,
      83,    95,    99,    99,    75,    10,    10,    99,    99,    99,
      13,    15,    10,    16,    17,    43,    44,    45,    46,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    61,    67,    15,    83,     9,    12,    11,    76,    88,
      89,    90,    93,    93,    74,    75,    99,   100,   101,   102,
      99,     8,    99,    99,    99,    99,    99,    99,    99,    99,
      99,    99,    99,    99,    99,    99,    99,    99,    99,    23,
      24,    26,    27,    28,    68,    41,    84,    99,    10,     7,
      12,    12,    12,    11,    12,    13,    33,    88,    91,    92,
      14,    88,    88,    88,    88,    88,    37,    43,    85,    40,
      10,    31,    32,    89,    96,    98,    89,    99,    99,   101,
      99,    99,     7,     9,    12,     7,    10,    53,     7,     7,
      15,     7,    86,    11,    90,     8,    93,    97,    11,    11,
      96,    92,    96,    69,    96,    96,    96,    96,    99,    36,
      87,     7,    11,    69,    14,    23,    14,    11,    12,    14,
      14,    53,    15,     7,    77,    99,     9,    93,     7,    96,
      99,    99,    15,    13,    18,    19,    20,    34,    96,    14,
      15,    99,    99,    94,    96,    14,    21,    22,    78,    78,
      94
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
     315,   316
  };
#endif

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
  const unsigned char
  parser::yyr1_[] =
  {
         0,    62,    63,    64,    64,    64,    65,    66,    66,    66,
      67,    67,    68,    68,    68,    68,    68,    69,    69,    70,
      71,    71,    72,    72,    73,    73,    74,    75,    75,    76,
      77,    77,    77,    77,    78,    78,    79,    80,    80,    81,
      82,    82,    83,    83,    84,    84,    85,    86,    86,    87,
      88,    88,    88,    88,    88,    89,    90,    90,    91,    91,
      92,    93,    94,    95,    95,    96,    96,    96,    96,    97,
      97,    98,    99,    99,    99,    99,    99,    99,    99,    99,
      99,    99,    99,    99,    99,    99,    99,    99,    99,    99,
      99,    99,    99,    99,    99,    99,    99,    99,    99,    99,
     100,   100,   101,   101,   102,   102,   103
  };

  /* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
  const unsigned char
  parser::yyr2_[] =
  {
         0,     2,     1,     1,     1,     1,     7,     1,     1,     1,
       0,     2,     5,     7,     5,     8,     5,     1,     3,     1,
       1,     2,     6,     5,     1,     3,     5,     0,     2,     7,
       4,     2,     3,     5,     0,     2,     1,     1,     2,     4,
       1,     2,     5,     5,     0,     2,     4,     0,     2,     4,
       1,     1,     1,     1,     1,     1,     1,     3,     1,     3,
       3,     3,     3,     1,     3,     1,     3,     6,     3,     1,
       3,     1,     6,     6,     3,     3,     3,     3,     2,     3,
       3,     3,     3,     3,     3,     2,     2,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     1,     1,     1,     4,
       1,     3,     1,     3,     1,     3,     1
  };

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
  /* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
     First, the terminals, then, starting at \a yyntokens_, nonterminals.  */
  const char*
  const parser::yytname_[] =
  {
    "FILE_END", "error", "$undefined", "RULE_FAMILY", "TITLE", "FOR",
  "GOAL_ORIGINS", "COLON", "LSB", "RSB", "LPAREN", "RPAREN", "COMMA",
  "AMPERSAND", "SEMIC", "DOT", "DOTDOT", "REQUIRES", "MAY_BE_REPLACED_BY",
  "MAY_BE_DEDUCED", "MAY_BE_DEDUCED_FROM", "ARE_INTERCHANGEABLE", "IF",
  "END", "FUNCTION", "PROCEDURE", "TYPE", "VAR", "CONST", "FOR_SOME",
  "FOR_ALL", "ARRAY", "RECORD", "ASSIGN", "OF", "SUBPROG_ID", "CONCL_ID",
  "HYP_ID", "TASK_TYPE", "ID", "NATNUM", "TRIPLESTAR", "TRIPLEBANG",
  "IMPLIES", "IFF", "OR", "AND", "NOT", "LE", "LT", "GE", "GT", "NE", "EQ",
  "MINUS", "PLUS", "MOD", "DIV", "SLASH", "STAR", "UMINUS", "STARSTAR",
  "$accept", "top", "file", "fdl_file", "program_kind", "fdl_decls",
  "fdl_decl", "types", "rls_file", "rule_families", "rule_family",
  "typeassums", "typeassum", "rules", "rule", "rule_body",
  "rule_condition", "vcg_file", "goalsets", "goalset", "goals", "goal",
  "hyps", "hyp", "concls", "concl", "id_str", "id", "ids", "typedecls",
  "typedecl", "multidecl", "expseq", "expseq1", "type", "recordtypedecls",
  "type_id", "exp", "aexpseq", "aexp", "indexset", "exp_id", 0
  };
#endif

#if YYDEBUG
  /* YYRHS -- A `-1'-separated list of the rules' RHS.  */
  const parser::rhs_number_type
  parser::yyrhs_[] =
  {
        63,     0,    -1,    64,    -1,    65,    -1,    70,    -1,    79,
      -1,     4,    66,    89,    14,    67,    23,    14,    -1,    38,
      -1,    24,    -1,    25,    -1,    -1,    67,    68,    -1,    26,
      88,    53,    96,    14,    -1,    28,    88,     7,    96,    53,
      99,    14,    -1,    27,    88,     7,    96,    14,    -1,    24,
      88,    10,    69,    11,     7,    96,    14,    -1,    24,    88,
       7,    96,    14,    -1,    96,    -1,    69,    12,    96,    -1,
      71,    -1,    72,    -1,    71,    72,    -1,     3,    88,     7,
      73,    15,    75,    -1,     3,    88,     7,    15,    75,    -1,
      74,    -1,    73,    13,    74,    -1,    99,    17,     8,    91,
       9,    -1,    -1,    75,    76,    -1,    88,    10,    40,    11,
       7,    77,    15,    -1,    99,    18,    99,    78,    -1,    99,
      19,    -1,    99,    20,    94,    -1,    99,    13,    99,    21,
      78,    -1,    -1,    22,    94,    -1,    80,    -1,    81,    -1,
      80,    81,    -1,     5,     6,     7,    82,    -1,    83,    -1,
      82,    83,    -1,    35,    15,    84,    43,    86,    -1,    35,
      15,    41,    88,    15,    -1,    -1,    84,    85,    -1,    37,
       7,    99,    15,    -1,    -1,    86,    87,    -1,    36,     7,
      99,    15,    -1,    39,    -1,    37,    -1,    36,    -1,    35,
      -1,    38,    -1,    88,    -1,    89,    -1,    90,    12,    89,
      -1,    92,    -1,    91,    12,    92,    -1,    88,     7,    96,
      -1,    90,     7,    96,    -1,     8,    95,     9,    -1,    99,
      -1,    95,    12,    99,    -1,    98,    -1,    10,    90,    11,
      -1,    31,     8,    69,     9,    34,    96,    -1,    32,    97,
      23,    -1,    93,    -1,    97,    14,    93,    -1,    89,    -1,
      30,    10,    93,    12,    99,    11,    -1,    29,    10,    93,
      12,    99,    11,    -1,    99,    43,    99,    -1,    99,    44,
      99,    -1,    99,    46,    99,    -1,    99,    45,    99,    -1,
      47,    99,    -1,    99,    53,    99,    -1,    99,    52,    99,
      -1,    99,    49,    99,    -1,    99,    51,    99,    -1,    99,
      48,    99,    -1,    99,    50,    99,    -1,    55,    99,    -1,
      54,    99,    -1,    99,    55,    99,    -1,    99,    54,    99,
      -1,    99,    59,    99,    -1,    99,    58,    99,    -1,    99,
      56,    99,    -1,    99,    57,    99,    -1,    99,    61,    99,
      -1,    99,    16,    99,    -1,    10,    99,    11,    -1,    40,
      -1,    94,    -1,   103,    -1,    88,    10,   100,    11,    -1,
     101,    -1,   100,    12,   101,    -1,    99,    -1,   102,    33,
      99,    -1,    99,    -1,   102,    13,    99,    -1,    89,    -1
  };

  /* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
     YYRHS.  */
  const unsigned short int
  parser::yyprhs_[] =
  {
         0,     0,     3,     5,     7,     9,    11,    19,    21,    23,
      25,    26,    29,    35,    43,    49,    58,    64,    66,    70,
      72,    74,    77,    84,    90,    92,    96,   102,   103,   106,
     114,   119,   122,   126,   132,   133,   136,   138,   140,   143,
     148,   150,   153,   159,   165,   166,   169,   174,   175,   178,
     183,   185,   187,   189,   191,   193,   195,   197,   201,   203,
     207,   211,   215,   219,   221,   225,   227,   231,   238,   242,
     244,   248,   250,   257,   264,   268,   272,   276,   280,   283,
     287,   291,   295,   299,   303,   307,   310,   313,   317,   321,
     325,   329,   333,   337,   341,   345,   349,   351,   353,   355,
     360,   362,   366,   368,   372,   374,   378
  };

  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
  const unsigned short int
  parser::yyrline_[] =
  {
         0,   188,   188,   192,   193,   194,   203,   212,   213,   214,
     218,   219,   223,   230,   237,   242,   249,   256,   257,   270,
     276,   277,   281,   283,   291,   292,   296,   300,   301,   305,
     309,   313,   315,   317,   324,   325,   335,   341,   343,   348,
     360,   361,   365,   372,   380,   381,   384,   388,   389,   392,
     405,   406,   407,   408,   409,   413,   416,   417,   421,   422,
     426,   432,   441,   445,   446,   457,   458,   459,   463,   467,
     468,   475,   494,   496,   498,   499,   500,   501,   502,   503,
     504,   505,   506,   507,   508,   509,   511,   513,   514,   515,
     516,   517,   518,   519,   520,   521,   522,   523,   524,   525,
     598,   599,   603,   604,   617,   618,   625
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
      55,    56,    57,    58,    59,    60,    61
    };
    if ((unsigned int) t <= yyuser_token_number_max_)
      return translate_table[t];
    else
      return yyundef_token_;
  }

  const int parser::yyeof_ = 0;
  const int parser::yylast_ = 634;
  const int parser::yynnts_ = 42;
  const int parser::yyempty_ = -2;
  const int parser::yyfinal_ = 24;
  const int parser::yyterror_ = 1;
  const int parser::yyerrcode_ = 256;
  const int parser::yyntokens_ = 62;

  const unsigned int parser::yyuser_token_number_max_ = 316;
  const parser::token_number_type parser::yyundef_token_ = 2;


/* Line 1054 of lalr1.cc  */
#line 1 "[Bison:b4_percent_define_default]"

} // yy

/* Line 1054 of lalr1.cc  */
#line 2007 "parser.tab.cc"


/* Line 1056 of lalr1.cc  */
#line 634 "parser.yy"

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




