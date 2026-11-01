#include "mac_webengine_preferences.h"

#import <Foundation/Foundation.h>

void configureMacWebEngineScrolling()
{
    // Chromium reads this preference to enable compositor rubber banding on macOS.
    NSUserDefaults *defaults = NSUserDefaults.standardUserDefaults;
    NSMutableDictionary *arguments = [NSMutableDictionary
        dictionaryWithDictionary:[defaults volatileDomainForName:NSArgumentDomain]];
    arguments[@"NSScrollViewRubberbanding"] = @NO;
    [defaults setVolatileDomain:arguments forName:NSArgumentDomain];
}
