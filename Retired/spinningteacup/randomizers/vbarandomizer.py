# all randomizers are classes
# the class will have two methods, required_function() and randomize_value(),
# required function returns a codeblock that must be inserted for randomize_string() to work properly (not randomized in v1)
# randomize_value takes a value and returns the modified version, up to main program to track usage if required
import random
import math
import string
import re
from enum import Enum
from handlers.vbaspinner import vbaspinner
from randomizers.RandomizerInterface import baseRandomizer
from antlr4 import *
from grammer.vba.vbaParser import vbaParser
from grammer.vba.vbaLexer import vbaLexer


businesswords = ['Expense_Account', 'Net_profit', 'Credit', 'Law_of_diminishing_returns', 'Macroeconomics', 'Cost_Insurance_Freight', 'Customer_Retention', 'High_End', 'Adjusted_Book_Value', 'Bullish', 'Close_end_Credit', 'Planning', 'Underwriter', 'Euro', 'Price_Planning', 'Volume', 'Open_end_Credit', 'Commodity_Exchange', 'Recession', 'Net_worth', 'Variable_annuity', 'Accounts_Payable', 'Abandonment_Value', 'Impaired_Capital', 'Variance', 'Accounts_Receivable_Financing', 'Expenses', 'Setup_costs', 'Labor_force', 'E_zine', 'Growth_Rate', 'Revolving_Fund', 'Cashback', 'Invisible_Exports', 'Terms_of_Trade', 'Accounting_Equation', 'Banner_Exchange', 'Liability', 'Turnkey_Contract', 'Hit', 'Bearish', 'Underwrite', 'Corporation', 'Income_Redistribution', 'Pro_forma_Invoice', 'Audit', 'Sales_Outlet', 'Margin_of_error', 'Wages', 'Variable_interest_rate', 'Capitalism', 'Gross_Profit', 'Sales_Promotion', 'Accounts_Receivable', 'Distributor', 'Optimize', 'Antitrust', 'Net_fixed_assets', 'Forward_Rate', 'IMF', 'Upsell', 'Pro_forma', 'Capital_Account', 'Deed_of_Trust', 'Debt_Capital', 'T_Bill', 'Wikis', 'Overdrawn', 'Telebanking', 'Controllable_Expenses', 'Globalization', 'Account_Balance', 'Commission', 'XML', 'edemption', 'NASDAQ', 'Market', 'Capitalization', 'Prepayment_Penalty', 'Weighted_Average', 'Exchange_Rate', 'Unit_of_Trade', 'B2C', 'Zero_fund', 'JavaScript', 'Perception', 'Back_Office', 'Future', 'Microbusiness', 'Distribution_Channel', 'Liquid_Assets', 'Quality', 'Order', 'Capital_Budget', 'Markup', 'Tax_Incentive', 'Net_errors_and_omissions', 'Deflation', 'Uncertainty_Analysis', 'Cash_Discount', 'Exchange', 'Withholding_tax', 'Abusive_Tax_Shelter', 'eCommerce', 'Black_market', 'Product', 'Price_war', 'Money_Laundering', 'Tax', 'Takeover', 'Current_Assets', 'Home_Page', 'Audiotaping', 'Certificate', 'No_load_fund', 'Import', 'Retailing', 'Bull_Market', 'Lifestyle', 'Service_Charge', 'Chief_Executive_Officer', 'Hacker', 'B2B', 'Marketing', 'Absorbed_Costs', 'Feedback', 'Cutthroat', 'Export_Agent', 'Original_Equipment_Manufacturer', 'Think_Tank', 'Insolvency', 'Resources', 'Payable', 'Chief_Financial_Officer', 'Money_Market', 'Overdraft_Facility', 'Capital_Appreciation', 'Fixed_Asset', 'Prebilling', 'Appraisal', 'Psychographics', 'Database', 'Backlink_Checking', 'Earnings', 'Finance', 'e_lance', 'Cash_Flow', 'Absorbed_Business', 'Limited_Liability', 'Niche', 'Unbalanced_Growth', 'Wholesale_price', 'Invoice', 'Equipment', 'Articles_of_Incorporation', 'Devaluation', 'Overdraft', 'Abstract_Of_Title', 'Facsimile_Machine', 'Cost_of_Goods_Sold', 'Brand_Name', 'Budget', 'Promotion', 'Obsolescence', 'Bill_of_Entry', 'Margin', 'Vision_Statement', 'Market_Targeting', 'Desktop_Publishing', 'ZeeEnd', 'Wall_Street', 'Market_Demand', 'Year_end', 'Refinance', 'Compounding', 'Freight_Forwarder', 'Annual_Report', 'Excise_Duty', 'Absorbed_Account', 'Tax_Return', 'Added_Value', 'Backup', 'After_Sales_Service', 'Telephone_Survey', 'Refund', 'Payment_Gateway', 'Cash_Cow', 'Appreciation', 'Flow_Chart', 'Direct_Mail', 'Limited_Partnership', 'Growth_Capital', 'Newsreader', 'Balance_of_Payments', 'Debit_Card', 'Equity', 'Price_Control', 'Vertical_Market', 'Market_Analysis', 'Impression', 'Payment_in_kind', 'Lemon', 'Unit', 'KPI', 'Keyword', 'Price_Discrimination', 'Sales_Quota', 'Bad_Debts', 'RSS', 'Mass_Marketing', 'Economist', 'Acceleration_Clause', 'Phantom_income', 'Inventory', 'Float', 'Unsecured_Debt', 'AAA', 'Amortization', 'Direct_Cost', 'Profit', 'High_pressure', 'Long_Tail', 'Taxable', 'Probability', 'Clicks_and_Bricks', 'Vulture_Capitalist', 'Floating_Rate', 'Equity_Capital', 'Self_liquidating', 'Dirty_Price', 'Cutting_edge', 'Suppliers', 'Bank_Guarantee', 'Collateral', 'Equal_Opportunities', 'Wallet_Technology', 'Gateway', 'Free_Enterprise', 'Wealth', 'Letter_of_Credit', 'Abandonment_Option', 'Accelerated_Cost_Recovery_System', 'Teamwork', 'Trademark', 'Response_Marketing', 'Whistleblowing', 'Banner_Advertising', 'Test_Marketing', 'Futures_Market', 'Variable', 'Account', 'Arrears', 'Commodity', 'Downsize', 'Team_Player', 'Unlimited_Liability', 'Centralization', 'Bankruptcy', 'Seasonal_Business', 'non_recurring', 'Economy', 'Interest_Rate', 'Internet', 'Lapse', 'Tags', 'Customer_Service', 'Franchise_Chain', 'Share', 'GIF', 'Net_cash_balance', 'Rate_of_Interest', 'Mail_Order', 'Piracy', 'Sales_Forecast', 'Variable_cost', 'Point_of_Purchase', 'Sales_Channel', 'Data', 'Bookkeeping', 'Cost_and_Freight', 'Undervalued', 'Net_assets', 'Bank_Statement', 'Deficit', 'Tax_Refund', 'Trade_Barrier', 'Negotiation', 'Verbal_Contract', 'Keystone', 'Capital_Asset', 'Cybermarketing', 'Income_Tax', 'Capitalist', 'Walk', 'Exchange_Controls', 'Venture_Funding', 'Tax_Bracket', 'Franchise', 'Receivable', 'Indirect_Channel', 'Fraud', 'Commodity_Future', 'Talent', 'Ergonomics', 'Sales_Network', 'Hedge_Fund', 'FIFO', 'Partnership', 'Close_Corporation', 'Salary', 'Money', 'Performance_Appraisal', 'Accounting_Rate_of_Return', 'Administrative_Expense', 'Accounting_Cost', 'Placement_fee', 'Direct_Selling', 'Waste_Management', 'E_business', 'Advertising', 'Capital_Controls', 'License', 'Gray_Market', 'Entrepreneurship', 'Net_capital', 'Letter_of_agreement', 'Virus', 'Brand', 'Service_Business', 'Balance_Sheet', 'Security', 'Ballpark', 'Market_Positioning', 'Rebating', 'Federal_Funds', 'Micromarketing', 'Affiliate', 'Back_Pay', 'Long_term_Liabilities', 'Agency', 'Bank_Card', 'Used_credit', 'Self_employment', 'Sample', 'User_Generated_Content', 'Assignment', 'Balanced_Investment_Strategy', 'Joint_Account', 'Factor', 'Qualified_Lead', 'Tax_shelter', 'Break_even', 'Price_Ceiling', 'Balance', 'Guerrilla_Marketing', 'Middleman', 'Social_bookmarking', 'Accounting_Insolvency', 'Authentication', 'Net_proceeds', 'Backlog', 'Prepaid_Expenses', 'Co_signers', 'Infomercial', 'Leverage', 'Operating_Cash_Flow', 'Viral_Marketing', 'Actuary', 'Response_Rate', 'Compound_Interest', 'Abilene_Paradox', 'Bill_of_Sale', 'Back_to_back_Loan', 'Venture_capital', 'Pink_slip', 'Profit_Margin', 'Zero_balance_account', 'Assets', 'Profit_and_Loss_Statement', 'Horizontal_Integration', 'Probation', 'Yield', 'Intellectual_Property', 'Forward_Pricing', 'Loan', 'Sales', 'Survey', 'Certified_Public_Accountant', 'Exporting', 'Management', 'Mortgage', 'Qualification_Payment', 'Product_line', 'Retail', 'Waiver_of_premium', 'Bear_Market', 'Seed_Money', 'Take_home_Pay', 'Hyperinflation', 'Capital_Goods', 'Overprice_T', 'Bottom_Line', 'Cash_Receipts', 'Redundancy', 'Choice', 'Absorption_Costing', 'Forecast', 'Business_Venture', 'Venture_Management', 'Rate_of_Return', 'Discount', 'Fundraising', 'Principal', 'Balance_of_Trade', 'Depreciation', 'Analysis', 'e_mail', 'Sales_Force', 'Tariff', 'Accrual_Basis', 'Differentiated_Marketing', 'Floor', 'Telemarketing', 'Operating_Costs', 'Original_Design_Manufacturer', 'Wholesaling', 'Holding_Company', 'GDP', 'GNP', 'Knowledgebase', 'Capital_Equipment', 'Poaching', 'Product_life_cycle', 'Market_Share', 'Value_addedValue_added_tax', 'Virtual_Organization', 'Terms_of_Sale', 'Accounts_Receivable_Factoring', 'Gap_Analysis', 'Merchandise', 'Ratio', 'PayPal', 'Recurring_Payments', 'Board_of_Directors', 'Competition', 'Target_market', 'DebtEquity_Ratio', 'Fixed_Expenses', 'Initial_Public_Offering', 'Organizational_Market', 'Downtime', 'Banker8217s_Draft', 'Benchmarking', 'Domain_Name', 'Entrepreneur', 'Accountability', 'Liquidity', 'Generation_X', 'Junk_Bond', 'Promotional_Pricing', 'Reinsurance', 'Game_Plan', 'Capital_inflow', 'Marketable', 'Mashups', 'Accounting_Period', 'Angel_Investor', 'Open_Market', 'Market_Niche', 'Unbundling', 'Joint_Ownership', 'Invest', 'Guarantor', 'Accounting_Cycle', 'Chamber_of_Commerce', 'Random_Sampling', 'Access_Bond', 'Target_marketing', 'Marketing_Mix', 'Sales_Representative', 'Freight', 'Bill_of_Lading', 'Above_the_line', 'Accelerated_Depreciation', 'Settlement', 'Unearned_Income', 'Price', 'Telecommut', 'Non_disclosure_agreement', 'Interest', 'Investment', 'Short_term_notes', 'Tax_subsidy', 'Passive_Investment_Management', 'Buyers_Market', 'Debt', 'Income_Tax_Return', 'Postdate', 'Accountant', 'Product_mix', 'Chief_Executive', 'Contract', 'Financial_Statements', 'Net_operating_income', 'e_learning', 'Enterprise', 'Revenue', 'Extranet', 'Debt_Instrument', 'Barren_Money', 'Cash', 'Commercial_Paper', 'Questionnaire', 'Vlog', 'Bar_Coding', 'Development_Capital', 'Voting_rights', 'Capital_Gains', 'Deficit_Spending', 'Incentive_Program', 'Prepaid_Interest', 'Patent', 'Liability_Insurance', 'Trade_Credit', 'Capital', 'Inflation', 'YouTube', 'Overhead', 'Fixed_Cost', 'Publicity', 'Producers', 'Income_Statement', 'Indirect_Cost', 'Objective', 'Fulfillment', 'Economic_Surplus', 'Deficit_Financing', 'Market_Development', 'Petty_cash', 'Capital_Allowance', 'Free_Market', 'Annuity', 'Trade_fair', 'Insurance', 'Guarantee', 'Feasibility_Study', 'Outsourcing', 'Marketing_Research', 'Agent', 'Fiscal', 'Freebie', 'Credit_Line', 'Price_Floor', 'Dot_com', 'Note', 'Economics', 'Board_of_Trustees', 'Market_Forecast', 'Tender', 'Current_Liabilities', 'Web_Marketing', 'Net_margin', 'Balanced_Budget', 'Loan_Agreement', 'Monopoly', 'Stock', 'Hard_Sell']
excluded_names = ["Workbook_Activate", "Workbook_AddinInstall", "Workbook_AddinUninstall", "Workbook_BeforeClose",
                  "Workbook_BeforePrint", "Workbook_BeforeSave", "Workbook_Deactivate", "Workbook_NewSheet",
                  "Workbook_Open", "Workbook_SheetActivate", "Workbook_SheetBeforeDoubleClick",
                  "Workbook_SheetBeforeRightClick", "Workbook_SheetCalculate", "Workbook_SheetChange",
                  "Workbook_SheetDeactivate", "Workbook_SheetFollowHyperlink", "Workbook_SheetSelectionChange",
                  "Workbook_WindowActivate", "Workbook_WindowDeactivate", "Workbook_WindowResize", "Worksheet_Activate",
                  "Worksheet_BeforeDoubleClick", "Worksheet_BeforeRightClick", "Worksheet_Calculate",
                  "Worksheet_Change", "Worksheet_Deactivate", "Worksheet_FollowHyperlink", "Worksheet_SelectionChange",
                  "Auto_Open", "Auto_Close", "Auto_Activate", "Auto_Deactivate", "AutoOpen", "AutoExec",
                  "Document_Close", "Document_ContentControlAfterAdd", "Document_ContentControlBeforeContentUpdate",
                  "Document_ContentControlBeforeDelete", "Document_ContentControlBeforeStoreUpdate",
                  "Document_ContentControlOnEnter", "Document_ContentControlOnExit", "Document_New", "Document_Open",
                  "Document_Sync", "Document_XMLAfterInsert", "Document_XMLBeforeDelete", "autonew"]

