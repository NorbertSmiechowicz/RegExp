
#ifndef REPROD_H
#define REPROD_H

/*

QUOTED_CHAR 
    \^    \.    \[    \]    \$    \(    \)    \|
    \*    \+    \?    \{    \}    \\






*/

class ExtendedRegExp 
{
private:
    ExtendedRegExp*     m_Expression;
    Branch*             m_Branch;
};

class Branch
{
private:
    Branch*             m_Branch;
    Expression*         m_Expression;   
};

class Expression
{
private:
    OneCharOrCollElem*  m_OneCharOrCollElem;
    Anchor*             m_Anchor;
};

#endif

#ifndef REPARSER_H
#define REPARSER_H

class RegExpParser
{
public:

};

#endif // REPARSER_H
