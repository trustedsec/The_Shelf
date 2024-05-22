#ifndef STROBF
#define STROBF
#define STRING_XOR_KEY 0x47
template <unsigned int N>
struct obfuscator {

    char m_data[N] = { 0 };

    constexpr obfuscator(const char* data) {
        /*
         * Implement encryption algorithm here.
         * Here we have simple XOR algorithm.
         */
        for (unsigned int i = 0; i < N; i++) {
            m_data[i] = data[i] ^ STRING_XOR_KEY;
        }
    }

    void deobfoscate(unsigned char* des) const {
        int i = 0;
        do {
            des[i] = m_data[i] ^ STRING_XOR_KEY;
            i++;
        } while (des[i - 1]);
    }
};



#define OBF(str) \
    []() -> char* { \
        constexpr auto size = sizeof(str)/sizeof(str[0]); \
        constexpr auto obfuscated_str = obfuscator<size>(str); \
        static char original_string[size]; \
        obfuscated_str.deobfoscate((unsigned char *)original_string); \
        return original_string; \
    }()

#define OBF_LEN(str,size) \
    []() -> char* { \
        constexpr auto obfuscated_str = obfuscator<size>(str); \
        static char original_string[size]; \
        obfuscated_str.deobfoscate((unsigned char *)original_string); \
        return original_string; \
    }()
#endif
