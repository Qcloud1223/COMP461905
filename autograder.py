#! /usr/bin/env python3
import subprocess
import signal

# C-style define
VOID_CHARP = '0'
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
    
    def run_task(self):
        argList = []
        argList.extend((self.customLdExe, self.soName, self.funcName, self.funcType))
        if self.extraArg:
            argList.extend(self.args)
        try:
            # print(argList)
            RetProc = subprocess.run(argList, check=True, capture_output=True, encoding='utf-8')
        except subprocess.CalledProcessError as e:
            if e.returncode == -signal.SIGSEGV:
                print("SIGSEGV received in custom loader. Maybe you want to debug it with gdb.")
            else:
                print("Oops, error in custom loader.")
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
            except subprocess.CalledProcessError as e:
                if e.returncode == -signal.SIGSEGV:
                    print("SIGSEGV received while dlopen. Please contact the TA.")
                else:
                    print("Oops, error in dlopen.")
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


if __name__ == '__main__':
    test1 = TestCase('SimpleMul', 'multiply', INT_INTINT, '2', '3')
    test1.assign_score(100)
    test1.run_task()
    print("Your Score:", test1.claimedScore, "/ 100")

