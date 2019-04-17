#include <iostream>
#include <memory>

#include "Parser.hpp"

Parser::Parser(Lexer &lex):
    lexer{lex}
{}
 
void Parser::die(std::string where, std::string message, std::shared_ptr<Token> token) {
    std::cout << where << " " << message << std::endl;
    token->print();
    std::cout << std::endl;
    std::cout << "\nThe following is a list of tokens that have been identified up to this point.\n";
    lexer.printProcessedTokens();
    exit(1);
} 

std::unique_ptr<GroupedStatements> Parser::file_input() {

    std::string scope = "Parser::file_input()";

    if (debug)
        std::cout << scope << std::endl;

    auto tok = lexer.getToken();

    std::unique_ptr<GroupedStatements> gStmts = std::make_unique<GroupedStatements>();

    while ( tok->isPrint() || tok->isName() || tok->isFor() || tok->isIf() ) {

        lexer.ungetToken();

        std::unique_ptr<Statements> statements = stmt();
        gStmts->addStatements(std::move(statements));

        tok = lexer.getToken();

    }
    return gStmts;
}

// stmt -> simple_stmt | compound_stmt
std::unique_ptr<Statements> Parser::stmt() {

    std::string scope = "Parser::stmt()";

    if (debug)
        std::cout << scope << std::endl;

    auto tok = lexer.getToken();

    if (tok->isPrint() || tok->isName()) {

        lexer.ungetToken();
        std::unique_ptr<Statements> simpleStatement = simple_stmt();
        return simpleStatement;

    } else if (tok->isIf() || tok->isFor()) {

        lexer.ungetToken();
        std::unique_ptr<Statements> compoundStatement = compound_stmt();
        return compoundStatement;

    } else {

        std::string err = "Expected simple_stmt<print|assign> | compound_stmt<if|for> instead got,";
        die(scope, err, tok);
    }

    return nullptr;
}

std::unique_ptr<Statements> Parser::simple_stmt() {
    // Parse grammar rule 
    // <simple_stmt> -> { print_stmt | assign_stmt }+ NEWLINE

    std::string scope = "Parser::simple_stmt";
    if (debug)
        std::cout << scope << std::endl;
    
    auto stmts = std::make_unique<Statements>();
    auto tok = lexer.getToken();

    if ( !( tok->isPrint() || tok->isName()) ) 
        die(scope, "Violating + rule, no statements", tok);

    while ( tok->isPrint() || tok->isName() ) {

        lexer.ungetToken();

        if ( tok->isName() ) {
            std::unique_ptr<AssignStmt> assignStmt = assign_stmt();
            stmts->addStatement(std::move(assignStmt));

        } else if ( tok->isPrint() ) {
            std::unique_ptr<PrintStatement> printStmt = print_stmt();
            stmts->addStatement(std::move(printStmt));
        } else {
            die(scope, "Expected `NAME` || `PRINT`, instead got", tok);
        }

        tok = lexer.getToken();

        if ( tok->eof() )
            break;

        if ( !tok->eol() ) {
            die(scope, "Expected `EOL`, instead got", tok);
        }

        tok = lexer.getToken();
    }  

    lexer.ungetToken();

    if (debug)
        std::cout << scope << " return" << std::endl;

    return stmts;
}

std::unique_ptr<PrintStatement> Parser::print_stmt() {

    std::string scope = "Parser::print_stmt()";

    if (debug)
        std::cout << scope << std::endl;

    auto tok = lexer.getToken();

    if ( !tok->isPrint() )
        die(scope, "Expected `PRINT` keyword, instead got", tok);

    auto testList = testlist();

    return std::make_unique<PrintStatement>(std::move(testList));

}

std::unique_ptr<AssignStmt> Parser::assign_stmt() {

    std::string scope = "Parser::assign_stmt";
    
    if (debug)
        std::cout << scope << std::endl;

    auto varName = lexer.getToken();

    if ( !varName->isName() )
        die(scope, "Parser::assign_stmt() expected `NAME` instead got ", varName);

    auto assignOp = lexer.getToken();
    if ( !assignOp->isAssignmentOperator() )
        die(scope, "Parser::assign_stmt() expected `ASSIGN_OP` instead got ", assignOp);

    std::unique_ptr<ExprNode> rightHandSideExpr = test();

    if (debug)
        std::cout << scope << " return" << std::endl;


    return std::make_unique<AssignStmt>(varName->getName(), std::move(rightHandSideExpr));
}

