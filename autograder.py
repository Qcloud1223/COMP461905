#! /usr/bin/env python3
from curses import noecho
import subprocess
import signal
import sys

# C-style define
VOID_VOID = '0'
INT_INTINT = '1'

INT_INTINT_L = 1 << 4

class FuncUsage:

    def __init__(self, funcList) -> None:
        self.funcList = funcList

    def compare(self, log):
        # print("***printing from func_usage class!")
        # print(log)
        stderrLine = log.split('\n')
        violate = 0
        for line in stderrLine:
            if len(line.split(':')) != 2:
                # ignore if the line does not conform to <funcName>: <usage>
                continue
            funcName = line.split(":")[0]
            usage    = line.split(":")[1]
            usageNum = int(usage)
            if usageNum == 0:
                continue
            if funcName in self.funcList:
                if self.funcList[funcName] != usageNum:
                    print("Function name:", funcName, "Expected usage:", self.funcList[funcName], "Real usage:", usageNum)
                    violate += 1
            else:
                # find a function that is not inside the function list
                print("Function name:", funcName, "Expected usage: 0, Real usage:", usageNum)
                violate += 1
        if violate == 0:
            return True
        else:
            return False

class TestCase:

    dlopenExe = './build/run-dlopen'
    customLdExe = './build/run-openlib'

    def __init__(self, soName, funcName, funcType, *args):
        self.soName = soName
        self.soName = './test_lib/' + soName + '.so'
        self.funcName = funcName
        self.funcType = funcType
        self.extraArg = len(args)
        self.args = args
        self.craftAns = 0
        self.funcWrapper = None

    def check_usage(self, log):
        if self.funcWrapper is not None:
            ret = self.funcWrapper.compare(log)
            return ret
        else:
            # no function wrapper, we don't check it 
            return True
    
    def add_wrapper(self, wrapList):
        self.funcWrapper = FuncUsage(wrapList)

    def assign_score(self, score):
        self.score = score
        self.claimedScore = 0
    
    def assign_name(self, name):
        self.testName = name
    
    def debug(self):
        self.debugArg = ['gdb', '--args']
        self.debugArg.extend((self.customLdExe, self.soName, self.funcName, self.funcType))
        if self.extraArg:
            self.debugArg.extend(self.args)
        subprocess.call(self.debugArg)
    
    def add_answer(self, ans):
        self.craftAns = 1
        self.ans = ans.decode("utf-8")
    
    def run_task(self):
        print("Test name:", self.testName)
        argList = []
        argList.extend((self.customLdExe, self.soName, self.funcName, self.funcType))
        if self.extraArg:
            argList.extend(self.args)
        try:
            # print(argList)
            # The reason why sometimes I do not want to encode it is that it can print non-utf8 char
            # typically from an uninitialized buffer
            if sys.version_info >= (3, 7, 0):
                RetProc = subprocess.run(argList, check=True, capture_output=True, encoding='utf-8')
                # RetProc = subprocess.run(argList, check=True, capture_output=True)
            else:
                RetProc = subprocess.run(argList, check=True, stderr=subprocess.PIPE, stdout=subprocess.PIPE)
                # print(type(RetProc), type(RetProc.stdout))
                #   -> subprocess.CompletedProcess, bytes
                RetProc.stdout = RetProc.stdout.decode('utf-8')
                RetProc.stderr = RetProc.stderr.decode('utf-8')
        # I get lazy and don't decode the error string, for we do not need it to compare
        except subprocess.CalledProcessError as e:
            if e.returncode == -signal.SIGSEGV:
                print("SIGSEGV received in custom loader. Maybe you want to debug it with gdb.")
            elif e.returncode == -signal.SIGABRT:
                print("SIGABRT received in custom loader. Exit as expected.")
                print("Last words from stdout:", e.stdout)
                print("Last words from stderr:", e.stderr)
            else:
                print("Oops, custom loader does not return normally.")
                print("Last words from stdout:", e.stdout)
                print("Last words from stderr:", e.stderr)
                print("Bad return code:", e.returncode)
            return
        else:
            # open test program in dlopen, if there is no problem in custom loader
            argList[0] = self.dlopenExe
            try:
                # print(argList)
                if sys.version_info >= (3, 7, 0):
                    DLProc = subprocess.run(argList, check=True, capture_output=True, encoding='utf-8')
                    # DLProc = subprocess.run(argList, check=True, capture_output=True)
                else:
                    DLProc = subprocess.run(argList, check=True, stderr=subprocess.PIPE, stdout=subprocess.PIPE)
                    DLProc.stdout = DLProc.stdout.decode('utf-8')
                    DLProc.stderr = DLProc.stderr.decode('utf-8')
            except subprocess.CalledProcessError as e:
                if e.returncode == -signal.SIGSEGV:
                    print("SIGSEGV received while dlopen. Please contact the TA.")
                else:
                    print("Oops, error in dlopen. Please contact the TA.")
                    print("Last words from stdout:", e.stdout)
                    print("Last words from stderr:", e.stderr)
                    print("Bad return code:", e.returncode)
                return
            else:
                if self.craftAns:
                    realAns = self.ans
                else:
                    realAns = DLProc.stdout
                if RetProc.stdout == realAns:
                    if self.check_usage(RetProc.stderr) == False:
                        # handle usage comparation in FuncUsage class
                        print("Correct output while abusing system functions.\nYou are free to test your implementation in this way, but no score for you.")
                        print("Shim checking failed!")
                    else:
                        self.claimedScore += self.score
                        print("Expected output:", realAns)
                        print("Your output:", RetProc.stdout)
                        print("Passed!")
                else:
                    print("TestCase failed.")
                    print("Expected output:", realAns)
                    print("Your output:", RetProc.stdout)

