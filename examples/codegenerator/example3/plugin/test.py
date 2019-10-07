
import something
import xbmc

class ExtendedExample(something.Example):
    def __init__(self, message):
        super(something.Example,self).__init__(message)

    def funcToCall(self, message) :
        xbmc.log("overridden - " + message, xbmc.LOGNOTICE)

if __name__ == '__main__':
    e = something.Example("hello in constructor");
    e.callFunc()
    ee = ExtendedExample("hello in child constructor");
    ee.callFunc()
    