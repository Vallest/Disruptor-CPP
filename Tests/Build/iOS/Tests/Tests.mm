#include "Tests.h"
#include "UnitTesting.h"

#include <dispatch/dispatch.h>

#include <string>
#include <fstream>

#include <stdio.h>
#include <stdarg.h>

UITextView* g_tv;

void runTestCasesAsync(UITextView* tv)
{
    g_tv = tv;

    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^(void){
        runTestCases();
    });
}

class StreamBuf : public std::streambuf {
protected:
    int_type overflow(int_type ch)
    {
        buf_.push_back(traits_type::to_char_type(ch));
        
        if (ch == traits_type::to_int_type('\n'))
        {
            NSString* str = [NSString stringWithCString:buf_.c_str() encoding:[NSString defaultCStringEncoding]];

            dispatch_async(dispatch_get_main_queue(), ^(void){
                g_tv.text = [g_tv.text stringByAppendingString:str];
                NSRange bottom = NSMakeRange(g_tv.text.length -1, 1);
                [g_tv scrollRangeToVisible:bottom];
            });

            buf_.clear();
        }

        return ch;
    }

private:
    std::string buf_;
};

std::ostream& getOutStream()
{
    static StreamBuf buf;
    static std::ostream os(&buf);

    return os;
}