def pretty_print(i):
    print('#' * 30)
    print("#{}#".format(' ' * 28))
    if i < 10:
        print('#   Evaluating TestCase ${}   #'.format(i))
    # guess we don't have over 100 testcases, hopefully...
    else:
        print('#   Evaluating TestCase ${}  #'.format(i))
    print("#{}#".format(' ' * 28))
    print('#' * 30)
    print()

if __name__ == '__main__':
    if sys.version_info < (3, 5, 0):
        print("Your python version is too low, please update it to be at least 3.5")
        quit()
    allTests = []
    test0 = TestCase('SimpleMul', 'multiply', INT_INTINT, '2', '3')
    test0.assign_score(80)
    test0.assign_name('zero relocation')
    test0.add_wrapper(dict())
    allTests.append(test0)
    test1 = TestCase('lib1', 'foo', VOID_VOID)
    test1.assign_score(5)
    test1.assign_name('fake libc(one PLT relocation with answer known)')
    test1.add_wrapper({"dlopen": 1, "dlsym": 2})
    allTests.append(test1)
    test2 = TestCase('SimpleIni', 'entry', VOID_VOID)
    test2.assign_score(5)
    test2.add_wrapper({"dlopen": 1, "dlsym": 2})
    test2.assign_name('one initialization(depend on test 1)')
    allTests.append(test2)
    test3 = TestCase('SimpleDep', 'wrapper', INT_INTINT, '2', '3')
    test3.assign_score(3)
    test3.assign_name('one true PLT relocation')
    test3.add_wrapper(dict())
    allTests.append(test3)
    test4 = TestCase('SimpleData', 'wrapper', VOID_VOID)
    test4.assign_score(3)
    test4.assign_name('one global data relocation')
    test4.add_wrapper({"dlopen": 1, "dlsym": 2})
    allTests.append(test4)
    test5 = TestCase('IndirectDep', 'wrapperAgain', INT_INTINT, '2', '3')
    test5.assign_score(2)
    test5.assign_name('one 2-layer relocation')
    test5.add_wrapper(dict())
    allTests.append(test5)
    # TODO: check whether the second call still calls trampoline
    # requires a distinct function type
    test6 = TestCase('SimpleDep', 'wrapper', str(INT_INTINT_L), '2', '3')
    test6.assign_score(2)
    test6.assign_name('lazy binding')
    test6.add_answer(b'Resolving address for entry 0\n6\n')
    test6.add_wrapper({"printf": 1})
    allTests.append(test6)
    # sanity tests end
    """
        further tests can include: 
        order-related init; 
        rpath handling(single and multi layer);
        register saving in assembly;
        relocation with addend;
        various relocation types;
        syspath searching;
        these tests are kinda like corner cases and contradict the goal of this project 
    """

    if len(sys.argv) > 2:
        print("Please specify an individual testcase!")
        exit(-1)
    elif len(sys.argv) == 2:
        allTests[int(sys.argv[1])].debug()
        print('-'*50)
        print("gdb exits")
        exit(0)

    totalScore = 0
    claimedScore = 0
    for i, t in list(enumerate(allTests)):
        pretty_print(i)
        t.run_task()
        totalScore += t.score
        claimedScore += t.claimedScore
        print('-'*50)
    print("Your Score:", claimedScore, "/", totalScore)