std::unique_ptr<Statements> Parser::compound_stmt() {

    std::string scope = "Parser::compound_stmt";
    if (debug)
        std::cout << scope << std::endl;

    auto stmts = std::make_unique<Statements>();
    auto tok = lexer.getToken();

    if (tok->isFor()) {

        lexer.ungetToken();
        stmts->addStatement(for_stmt());

    } else if (tok->isIf()) {

        lexer.ungetToken();
        stmts->addStatement(if_stmt());

    } else {
        die(scope, "Parser::compound_stmt() expected _keyword -> { FOR | IF }, instead got ", tok);
    }

    if (debug)
        std::cout << scope << " return" << std::endl;


    return stmts;
}

std::unique_ptr<IfStatement> Parser::if_stmt() {

    // Parses the grammar rule 

    // <if_stmt> -> 'if' <test> ':' suite { 'elif' test ':' suite }* ['else' ':' suite]

    std::string scope = "Parser::if_stmt";
    
    if (debug)
        std::cout << scope << std::endl;

    auto ifStatement = std::make_unique<IfStatement>();
    auto tok = lexer.getToken();

    if (! tok->isIf()) {
        die(scope, "Expected `if` keyword, instead got", tok);
    }

    // ExprNode *comp = test();
    std::unique_ptr<ExprNode> comp = test();

    tok = lexer.getToken();

    if ( !tok->isColon() ) {
        die(scope, "Expected `:` keyword, instead got", tok);
    }

    std::unique_ptr<GroupedStatements> stmts = suite();

    auto ifStmt = std::make_unique<IfStmt>(std::move(comp), std::move(stmts));
    ifStatement->addIfStmt(std::move(ifStmt));
    tok = lexer.getToken();

    bool conditionHit = false;

    if (tok->isElIf()) {

        conditionHit = true;
        auto elseIfStatements = std::make_unique<ElifStmt>();

        while (tok ->isElIf()) {

            std::unique_ptr<ExprNode> elifCmp = test();

            tok = lexer.getToken();
            if ( !tok->isColon() ) {
                die(scope, "Expected `:` keyword, instead got", tok);
            }

            std::unique_ptr<GroupedStatements> stmts = suite();

            elseIfStatements->addStatement(std::move(elifCmp), std::move(stmts));

            tok = lexer.getToken();
            conditionHit = false;
        }

    //need this unget token for if / elif no else
        // lexer.ungetToken();

        ifStatement->addElifStmt(std::move(elseIfStatements));
    }

    if (tok->isElse()) {

        conditionHit = true;
        tok = lexer.getToken();
        if ( !tok->isColon() )
            die(scope, "Expected `:` keyword, instead got", tok);

        std::unique_ptr<GroupedStatements> stmts = suite();
        auto elseStmt = std::make_unique<ElseStmt>(std::move(stmts));

        ifStatement->addElseStmt(std::move(elseStmt));
    }

    if ( !conditionHit )
        lexer.ungetToken();


    if (debug)
        std::cout << scope << " return" << std::endl;

    return ifStatement;
}

std::unique_ptr<RangeStmt> Parser::for_stmt() {

    std::string scope = "Parser::for_stmt";

    if (debug)
        std::cout << scope << std::endl;

    auto tok = lexer.getToken();

    if (! tok->isFor() )
        die(scope, "Expected `FOR` _keyword, instead got", tok);

    // GET ID
    tok = lexer.getToken();

    if ( ! tok->isName() )
        die(scope, "Expected `NAME` _keyword, instead got", tok);

    std::string varName = tok->getName();

    tok = lexer.getToken();

    if (! tok->isIn() )
        die(scope, "Expected `IN` _keyword, instead got", tok);


    tok = lexer.getToken();

    if ( !tok->isRange() )
        die(scope, "Expected `RANGE`, instead got", tok);

    tok = lexer.getToken();


    if ( !tok->isOpenParen() )
        die(scope, "Expected `OPENPAREN`, instead got", tok);


    auto list = testlist();

    tok = lexer.getToken();

    if ( !tok->isCloseParen() ) {
        die(scope, "Expected `CLOSEPAREN`, instead got", tok);
    }

    tok = lexer.getToken();

    if ( !tok->isColon() )
        die(scope, "Expected `:` symbol, instead got", tok);

    std::unique_ptr<GroupedStatements> stmts = suite();

    if (debug)
        std::cout << scope << " return" << std::endl;

    std::unique_ptr<RangeStmt> range = std::make_unique<RangeStmt>(varName);
    // range->parseTestList(list);
    range->addTestList(std::move(list));
    range->addStatements(std::move(stmts));

    return range;
    // return std::make_unique<ForStmt>(varName, wholeNumber, std::move(stmts));
}

