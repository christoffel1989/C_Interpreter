#pragma once;

#include "ASTNode.h"

//执行语法树
double executeAST(std::shared_ptr<ASTNode> ast, Environment* env);