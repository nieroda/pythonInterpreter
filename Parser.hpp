
#ifndef __PARSER_HPP
#define __PARSER_HPP

#include <vector>
#include <string>

#include "statements/Statement.hpp"
#include "lex/Lexer.hpp"
#include "ArithExpr.hpp"
// #include "FunctionMap.hpp"
class Parser { 
    public:

        Parser(Lexer &);

        void die(
            std::string, 
            std::string, 
            std::shared_ptr<Token>
        );

        void getEOL(std::string);

        std::unique_ptr<Statements> file_input();

        std::unique_ptr<Statement> stmt();

        std::unique_ptr<Statement> simple_stmt();

        std::unique_ptr<AssignStmt> assign_stmt(std::shared_ptr<Token>);
        
        std::unique_ptr<Statement> compound_stmt();

        std::unique_ptr<PrintStatement> print_stmt();

        std::unique_ptr<IfStatement> if_stmt();
        std::unique_ptr<RangeStmt> for_stmt();

        std::unique_ptr<Statement> func_def();
        std::vector<std::string> parameter_list();

        std::unique_ptr<Statements> suite();

        std::unique_ptr<std::vector<std::unique_ptr<ExprNode>>> testlist();


        std::unique_ptr<ExprNode> test();
        std::unique_ptr<ExprNode> or_test();
        std::unique_ptr<ExprNode> and_test();
        std::unique_ptr<ExprNode> not_test();
        std::unique_ptr<ExprNode> comparison();
        std::unique_ptr<ExprNode> arith_expr();
        
        std::unique_ptr<ExprNode> term();
        
        std::unique_ptr<ExprNode> factor();

        std::unique_ptr<ExprNode> call(std::shared_ptr<Token>);

        std::unique_ptr<ExprNode> atom();

    private:
        Lexer &lexer;
        // std::shared_ptr<FunctionMap> _functionMap;

};

#endif