std::unique_ptr<GroupedStatements> Parser::suite() {
    //Parses the grammar rule 
    // <suite> -> EOL INDENT stmt+ DEDENT

    std::string scope = "Parser::suite";

    std::unique_ptr<GroupedStatements> groupedStatements = std::make_unique<GroupedStatements>();

    if (debug)
        std::cout << scope << std::endl;

    auto tok = lexer.getToken();

    if ( !tok->eol() )
        die(scope, "Expected an `EOL`, instead got", tok);

    tok = lexer.getToken();

    if ( !tok->isIndent() )
        die(scope, "Expected an `INDENT`, instead got", tok);

    //Rule is +
    auto stmts = stmt();
    groupedStatements->addStatements(std::move(stmts));
    tok = lexer.getToken();

    while ( tok->isPrint() || tok->isName() || tok->isIf() || tok->isFor() ) {

        lexer.ungetToken();
        auto stmts = stmt();
        groupedStatements->addStatements(std::move(stmts));

        tok = lexer.getToken();
    }

    if ( !tok->isDedent() )
        die(scope, "Expected a DEDENT, instead got", tok);

    if (debug)
        std::cout << scope << " return" << std::endl;

    return groupedStatements;
}

std::unique_ptr<std::vector<std::unique_ptr<ExprNode>>> Parser::testlist() {

    std::string scope = "Parser::testlist()"; 

    auto p = std::make_unique<std::vector<std::unique_ptr<ExprNode>>>();
    p->push_back(test());

    auto tok = lexer.getToken();

    while ( tok->isComma() ) {
        p->push_back(test());
        tok = lexer.getToken();
    }

    lexer.ungetToken();
    return p;
}

std::unique_ptr<ExprNode> Parser::test() {

    std::string scope = "*Parser::test()";

    if (debug)
        std::cout << scope << std::endl;

    return or_test();
}

std::unique_ptr<ExprNode> Parser::or_test() {

    std::string scope = "*Parser::or_test()";

    if (debug)
        std::cout << scope << std::endl;

    std::unique_ptr<ExprNode> left = and_test();
    auto tok = lexer.getToken();

    while ( tok->isOr() ) {
        std::unique_ptr<BooleanExprNode> p = std::make_unique<BooleanExprNode>(tok);
        p->_left = std::move(left); 
        p->_right = and_test();
        left = std::move(p);
        tok = lexer.getToken();
    }

    lexer.ungetToken();

    if (debug)
        std::cout << scope << " return" << std::endl;

    return left;
}

std::unique_ptr<ExprNode> Parser::and_test() {
    std::string scope = "*Parser::and_test()";

    if (debug)
        std::cout << scope << std::endl;

    std::unique_ptr<ExprNode> left = not_test();
    auto tok = lexer.getToken();

    while ( tok->isAnd() ) {

        std::unique_ptr<BooleanExprNode> p = std::make_unique<BooleanExprNode>(tok);
        p->_left = std::move(left);
        p->_right = not_test();
        left = std::move(p);
        tok = lexer.getToken();
    }
    lexer.ungetToken();


    if (debug)
        std::cout << scope << " return" << std::endl;

    return left;
}

std::unique_ptr<ExprNode> Parser::not_test() {

    std::string scope = "*Parser::not_test()";

    if (debug)
        std::cout << scope << std::endl;   

    auto tok = lexer.getToken();

    if ( tok->isNot() ) {
        std::unique_ptr<BooleanExprNode> p = std::make_unique<BooleanExprNode>(tok);
        p->_left = not_test();
        return p;

    } else {
        lexer.ungetToken();
        return comparison();
    }
}

std::unique_ptr<ExprNode> Parser::comparison() {
    // This function parses the grammar rule:

    // <comparison> -> <arith_expr> { <comp_op> <arith_expr> }*

    std::string scope = "*Parser::comparison()";

    if (debug)
        std::cout << scope << std::endl;

    std::unique_ptr<ExprNode> left = arith_expr();
    auto tok = lexer.getToken();
    
    while (tok->isCompOp()) {
        std::unique_ptr<ComparisonExprNode> p = std::make_unique<ComparisonExprNode>(tok);

        p->_left = std::move(left);
        p->_right = arith_expr();
        left = std::move(p);
        tok = lexer.getToken();
    }

    lexer.ungetToken();

    if (debug)
        std::cout << scope << " return" << std::endl;

    return left;
}

