#ifndef __STATEMENT_HPP
#define __STATEMENT_HPP

#include <memory>
#include <vector>
#include <optional>

#include "../SymTab.hpp"
#include "../ArithExpr.hpp"
#include "../Token.hpp"
#include "../Debug.hpp"
#include "../Descriptor.hpp"
#include "../DescriptorFunctions.hpp"

class IfStmt;
class ElifStmt;
class ElseStmt;

class Statement {

public:
    Statement();

    virtual ~Statement() = default;
    virtual void evaluate(SymTab &symTab) = 0;
    virtual void dumpAST(std::string) = 0;
};

class Statements {

public:
    Statements();

    void addStatement(std::unique_ptr<Statement> statement);
    void evaluate(SymTab &symTab);
    ~Statements();

    void dumpAST(std::string);

private:
    std::vector<std::unique_ptr<Statement>> _statements;
};

class GroupedStatements {

public:
    GroupedStatements() = default;

    void addStatements(std::unique_ptr<Statements> statements);
    void evaluate(SymTab &symTab);
    ~GroupedStatements(); /*= default;*/

    void dumpAST(std::string);

private:
    std::vector<std::unique_ptr<Statements>> _groupedStatements;
};

class AssignStmt : public Statement {

public:
    // AssignStmt(std::string, ExprNode *);
    AssignStmt(std::string, std::unique_ptr<ExprNode>);

    virtual ~AssignStmt();
    virtual void evaluate(SymTab &symTab);
    virtual void dumpAST(std::string);
private:
    std::string _lhsVariable;
    std::unique_ptr<ExprNode> _rhsExpression;
    // ExprNode *_rhsExpression;
};

class IfStatement : public Statement {

public:
    IfStatement();

    void addIfStmt(std::unique_ptr<IfStmt>);
    void addElifStmt(std::unique_ptr<ElifStmt>);
    void addElseStmt(std::unique_ptr<ElseStmt>);

    virtual ~IfStatement();
    virtual void evaluate(SymTab &symTab);
    virtual void dumpAST(std::string);

private:
    std::unique_ptr<IfStmt>   _if;
    std::unique_ptr<ElifStmt> _elif;
    std::unique_ptr<ElseStmt> _else;
};

class PrintStatement : public Statement {

public:
    // PrintStatement(std::string);
    PrintStatement(std::unique_ptr<std::vector<std::unique_ptr<ExprNode>>>);

    virtual ~PrintStatement();
    virtual void evaluate(SymTab &symTab);
    virtual void dumpAST(std::string);

private:
    std::unique_ptr<std::vector<std::unique_ptr<ExprNode>>> _testList;
};


class RangeStmt : public Statement {

public:
    RangeStmt(std::string);

    virtual ~RangeStmt();
    virtual void evaluate(SymTab &symTab);
    virtual void dumpAST(std::string);

    void parseTestList(SymTab &symTab);
    
    void addStatements(std::unique_ptr<GroupedStatements>);
    
    void addTestList(std::unique_ptr<std::vector<std::unique_ptr<ExprNode>>>);
    void editOptionals(int, std::optional<int>);

private:
    std::string _id;
    std::optional<int> _start, _end, _step;

    std::unique_ptr<GroupedStatements> _forBody;

    std::unique_ptr<std::vector<std::unique_ptr<ExprNode>>> _testList;
};

class FunctionDefinition : public Statement  {
public:
    Statement();

    virtual ~Statement() = default;
    virtual void evaluate(SymTab &symTab) = 0;
    virtual void dumpAST(std::string) = 0;
    
};
 

class Comparison {

public:
    Comparison();
    virtual ~Comparison() = default;
    virtual bool evaluate(SymTab &symTab) = 0;
    virtual void dumpAST(std::string) = 0;
};

class IfStmt : public Comparison {

public:

    IfStmt(
        std::unique_ptr<ExprNode>,
        std::unique_ptr<GroupedStatements>
    );

    virtual ~IfStmt(); /* = default;*/
    virtual bool evaluate(SymTab &symTab);
    virtual void dumpAST(std::string);

private: 
    std::pair<
        std::unique_ptr<ExprNode>,
        std::unique_ptr<GroupedStatements>
    > _if;
};

class ElifStmt : public Comparison {

public:
    ElifStmt();
    virtual ~ElifStmt(); /* = default*/;

    void addStatement(std::unique_ptr<ExprNode>, std::unique_ptr<GroupedStatements>);
    // void addStatement(ExprNode *, std::unique_ptr<GroupedStatements>);

    virtual bool evaluate(SymTab &symTab);
    virtual void dumpAST(std::string);
 
private: 
    std::vector<
        std::pair<
            std::unique_ptr<ExprNode>,
            std::unique_ptr<GroupedStatements>
        >
    > _elif;
};

class ElseStmt : public Comparison {

public:
    ElseStmt(std::unique_ptr<GroupedStatements>);
    virtual ~ElseStmt(); /* = default;*/

    virtual bool evaluate(SymTab &symTab);
    virtual void dumpAST(std::string);

private:
    std::unique_ptr<GroupedStatements> stmts;
};


#endif