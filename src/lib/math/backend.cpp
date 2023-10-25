#include "backend.h"

namespace lbcrypto {

    std::string uint128ToString(const ui128& value)
    {
        static const char* charmap = "0123456789";
        std::string result;
        result.reserve( 40 ); // max. 40 digits possible ( uint64_t has 20) 
        ui128 helper = value;

        do {
            result += charmap[ helper % 10 ];
            helper /= 10;
        } while ( helper );
        std::reverse( result.begin(), result.end() );
        return result;
    }

}  // namespace lbcrypto ends