std::unique_ptr<ExprNode> Parser::arith_expr() {
    // This function parses the grammar rules:

    // <arith_expr> -> <term> { ( + | - ) <term> }
    std::string scope = "*Parser::arith_expr()";

    if (debug)
        std::cout << scope << std::endl;

    std::unique_ptr<ExprNode> left = term();
    auto tok = lexer.getToken();

    while (tok->isAdditionOperator() || tok->isSubtractionOperator()) {
        std::unique_ptr<InfixExprNode> p = std::make_unique<InfixExprNode>(tok);
        p->_left = std::move(left);
        p->_right = term();
        left = std::move(p);
        tok = lexer.getToken();
    }
    lexer.ungetToken();

    if (debug)
        std::cout << scope << " return" << std::endl;

    return left;
}

//TODO -> add support for -> // <- op in tokenizer
std::unique_ptr<ExprNode> Parser::term() {
    // This function parses the grammar rules:

    // <term> -> <factor> { ( * | / | % | // ) <factor> }*
    std::string scope = "ExprNode *Parser::term()";

    if (debug)
        std::cout << scope << std::endl;
    
    std::unique_ptr<ExprNode> left = factor();
    
    auto tok = lexer.getToken();
    
    while ( tok->isMultiplicationOperator() || tok->isDivisionOperator() || tok->isModuloOperator() ) {
        std::unique_ptr<InfixExprNode> p = std::make_unique<InfixExprNode>(tok);
        p->_left = std::move(left);
        p->_right = factor();
        left = std::move(p);
        tok = lexer.getToken();
    }
    lexer.ungetToken();

    if (debug)
        std::cout << scope << " return" << std::endl;

    return left;
}

std::unique_ptr<ExprNode> Parser::call(std::shared_ptr<Token> ID) {

    std::string scope = "Parser::call()";
    auto tok = lexer.getToken();

    if ( !tok->isOpenParen() ) {
        die(scope, "Expected (", tok);
    }

    std::unique_ptr<std::vector<std::unique_ptr<ExprNode>>> tlist = testlist();

    tok = lexer.getToken();

    if ( !tok->isCloseParen() ) {
        die(scope, "Expected )", tok);
    }

    return std::make_unique<FunctionCall>(ID, std::move(tlist));

}

std::unique_ptr<ExprNode> Parser::factor() {
    // This function parses the grammar rules:

    // <factor> -> {'-'} <factor> | <atom>
    // TODO: is this correct?
    std::string scope = "*Parser::factor()";

    if (debug)
        std::cout << scope << std::endl;

    auto tok = lexer.getToken();

    if ( tok->isSubtractionOperator() ) {
        std::unique_ptr<InfixExprNode> p = std::make_unique<InfixExprNode>(tok);
        p->_left = factor();
        p->_right = nullptr;

        return p;

    } else {

        lexer.ungetToken();

        if (debug)
            std::cout << scope << " return" << std::endl;

        auto left = atom();

        if ( left->token()->isName() ) {
            if ( lexer.getToken()->isOpenParen() ) {
                //function
                lexer.ungetToken();
                return call( left->token() );
            } else {
                lexer.ungetToken();
            }           

        }

        return left;

        
    }
    die("Parser::factor", "ERROR", tok);
    return nullptr;
}


std::unique_ptr<ExprNode> Parser::atom() {
    //This function parses the grammar rules:

    // <atom> -> <id>
    // <atom> -> <number>
    // <atom> -> <string>+
    // <atom> -> '(' <test> ')'
    std::string scope = "Parser::atom";

    if (debug)
        std::cout << scope << std::endl;    

    auto tok = lexer.getToken();

    if ( tok->isName() )              // <id>
        return std::make_unique<Variable>(tok);
    else if ( tok->isWholeNumber() )  // <number>
        return std::make_unique<WholeNumber>(tok);
    else if ( tok->isString() )      // <string> NOT +
        return std::make_unique<StringExp>(tok);
    else if ( tok->isFloat() ) /*Swap to isDouble */
        return std::make_unique<Double>(tok); 
    else if ( tok->isOpenParen() ) {
        std::unique_ptr<ExprNode> p = test();
        auto token = lexer.getToken();
        if ( !token->isCloseParen() )
            die("Parser::atom", "Expected close-parenthesis, instead got", token);
        return p;
    }
    die(scope, "Unexpected token", tok);

    return nullptr;
}

