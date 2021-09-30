#! /usr/bin/env python3
import subprocess
import signal

# C-style define
VOID_VOID = '0'
INT_INTINT = '1'

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

    def assign_score(self, score):
        self.score = score
        self.claimedScore = 0
    
    def assign_name(self, name):
        self.testName = name
    
    def run_task(self):
        print("Test name:", self.testName)
        argList = []
        argList.extend((self.customLdExe, self.soName, self.funcName, self.funcType))
        if self.extraArg:
            argList.extend(self.args)
        try:
            # print(argList)
            # The reason why somtimes I do not want to encode it is that it can print non-utf8 char
            # typically from an uninitialized buffer
            RetProc = subprocess.run(argList, check=True, capture_output=True, encoding='utf-8')
            # RetProc = subprocess.run(argList, check=True, capture_output=True)
        except subprocess.CalledProcessError as e:
            if e.returncode == -signal.SIGSEGV:
                print("SIGSEGV received in custom loader. Maybe you want to debug it with gdb.")
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
                DLProc = subprocess.run(argList, check=True, capture_output=True, encoding='utf-8')
                # DLProc = subprocess.run(argList, check=True, capture_output=True)
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
                if RetProc.stdout == DLProc.stdout:
                    self.claimedScore += self.score
                    print("Expected output:", DLProc.stdout)
                    print("Your output:", RetProc.stdout)
                    print("Passed!")
                else:
                    print("TestCase failed.")
                    print("Expected output:", DLProc.stdout)
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
    allTests = []
    test0 = TestCase('SimpleMul', 'multiply', INT_INTINT, '2', '3')
    test0.assign_score(80)
    test0.assign_name('zero relocation')
    allTests.append(test0)
    test1 = TestCase('lib1', 'foo', VOID_VOID)
    test1.assign_score(5)
    test1.assign_name('fake libc(one PLT relocation with answer known)')
    allTests.append(test1)
    test2 = TestCase('SimpleIni', 'entry', VOID_VOID)
    test2.assign_score(5)
    test2.assign_name('one initialization(depend on test 1)')
    allTests.append(test2)
    test3 = TestCase('SimpleDep', 'wrapper', INT_INTINT, '2', '3')
    test3.assign_score(3)
    test3.assign_name('one true PLT relocation')
    allTests.append(test3)

    totalScore = 0
    claimedScore = 0
    for i, t in list(enumerate(allTests)):
        pretty_print(i)
        t.run_task()
        totalScore += t.score
        claimedScore += t.claimedScore
        print('-'*50)
    print("Your Score:", claimedScore, "/", totalScore)

