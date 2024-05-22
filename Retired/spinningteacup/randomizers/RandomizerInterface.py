#This class represents what the public class methods of a randomizer needs to be in order to operate properly

class baseRandomizer():
    def __init__(self):
        self.rstring = None
        self.rvariable = None
        self.rnum = None

    def set_variable_randomizer(self, name, **kwargs):
        raise Exception("set variable randomizer not configured for class {}".format(self.__class__.__name__))

    def set_numeric_randomizer(self, name, **kwargs):
        raise Exception("set numeric randomizer not configured for class {}".format(self.__class__.__name__))

    def set_string_randomizer(self, name, **kwargs):
        raise Exception("set string randomizer not configured for class {}".format(self.__class__.__name__))

    def reset(self, data):
        raise Exception("reset not defined for class {}".format(self.__class__.__name__))

    def randomize(self):
        #Here is where you verify you have the data you need, methods are set, and you return the randomized script
        raise Exception("reset not defined for class {}".format(self.__class__.__name__))
