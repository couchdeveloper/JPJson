#!/bin/bash

#  RunAllTests.sh
#  
#
#  Created by Andreas Grosam on 11/3/11.
#  Copyright (c) 2011 __MyCompanyName__. All rights reserved.

tmpFile=${CONFIGURATION_TEMP_DIR}/run_test_script.sh
echo "#!/bin/bash" > ${tmpFile}
echo "cd ${BUILT_PRODUCTS_DIR}" >> ${tmpFile}
echo "./AllTests" >> ${tmpFile}
chmod +x ${tmpFile}
open -a Terminal.app ${tmpFile}