ENCODE_ROT = """
Function November(Tuesday)
    November = Chr(Tuesday - 19)
End Function

Function October(Wednesday)
    October = Left(Wednesday, 3)
End Function

Function April(Friday)
    April = Right(Friday, Len(Friday) - 3)
End Function

Function June(Thursday)
    Dim Tomorrow
    Do
    Tomorrow = Tomorrow + November(October(Thursday))
    Thursday = April(Thursday)
    Loop While Len(Thursday)
    June = Tomorrow
End Function

"""


class generateorType(Enum):
    randomstring = 1
    wordlist = 2


FUNCTIONTYPE = 0
VARIABLETYPE = 1


class vbaRandomizer(baseRandomizer):
    #  Should be an argument flag to not do cuts with newlines.  VBA only supports up to 25 line continuations and we can't see them when splitting up strings
    lineendingchoices = ["\" & ", "\" & _ \n", "\" & ", "\" & "]  # non newline 75% newline 25%

    def __init__(self, data):
        random.seed()
        self.usedWords = {}  # tracks used words when generating words so we don't repeat
        self.handler = vbaspinner()
        self.replaceStrings = False
        self.originalwordlist = []
        self.wordmappings = {}
        self.data = data
        self.requiredFunctions = []  # list of functions that need to be inserted in our code to support other calls
        # Functions will be inserted prior to any other randomization
        self.postrequiredFixups = []
        self.postrequiredFunctions = []  # list of functions that need to be inserted after the fact
        super(vbaRandomizer, self).__init__()

    def __fixdecodeFuncname(self):
        newpost = []
        for item in self.postrequiredFunctions:
            newpost.append(item.replace('June', self.wordmappings['June']))
        self.postrequiredFunctions = newpost

    def set_string_randomizer(self, name, **kwargs):
        if name == 'cuts':
            self.minlen = kwargs['mincut']
            self.maxlen = kwargs['maxcut']
            self.rstring = self.__cut_string
        elif name == 'encode':
            self.requiredFunctions.append(ENCODE_ROT)
            self.replaceStrings = True
            self.rstring = self.__encodestring
        elif name == 'encode_funcs':
            self.requiredFunctions.append(ENCODE_ROT)
            self.postrequiredFixups.append(self.__fixdecodeFuncname)
            self.replaceStrings = True
            self.rstring = self.__encode_call
        elif name == 'none':
            self.rstring = self.__nopRandom

    def set_variable_randomizer(self, name, **kwargs):
        if name == 'staticwordlist':
            self.pickword = random.Random()
            self.pickword.seed()
            if type(kwargs['wordlist']) != type([]):
                raise Exception('staticwordlist expects a wordlist that is already a list')
            self.wordlist = list(set(kwargs['wordlist']))
            self.originalwordlist = self.wordlist
            self.rvariable = self.__get_word_from_wordlist
        elif name == 'wordlist':
            self.pickword = random.Random()
            self.pickword.seed()
            with open(kwargs['wordlistpath'], 'r') as fp:
                data = fp.read()
            if data.count('\n') > 2:  # assume newline separated
                lines = data.replace('\r', '').split('\n')
            else:  # assume csv
                lines = data.replace('\r', '').replace('\n', '').split(',')
            self.wordlist = list(set(lines))
            self.originalwordlist = self.wordlist
            self.rvariable = self.__get_word_from_wordlist
        elif name == 'businesswords':
            self.pickword = random.Random()
            self.pickword.seed()
            self.wordlist = businesswords
            self.rvariable = self.__get_word_from_wordlist
        elif name == "none":
            self.rvariable = self.__nopRandom

    def set_numeric_randomizer(self, name, **kwargs):
        if name == "math":
            self.intgen = random.Random()
            self.rnum = self.__gen_whole_number
        elif name == "none":
            self.rnum = self.__nopRandom

    def reset(self, data):
        self.data = data
        self.usedWords = {}
        self.wordlist = self.originalwordlist
        self.wordmappings = {}
        random.seed()

    def randomize(self):
        if self.rstring is None or self.rvariable is None or self.rnum is None:
            Exception("attempt to randomize script without setting all randomizations methods")
        data = self.data
        addfunc = re.search(r"(^|[\t ]*)(?<![Dd]eclare )(?<![Pp]tr[Ss]afe )([Pp]rivate |[Pp]ublic |[Ff]riend )?([Ss]tatic )?([Ff]unction |[Ss]ub )", data)
        if addfunc == None:  # would be really odd to hit this
            insertloc = 0
        else:
            insertloc = addfunc.start()
        header = data[:insertloc]
        data = data[insertloc:]
        for script in self.requiredFunctions:
            data = script + data
        data = header + data
        data = self.handler.stripComments(data)
        lexer = vbaLexer(InputStream(data))
        stream = CommonTokenStream(lexer)
        parser = vbaParser(stream)
        tree = parser.module()
        walker = ParseTreeWalker()
        walker.walk(self.handler, tree)
        for item in self.handler.Literals:
            newval = self.__randomize_value(item[2])
            data = data[:item[0]] + newval + data[item[1] + 1:]
            self.handler.updatePos(len(item[2]), len(newval), item[0])
        with open('debugjune.txt', 'w') as fp:
            fp.write(data)
        # print('*************Post litterals************')
        for item in self.handler.Functions:
            # print('replacing function {}'.format(item))
            data = self.__replace_value(data, item, FUNCTIONTYPE)
            # print(data)
            # self.handler.printLiterals(data)
        for item in self.handler.Variables:
            # print('replacing variable {}'.format(item))
            data = self.__replace_value(data, item, VARIABLETYPE)
            # print(data)
            # self.handler.printLiterals(data)
        if self.postrequiredFixups != []:
            for i in self.postrequiredFixups:
                i()
        if self.postrequiredFunctions != []:
            addfunc = re.search(r"(^|\s*)(?<![Dd]eclare )(?<![Pp]tr[Ss]afe )([Ff]unction |[Ss]ub )", data, re.MULTILINE)
            if addfunc == None:
                insertloc = 0
            else:
                insertloc = addfunc.start()
            header = data[:insertloc]
            data = data[insertloc:]
            for script in self.postrequiredFunctions:
                data = script + data
            data = header + data
        return data

    # Wordlist Functions
    def __get_random_word(self, junk=''):
        length = random.randint(5, 10)
        word = ''.join(random.choice(string.ascii_letters) for i in range(length))
        while word in self.usedWords:
            word = ''.join(random.choice(string.ascii_letters) for i in range(length))
        self.usedWords[word] = 1
        return word

    def __get_word_from_wordlist(self, originalword):
        if len(self.wordlist) == 0:
            print('exhasted wordlist, falling back to random word')
            return self.__get_random_word()
        chosen = self.pickword.choice(self.wordlist)
        self.wordlist.remove(chosen)
        return chosen

    # String Randomization functions
    def __encodestring(self, originalstring):
        originalstring = originalstring[1:-1].replace('""', '"') # replace vba escapes since our encoding takes care of it
        prerot = " ".join(str(ord(char)) for char in originalstring)
        withrot = []
        count = 0
        lines = 1
        for hex in prerot.split():
            withrot.append(str(int(hex) + 19).zfill(3))
            if count != 0 and count % 60 < 3:
                withrot.append('" & _\n"')
                lines += 1
                if lines > 24:
                    raise Exception("String {} to long to use __encodestring function".format(originalstring))
            count += 3
        return 'June("{}")'.format("".join(withrot))

    ###
    def __encode_call(self, originalstring):
        if self.rvariable == self.__nopRandom:
            r = self.__get_random_word
        else:
            r = self.rvariable
        encoded = self.__encodestring(originalstring)
        funcname = r('bannana')  # we just want a word
        func = """
Function {}()
    {} = {}
End Function
""".format(funcname, funcname, encoded)
        self.postrequiredFunctions.append(func)
        return funcname

    def __cut_string(self, originalstring):  # need to handle "" being a quote litteral
        minlen = self.minlen
        maxlen = self.maxlen

        if originalstring == '\"\"':
            return originalstring
        parts = []
        offset = 0
        pos = random.randint(minlen, maxlen)
        index = 0
        linecontinueation = 0
        newline = ''
        while offset < len(originalstring):  # second part to avoid screwing up already tokenized strings
            string = originalstring[offset:offset + pos]
            while (string[-1] == ord('\"') and offset + pos + index < len(
                    originalstring)):  # "" is really the only valid escape I can find for vba
                # print('walking')
                index += 1
                string = originalstring[offset:offset + pos + index]
            newline = random.choice(self.lineendingchoices)
            if '_' in newline:
                linecontinueation += 1
            parts.append("\"" + string + newline)
            offset += pos + index
            index = 0
            if linecontinueation == 23:
                parts.append("\"{}\"".format(originalstring[offset:]))
                break
            pos = random.randint(minlen, maxlen)
        if parts == []:
            return originalstring
        parts[-1] = parts[-1][
                    :parts[-1].rfind("\"")] + "\""  # remove + and possible newlines / spaces from last part
        return ''.join(parts)[1:-1]

    # numeric randomizer
    def __gen_whole_number(self, original, endchar=''):  # endchar is the typechar if used
        try:
            origint = abs(int(original))
        except:
            print('Unhandled number {}'.format(original))
            return original
        if (origint == 0):
            return '0'
        i = 0
        totalval = 0
        isnegative = True if original[0] == '-' else False
        newnumbers = []
        while (totalval != origint):
            assert (totalval < origint)
            if i == 2:
                num = origint - totalval  # avoids statement to complex error
            else:
                num = self.intgen.randint(1, origint - totalval)
            totalval += num
            if (isnegative):
                num = -num
            newnumbers.append((num))
            i += 1
        replacestring = ''
        for num in newnumbers:
            replacestring += "{}{} + ".format(num, endchar)
        replacestring = replacestring[:replacestring.rfind('+') - 1]
        return "({})".format(replacestring)

    # nop randomizer
    def __nopRandom(self, value, *args, **kwargs):
        return value

    # backend randomizer selection
    def __randomize_value(self, value):
        length = len(value)
        if (value[0] == '"'):  # assuming this is a string, vba doesn't do ' ' for strings
            if length == 2 and value[1] == '"': # emptry string
                return '""'
            return self.rstring(value)
        elif (value[0] not in string.digits and value[0] not in ['-', '&']):  # likely a constant, don't touch those
            return value
        else:  # assuming an "integer" like value
            if value[0] == '&':  # representing in hex, octal or binary, passing for now
                return value
            elif value[0] == '#':  # this is a date, nope nope our way out of here
                return value
            else:
                typechar = value[-1]  # typechar needs to go on all parts
                if typechar in string.digits:
                    typechar = ''
                else:
                    value = value[:-1]
                remainder = value[value.find('.'):] if value.find('.') != -1 else -1
                whole = value[:value.find('.')] if remainder != -1 else value
                retnum = self.rnum(whole, typechar)
                if remainder != -1:
                    # print('remmainder is {}'.format(remainder))
                    remainder = '-' + remainder if value[0] == '-' else remainder
                    retnum += ' + {}{}'.format(remainder, typechar)
                return retnum

    def isinstring(self, pos):
        for item in self.handler.Literals:
            if self.replaceStrings:
                if pos == item[0]:
                    return (False, 0, 0)  # we are replacing string constants with something else
            if pos >= item[0] and pos <= item[1] + 1:
                return (True, item[0], item[1] + 1)
        return (False, 0, 0)

    # Generic value replacer
    def __replace_value(self, data, original, vartype):
        lastloc = 0
        parts = []
        if vartype == FUNCTIONTYPE:
            if original in excluded_names:
                return data
            formatstring = r"(\b)[\t ]*[ =+\-*&/]? *{}( ?\([\S \n]*?\)| +| *[\r\n])".format(original)
            locs = re.finditer(formatstring, data, re.MULTILINE)
        elif vartype == VARIABLETYPE:
            locs = re.finditer(r"(^|[ \t=+\-*&/\(]+ *|\.){}(\.| *[,\)=_& \-*/]+?| *\(| *[\r\n]+)".format(original),
                               data, re.MULTILINE)
        else:
            raise Exception("invalid vartype")
        replacement = self.rvariable(original)
        self.wordmappings[original] = replacement
        offset = 0
        updates = []
        for l in locs:
            base = data.find(original, l.start(), l.end())  # find start of our variable in match
            stat, start, end = self.isinstring(base)
            if stat:
                continue
            parts.append(data[offset:base] + replacement)
            offset = base + len(original)
            updates.append((len(original), len(replacement), base))
        parts.append(data[offset:])
        updates = sorted(updates, key=lambda x: x[2],
                         reverse=True)  # This is REALLY important so we don't have false moves inside of words
        for u in updates:
            self.handler.updatePos(u[0], u[1], u[2])
        return "".join(parts)
