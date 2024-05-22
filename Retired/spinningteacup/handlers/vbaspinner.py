from grammer.vba.vbaListener import vbaListener
from grammer.vba.vbaParser import vbaParser
import re
import argparse

#Key = .text or .Ambiguousidentifier based on waht is returned
#when getting text need to go to stop +1
class vbaspinner(vbaListener):
    def __init__(self):
        self.enums = {}
        self.consts = {} # Const identifier unified randomized, const value in literals, not handled in const These can have type cahracters
        self.Variables = set()
        self.Structures = set()
        self.Macros = {}
        self.Literals = [] # don't care if its a match, literals get randomized differently each time
        self.Functions = set()
        self.TypeStructs = {} # need to track both type statment and arguments
        self.subroutines = {}
        self.Declare = {} # need to pull out the arguments here
        super(vbaspinner, self).__init__()

    def updatePos(self, origlength, newlength, startpos):
        posdiff = newlength - origlength
        for l in self.Literals:
            if startpos < l[0]:
                l[0] += posdiff
                l[1] += posdiff
            elif l[0] <= startpos < l[1]:
                l[1] += posdiff


    def printLiterals(self, data):
        for l in self.Literals:
            print("c = " + data[l[0]:l[1]+1])
            print("o = " + l[2])

    def stripComments(self, originaldata):
        stripper = re.compile(r"(^(\'|\s+\').*\n|\s?\'(?![^'\n]\")+\n|\'[^\"]+?\n)", re.MULTILINE)
        nodebug = re.compile(r'^.*?Debug..*?\(.*?\)[ \n]+?')
        newdata = re.sub(r'.*?Debug\..*?\(.*?\).*', '', originaldata)
        newdata = stripper.sub('\n', newdata)
        return newdata



    # def enterEnumerationStmt(self, ctx: vbaParser.EnumerationStmtContext): #word after enum token
        # print('found an enum')
        # print(repr(ctx.start.tokenIndex))
        # print(repr(ctx.start.start))
        # print(repr(ctx.start.stop))
        # print(repr(ctx.start.line))
        # print(repr(ctx.start.text))


    def enterConstSubStmt(self, ctx: vbaParser.ConstSubStmtContext): #parts of a const statement
        self.Variables.add(ctx.ambiguousIdentifier().getText())

    def enterVariableSubStmt(self, ctx:vbaParser.VariableSubStmtContext): # part of a normal variable definition
        self.Variables.add(ctx.ambiguousIdentifier().getText())

    # def enterMacroConstStmt(self, ctx:vbaParser.MacroConstStmtContext):
    #     for arg in ctx.argList().arg():
    #         self.Variables.add(arg.ambiguousIdentifier().getText())

    #For litterals we don't need to track what we change too
    #In our class definition, we want to take the overrides that return the updated types
    #tuple will be (startloc, endloc, constant)
    def enterLiteral(self, ctx:vbaParser.LiteralContext):
        self.Literals.append([ctx.start.start, ctx.start.stop, ctx.start.text])

    def enterTypeStmt(self, ctx:vbaParser.TypeStmtContext):
        self.Variables.add(ctx.ambiguousIdentifier().getText())
        if ctx.typeStmt_Element() != None:
            for arg in ctx.typeStmt_Element():
                self.Variables.add(arg.ambiguousIdentifier().getText())

    def enterFunctionStmt(self, ctx:vbaParser.FunctionStmtContext):
        self.Functions.add(ctx.ambiguousIdentifier().getText())
        if ctx.argList() != None:
            for arg in ctx.argList().arg():
                self.Variables.add(arg.ambiguousIdentifier().getText())

    def enterSubStmt(self, ctx:vbaParser.SubStmtContext):
        self.Functions.add(ctx.ambiguousIdentifier().getText())
        if ctx.argList() != None:
            for arg in ctx.argList().arg():
                self.Variables.add(arg.ambiguousIdentifier().getText())

    def enterDeclareStmt(self, ctx:vbaParser.DeclareStmtContext):
        if 'Alias' in ctx.getText():
            self.Functions.add(ctx.ambiguousIdentifier().getText())
        if ctx.argList() != None:
            for arg in ctx.argList().arg():
                self.Variables.add(arg.ambiguousIdentifier().getText())                     #In later iteration randomize these as well, need to be linked to function