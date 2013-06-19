/* test_sums.c - unit tests and benchmark for LibRHash algorithms
 * written by Alexei Kravchenko.
 *
 * Copyleft:
 * I, the author, hereby release this code into the public domain.
 * This applies worldwide. I grant any entity the right to use this work for
 * ANY PURPOSE, without any conditions, unless such conditions are required
 * by law.
 */

#include "unistd.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include "byte_order.h"
#include "timing.h"

#ifdef USE_RHASH_DLL
# define RHASH_API __declspec(dllimport)
#endif
#include "rhash.h"
#include "test_sums.h"

/*=======================================================================*
 *                         Data for tests                                *
 *=======================================================================*/
const char* crc32_tests[] = { /* verified with cksfv */
  "", "00000000",
  "a", "E8B7BE43",
  "abc", "352441C2",
  "message digest", "20159D7F",
  "abcdefghijklmnopqrstuvwxyz", "4C2750BD",
  "The quick brown fox jumps over the lazy dog", "414FA339",
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", "1FC2E6D2",
  "12345678901234567890123456789012345678901234567890123456789012345678901234567890", "7CA94A72",
  0
};

const char* md4_tests[] = {
  "", "31D6CFE0D16AE931B73C59D7E0C089C0",
  "a", "BDE52CB31DE33E46245E05FBDBD6FB24",
  "abc", "A448017AAF21D8525FC10AE87AA6729D",
  "message digest", "D9130A8164549FE818874806E1C7014B",
  "abcdefghijklmnopqrstuvwxyz", "D79E1C308AA5BBCDEEA8ED63DF412DA9",
  "The quick brown fox jumps over the lazy dog", "1BEE69A46BA811185C194762ABAEAE90",
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", "043F8582F241DB351CE627E153E7F0E4",
  "12345678901234567890123456789012345678901234567890123456789012345678901234567890", "E33B4DDC9C38F2199C3E7B164FCC0536",
  0
};

/* for short strings ed2k hashes are equal to md4 */
#define ed2k_tests md4_tests

/* test vectors from spec */
const char* md5_tests[] = {
  "", "D41D8CD98F00B204E9800998ECF8427E",
  "a", "0CC175B9C0F1B6A831C399E269772661",
  "abc", "900150983CD24FB0D6963F7D28E17F72",
  "message digest", "F96B697D7CB7938D525A2F31AAF161D0",
  "abcdefghijklmnopqrstuvwxyz", "C3FCD3D76192E4007DFB496CCA67E13B",
  "The quick brown fox jumps over the lazy dog", "9E107D9D372BB6826BD81D3542A419D6",
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", "D174AB98D277D9F5A5611C2C9F419D9F",
  "12345678901234567890123456789012345678901234567890123456789012345678901234567890", "57EDF4A22BE3C955AC49DA2E2107B67A",
  0
};

/* test vectors from spec */
const char* sha1_tests[] = {
  "", "DA39A3EE5E6B4B0D3255BFEF95601890AFD80709",
  "a", "86F7E437FAA5A7FCE15D1DDCB9EAEAEA377667B8",
  "abc", "A9993E364706816ABA3E25717850C26C9CD0D89D",
  "message digest", "C12252CEDA8BE8994D5FA0290A47231C1D16AAE3",
  "The quick brown fox jumps over the lazy dog", "2FD4E1C67A2D28FCED849EE1BB76E7391B93EB12",
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", "761C457BF73B14D27E9E9265C46F4B4DDA11F940",
  "12345678901234567890123456789012345678901234567890123456789012345678901234567890", "50ABF5706A150990A08B2C5EA40FA0E585554732",
  0
};

/* tests from spec and NESSIE test vectors */
const char* tiger_hashes[] = {
  "", "3293AC630C13F0245F92BBB1766E16167A4E58492DDE73F3",
  "a", "77BEFBEF2E7EF8AB2EC8F93BF587A7FC613E247F5F247809",
  "abc", "2AAB1484E8C158F2BFB8C5FF41B57A525129131C957B5F93",
  "Tiger", "DD00230799F5009FEC6DEBC838BB6A27DF2B9D6F110C7937",
  "The quick brown fox jumps over the lazy dog", "6D12A41E72E644F017B6F0E2F7B44C6285F06DD5D2C5B075",
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+-", "F71C8583902AFB879EDFE610F82C0D4786A3A534504486B5",
  "ABCDEFGHIJKLMNOPQRSTUVWXYZ=abcdefghijklmnopqrstuvwxyz+0123456789", "48CEEB6308B87D46E95D656112CDF18D97915F9765658957",
  "Tiger - A Fast New Hash Function, by Ross Anderson and Eli Biham", "8A866829040A410C729AD23F5ADA711603B3CDD357E4C15E",
  "Tiger - A Fast New Hash Function, by Ross Anderson and Eli Biham, proceedings of Fast Software Encryption 3, Cambridge.", "CE55A6AFD591F5EBAC547FF84F89227F9331DAB0B611C889",
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+-ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+-", "C54034E5B43EB8005848A7E0AE6AAC76E4FF590AE715FD25",
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", "8DCEA680A17583EE502BA38A3C368651890FFBCCDC49A8CC",

  "message digest", "D981F8CB78201A950DCF3048751E441C517FCA1AA55A29F6",
  "abcdefghijklmnopqrstuvwxyz", "1714A472EEE57D30040412BFCC55032A0B11602FF37BEEE9",
  "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", "0F7BF9A19B9C58F2B7610DF7E84F0AC3A71C631E7B53F78E",
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", "8DCEA680A17583EE502BA38A3C368651890FFBCCDC49A8CC",
  "12345678901234567890123456789012345678901234567890123456789012345678901234567890", "1C14795529FD9F207A958F84C52F11E887FA0CABDFD91BFD",
  0
};

/* verified by strong dc++ */
const char* tth_tests[] = {
  "", "LWPNACQDBZRYXW3VHJVCJ64QBZNGHOHHHZWCLNQ",
  "a", "CZQUWH3IYXBF5L3BGYUGZHASSMXU647IP2IKE4Y",
  "abc", "ASD4UJSEH5M47PDYB46KBTSQTSGDKLBHYXOMUIA",
  "message digest", "YM432MSOX5QILIH2L4TNO62E3O35WYGWSBSJOBA",
  "abcdefghijklmnopqrstuvwxyz", "LMHNA2VYO465P2RDOGTR2CL6XKHZNI2X4CCUY5Y",
  "The quick brown fox jumps over the lazy dog", "WLM2MITXFTCQXEOYO3M4EL5APES353NQLI66ORY",
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", "TF74ENF7MF2WPDE35M23NRSVKJIRKYRMTLWAHWQ",
  "12345678901234567890123456789012345678901234567890123456789012345678901234567890", "NBKCANQ2ODNTSV4C7YJFF3JRAV7LKTFIPHQNBJY",
  0
};

const char* aich_tests[] = {
  "", "3I42H3S6NNFQ2MSVX7XZKYAYSCX5QBYJ",
  "a", "Q336IN72UWT7ZYK5DXOLT2XK5I3XMZ5Y",
  "abc", "VGMT4NSHA2AWVOR6EVYXQUGCNSONBWE5",
  "message digest", "YERFFTW2RPUJSTK7UAUQURZDDQORNKXD",
  "abcdefghijklmnopqrstuvwxyz", "GLIQY64M7FSXBSQEZY37FIM5QQSA2OUJ",
  "The quick brown fox jumps over the lazy dog", "F7KODRT2FUUPZ3MET3Q3W5XHHENZH2YS",
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", "OYOEK67XHMKNE7U6SJS4I32LJXNBD6KA",
  "12345678901234567890123456789012345678901234567890123456789012345678901234567890", "KCV7K4DKCUEZBIELFRPKID5A4WCVKRZS",
  0
};

const char* whirlpool_tests[] = {
  "", "19FA61D75522A4669B44E39C1D2E1726C530232130D407F89AFEE0964997F7A73E83BE698B288FEBCF88E3E03C4F0757EA8964E59B63D93708B138CC42A66EB3",
  "a", "8ACA2602792AEC6F11A67206531FB7D7F0DFF59413145E6973C45001D0087B42D11BC645413AEFF63A42391A39145A591A92200D560195E53B478584FDAE231A",
  "abc", "4E2448A4C6F486BB16B6562C73B4020BF3043E3A731BCE721AE1B303D97E6D4C7181EEBDB6C57E277D0E34957114CBD6C797FC9D95D8B582D225292076D4EEF5",
  "message digest", "378C84A4126E2DC6E56DCC7458377AAC838D00032230F53CE1F5700C0FFB4D3B8421557659EF55C106B4B52AC5A4AAA692ED920052838F3362E86DBD37A8903E",
  "abcdefghijklmnopqrstuvwxyz", "F1D754662636FFE92C82EBB9212A484A8D38631EAD4238F5442EE13B8054E41B08BF2A9251C30B6A0B8AAE86177AB4A6F68F673E7207865D5D9819A3DBA4EB3B",
  "The quick brown fox jumps over the lazy dog", "B97DE512E91E3828B40D2B0FDCE9CEB3C4A71F9BEA8D88E75C4FA854DF36725FD2B52EB6544EDCACD6F8BEDDFEA403CB55AE31F03AD62A5EF54E42EE82C3FB35",
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
    "DC37E008CF9EE69BF11F00ED9ABA26901DD7C28CDEC066CC6AF42E40F82F3A1E08EBA26629129D8FB7CB57211B9281A65517CC879D7B962142C65F5A7AF01467",
  "12345678901234567890123456789012345678901234567890123456789012345678901234567890", 
    "466EF18BABB0154D25B9D38A6414F5C08784372BCCB204D6549C4AFADB6014294D5BD8DF2A6C44E538CD047B2681A51A2C60481E88C5A20B2C2A80CF3A9A083B",
  "abcdbcdecdefdefgefghfghighijhijk",
    "2A987EA40F917061F5D6F0A0E4644F488A7A5A52DEEE656207C562F988E95C6916BDC8031BC5BE1B7B947639FE050B56939BAAA0ADFF9AE6745B7B181C3BE3FD",
  0
};

/* test vectors from RIPEMD-160 spec */
const char* ripemd_tests[] = {
  "", "9C1185A5C5E9FC54612808977EE8F548B2258D31",
  "a", "0BDC9D2D256B3EE9DAAE347BE6F4DC835A467FFE",
  "abc", "8EB208F7E05D987A9B044A8E98C6B087F15A0BFC",
  "message digest", "5D0689EF49D2FAE572B881B123A85FFA21595F36",
  "abcdefghijklmnopqrstuvwxyz", "F71C27109C692C1B56BBDCEB5B9D2865B3708DBC",
  "The quick brown fox jumps over the lazy dog", "37F332F68DB77BD9D7EDD4969571AD671CF9DD3B",
  "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", "12A053384A9C0C88E405A06C27DCF49ADA62EB2B",
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", "B0E20B6E3116640286ED3A87A5713079B21F5189",
  "12345678901234567890123456789012345678901234567890123456789012345678901234567890", "9B752E45573D4B39F4DBD3323CAB82BF63326BFB",
  0
};

/*
 * Two important test-cases (some sites calculate them incorrectly):
 * GOST( <100000 characters of 'a'> ) = 5C00CCC2734CDD3332D3D4749576E3C1A7DBAF0E7EA74E9FA602413C90A129FA
 * GOST( <128 characters of 'U'> ) = 53A3A3ED25180CEF0C1D85A074273E551C25660A87062A52D926A9E8FE5733A4 
 */

/* test vectors from internet, verified by openssl and some other progs */
const char* gost_tests[] = {
  "", "CE85B99CC46752FFFEE35CAB9A7B0278ABB4C2D2055CFF685AF4912C49490F8D",
  "a", "D42C539E367C66E9C88A801F6649349C21871B4344C6A573F849FDCE62F314DD",
  "abc", "F3134348C44FB1B2A277729E2285EBB5CB5E0F29C975BC753B70497C06A4D51D",
  "message digest", "AD4434ECB18F2C99B60CBE59EC3D2469582B65273F48DE72DB2FDE16A4889A4D",
  "The quick brown fox jumps over the lazy dog", "77B7FA410C9AC58A25F49BCA7D0468C9296529315EACA76BD1A10F376D1F4294",
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", "95C1AF627C356496D80274330B2CFF6A10C67B5F597087202F94D06D2338CF8E",
  "12345678901234567890123456789012345678901234567890123456789012345678901234567890", "CC178DCAD4DF619DCAA00AAC79CA355C00144E4ADA2793D7BD9B3518EAD3CCD3",
  "UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "53A3A3ED25180CEF0C1D85A074273E551C25660A87062A52D926A9E8FE5733A4",
  /* two test strings from GOST standard */
  "This is message, length=32 bytes", "B1C466D37519B82E8319819FF32595E047A28CB6F83EFF1C6916A815A637FFFA",
  "Suppose the original message has length = 50 bytes", "471ABA57A60A770D3A76130635C1FBEA4EF14DE51F78B4AE57DD893B62F55208",
  /* tests from Wikipedia */
  "\303\316\321\322 \320 34.11-94", "30CFF16501E3D77D84F24C55284945F7297120C78BC6B3ED4F537B674735A8B6",
  "The quick brown fox jumps over the lazy cog", "A3EBC4DAAAB78B0BE131DAB5737A7F67E602670D543521319150D2E14EEEC445",
  0
};

/* tested with openssl */
const char* gost_cryptopro_tests[] = {
  "", "981E5F3CA30C841487830F84FB433E13AC1101569B9C13584AC483234CD656C0",
  "a", "E74C52DD282183BF37AF0079C9F78055715A103F17E3133CEFF1AACF2F403011",
  "abc", "B285056DBF18D7392D7677369524DD14747459ED8143997E163B2986F92FD42C",
  "message digest", "BC6041DD2AA401EBFA6E9886734174FEBDB4729AA972D60F549AC39B29721BA0",
  "The quick brown fox jumps over the lazy dog", "9004294A361A508C586FE53D1F1B02746765E71B765472786E4770D565830A76",
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", "73B70A39497DE53A6E08C67B6D4DB853540F03E9389299D9B0156EF7E85D0F61",
  "12345678901234567890123456789012345678901234567890123456789012345678901234567890", "6BC7B38989B28CF93AE8842BF9D752905910A7528A61E5BCE0782DE43E610C90",
  "UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "1C4AC7614691BBF427FA2316216BE8F10D92EDFD37CD1027514C1008F649C4E8",
  "This is message, length=32 bytes", "2CEFC2F7B7BDC514E18EA57FA74FF357E7FA17D652C75F69CB1BE7893EDE48EB",
  "Suppose the original message has length = 50 bytes", "C3730C5CBCCACF915AC292676F21E8BD4EF75331D9405E5F1A61DC3130A65011",
  0
};

/* test vectors verified by mhash */
const char* snefru256_tests[] = {
  "", "8617F366566A011837F4FB4BA5BEDEA2B892F3ED8B894023D16AE344B2BE5881",
  "a", "45161589AC317BE0CEBA70DB2573DDDA6E668A31984B39BF65E4B664B584C63D",
  "abc", "7D033205647A2AF3DC8339F6CB25643C33EBC622D32979C4B612B02C4903031B",
  "message digest", "C5D4CE38DAA043BDD59ED15DB577500C071B917C1A46CD7B4D30B44A44C86DF8",
  "abcdefghijklmnopqrstuvwxyz", "9304BB2F876D9C4F54546CF7EC59E0A006BEAD745F08C642F25A7C808E0BF86E",
  "The quick brown fox jumps over the lazy dog", "674CAA75F9D8FD2089856B95E93A4FB42FA6C8702F8980E11D97A142D76CB358",
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", "83AA9193B62FFD269FAA43D31E6AC2678B340E2A85849470328BE9773A9E5728",
  "12345678901234567890123456789012345678901234567890123456789012345678901234567890", "D5FCE38A152A2D9B83AB44C29306EE45AB0AED0E38C957EC431DAB6ED6BB71B8",
  0
};

/* test vectors verified by mhash */
const char* snefru128_tests[] = {
  "", "8617F366566A011837F4FB4BA5BEDEA2",
  "a", "BF5CE540AE51BC50399F96746C5A15BD",
  "abc", "553D0648928299A0F22A275A02C83B10",
  "message digest", "96D6F2F4112C4BAF29F653F1594E2D5D",
  "abcdefghijklmnopqrstuvwxyz", "7840148A66B91C219C36F127A0929606",
  "The quick brown fox jumps over the lazy dog", "59D9539D0DD96D635B5BDBD1395BB86C",
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", "0EFD7F93A549F023B79781090458923E",
  "12345678901234567890123456789012345678901234567890123456789012345678901234567890", "D9204ED80BB8430C0B9C244FE485814A",
  0
};

/* checked against test vectors: http://www.randombit.net/text/has160.html */
const char* has160_tests[] = {
  "",  "307964EF34151D37C8047ADEC7AB50F4FF89762D",
  "a", "4872BCBC4CD0F0A9DC7C2F7045E5B43B6C830DB8",
  "abc", "975E810488CF2A3D49838478124AFCE4B1C78804",
  "message digest", "2338DBC8638D31225F73086246BA529F96710BC6",
  "abcdefghijklmnopqrstuvwxyz", "596185C9AB6703D0D0DBB98702BC0F5729CD1D3C",
  "The quick brown fox jumps over the lazy dog", "ABE2B8C711F9E8579AA8EB40757A27B4EF14A7EA",
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", "CB5D7EFBCA2F02E0FB7167CABB123AF5795764E5",
  "12345678901234567890123456789012345678901234567890123456789012345678901234567890", "07F05C8C0773C55CA3A5A695CE6ACA4C438911B5",
  0
};

/* unconfirmed test vectors */
const char* sha224_tests[] = {
  "", "D14A028C2A3A2BC9476102BB288234C415A2B01F828EA62AC5B3E42F",
  "a", "ABD37534C7D9A2EFB9465DE931CD7055FFDB8879563AE98078D6D6D5",
  "abc", "23097D223405D8228642A477BDA255B32AADBCE4BDA0B3F7E36C9DA7",
  "message digest", "2CB21C83AE2F004DE7E81C3C7019CBCB65B71AB656B22D6D0C39B8EB",
  "abcdefghijklmnopqrstuvwxyz", "45A5F72C39C5CFF2522EB3429799E49E5F44B356EF926BCF390DCCC2",
  "The quick brown fox jumps over the lazy dog", "730E109BD7A8A32B1CB9D9A09AA2325D2430587DDBC0C38BAD911525",
  "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", "75388B16512776CC5DBA5DA1FD890150B0C6455CB4F58B1952522525",
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", "BFF72B4FCB7D75E5632900AC5F90D219E05E97A7BDE72E740DB393D9",
  "12345678901234567890123456789012345678901234567890123456789012345678901234567890", "B50AECBE4E9BB0B57BC5F3AE760A8E01DB24F203FB3CDCD13148046E",
  0
};

/* test vectors from the NESSIE project */
const char* sha256_tests[] = {
  "", "E3B0C44298FC1C149AFBF4C8996FB92427AE41E4649B934CA495991B7852B855",
  "a", "CA978112CA1BBDCAFAC231B39A23DC4DA786EFF8147C4E72B9807785AFEE48BB",
  "abc", "BA7816BF8F01CFEA414140DE5DAE2223B00361A396177A9CB410FF61F20015AD",
  "message digest", "F7846F55CF23E14EEBEAB5B4E1550CAD5B509E3348FBC4EFA3A1413D393CB650",
  "abcdefghijklmnopqrstuvwxyz", "71C480DF93D6AE2F1EFAD1447C66C9525E316218CF51FC8D9ED832F2DAF18B73",
  "The quick brown fox jumps over the lazy dog", "D7A8FBB307D7809469CA9ABCB0082E4F8D5651E46D3CDB762D02D0BF37C9E592",
  "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", "248D6A61D20638B8E5C026930C3E6039A33CE45964FF2167F6ECEDD419DB06C1",
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", "DB4BFCBD4DA0CD85A60C3C37D3FBD8805C77F15FC6B1FDFE614EE0A7C8FDB4C0",
  "12345678901234567890123456789012345678901234567890123456789012345678901234567890", "F371BC4A311F2B009EEF952DD83CA80E2B60026C8E935592D0F9C308453C813E",
  0
};

const char* sha384_tests[] = {
  "", "38B060A751AC96384CD9327EB1B1E36A21FDB71114BE07434C0CC7BF63F6E1DA274EDEBFE76F65FBD51AD2F14898B95B",
  "a", "54A59B9F22B0B80880D8427E548B7C23ABD873486E1F035DCE9CD697E85175033CAA88E6D57BC35EFAE0B5AFD3145F31",
  "abc", "CB00753F45A35E8BB5A03D699AC65007272C32AB0EDED1631A8B605A43FF5BED8086072BA1E7CC2358BAECA134C825A7",
  "message digest", "473ED35167EC1F5D8E550368A3DB39BE54639F828868E9454C239FC8B52E3C61DBD0D8B4DE1390C256DCBB5D5FD99CD5",
  "abcdefghijklmnopqrstuvwxyz", "FEB67349DF3DB6F5924815D6C3DC133F091809213731FE5C7B5F4999E463479FF2877F5F2936FA63BB43784B12F3EBB4",
  "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", "3391FDDDFC8DC7393707A65B1B4709397CF8B1D162AF05ABFE8F450DE5F36BC6B0455A8520BC4E6F5FE95B1FE3C8452B",
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", "1761336E3F7CBFE51DEB137F026F89E01A448E3B1FAFA64039C1464EE8732F11A5341A6F41E0C202294736ED64DB1A84",
  "12345678901234567890123456789012345678901234567890123456789012345678901234567890", "B12932B0627D1C060942F5447764155655BD4DA0C9AFA6DD9B9EF53129AF1B8FB0195996D2DE9CA0DF9D821FFEE67026",
  0
};

const char* sha512_tests[] = {
  "", "CF83E1357EEFB8BDF1542850D66D8007D620E4050B5715DC83F4A921D36CE9CE47D0D13C5D85F2B0FF8318D2877EEC2F63B931BD47417A81A538327AF927DA3E",
  "a", "1F40FC92DA241694750979EE6CF582F2D5D7D28E18335DE05ABC54D0560E0F5302860C652BF08D560252AA5E74210546F369FBBBCE8C12CFC7957B2652FE9A75",
  "abc", "DDAF35A193617ABACC417349AE20413112E6FA4E89A97EA20A9EEEE64B55D39A2192992A274FC1A836BA3C23A3FEEBBD454D4423643CE80E2A9AC94FA54CA49F",
  "message digest", "107DBF389D9E9F71A3A95F6C055B9251BC5268C2BE16D6C13492EA45B0199F3309E16455AB1E96118E8A905D5597B72038DDB372A89826046DE66687BB420E7C",
  "abcdefghijklmnopqrstuvwxyz", "4DBFF86CC2CA1BAE1E16468A05CB9881C97F1753BCE3619034898FAA1AABE429955A1BF8EC483D7421FE3C1646613A59ED5441FB0F321389F77F48A879C7B1F1",
  "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", "204A8FC6DDA82F0A0CED7BEB8E08A41657C16EF468B228A8279BE331A703C33596FD15C13B1B07F9AA1D3BEA57789CA031AD85C7A71DD70354EC631238CA3445",
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", "1E07BE23C26A86EA37EA810C8EC7809352515A970E9253C26F536CFC7A9996C45C8370583E0A78FA4A90041D71A4CEAB7423F19C71B9D5A3E01249F0BEBD5894",
  "12345678901234567890123456789012345678901234567890123456789012345678901234567890", "72EC1EF1124A45B047E8B7C75A932195135BB61DE24EC0D1914042246E0AEC3A2354E093D76F3048B456764346900CB130D2A4FD5DD16ABB5E30BCB850DEE843",
  0
};

/* verified by eBASH SUPERCOP implementation */
const char* edonr256_tests[] = {
  "", "86E7C84024C55DBDC9339B395C95E88DB8F781719851AD1D237C6E6A8E370B80",
  "a", "943AA9225A2CF154EC2E4DD81237720BA538CA8DF2FD83C0B893C5D265F353A0",
  "abc", "0360F65D97C2152EA6EBE3D462BF49831E2D5F67B6140992320585D89FD271CE",
  "message digest", "8D27558F4DD9307614A8166CADB136927D1E79A0C04BD8EF77C3FAFC0917E28A",
  "abcdefghijklmnopqrstuvwxyz", "5415737AF0D827459EFACB7FE33C0E89CF807E6E608A4D70EF9DEB07BF3BF6BF",
  "The quick brown fox jumps over the lazy dog", "E77A5AC00923B86C1811D42F1CB1198F43412A6D987DC98BDAE11E6D91399609",
  "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", "19DE86BC3F0481098A3E623AA1330995043300A9A5D6C2AD584705F62686417F",
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", "3B57F954420F49FAC6A80CE6CE013FDB47E71CE824DA78A8F66864203D8EF252",
  "12345678901234567890123456789012345678901234567890123456789012345678901234567890", "286F39D5168775C8E541ED2F0FE3ECF3146380B9C479DE41BD847E866420A776",
  0
};

/* verified by eBASH SUPERCOP implementation */
const char* edonr512_tests[] = {
  "", "C7AFBDF3E5B4590EB0B25000BF83FB16D4F9B722EE7F9A2DC2BD382035E8EE38D6F6F15C7B8EEC85355AC59AF989799950C64557EAB0E687D0FCBDBA90AE9704",
  "a", "B59EC44F7BEEF8A04CEED38A973D77C65E22E9458D5F67B497948DA34986C093B5EFC5483FBEE55F2F740FCAD31F18D80DB44BB6B8843E7FD599188E7C07233B",
  "abc", "FE79BCFA310245D9139DA8BC91B99FD022326F7F3ACA1DFDFB6C84E4125D71FE9BB6A1D41AFCE358F8472835220A7829D5146B2BBFC8E5C2627F60A9B517C1A4",
  "message digest", "A76B6C5CA8778F39EC1F85D64BADBDBF329725C9A6FB92656D94A82922A26FD51D271A6F135F33157143B960CD8D7D20DC99503AA39871FD64050E061689E4E3",
  "abcdefghijklmnopqrstuvwxyz", "754640B7B01782C1F345A3864B456DB805E39163FA1A06113A37CB8FB18D30F8DC43C7C3FDB407849CAD437C90DBD28E28AEFEF8898589B388ADEBA153B3DE0B",
  "The quick brown fox jumps over the lazy dog", "B986ADABFA9ADB1E5B152B6D64C733389082E354FDE2FD9740FAEA6766F440EA4391FC745BB9B11A821756944077BB30723F616645492C70FA4C614DB7E9D45B",
  "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", "EE5EF974E8677636970A50E7636EC34EFB1F9D8023C715A26747D73D3665D78D2BB4962381901F76892A630133D476A278E4E3C62176FCE1563904636284415B",
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", "0755F846450A6F84001846E1066828727BF5975383867B87E0120F27B79482524EB01137459185F73C24C23BDD9D901AD1577C3EA1A824E6ACE34BBBA119E92F",
  "12345678901234567890123456789012345678901234567890123456789012345678901234567890", "0998912DA5B13FC5D7332CBC3B240E44547CE9C861867D901DD39D5A43D2EE80686BC4AD70DFF9159FE12CE94255AD5467B2B59D31562FC08B3697B67323075F",
  0
};

/* test vector verified with uTorrent */
const char* btih_tests[] = {
  "", "A66A2FA70401A9DCA25C7C39937921D377A24C1A",
  0
};

typedef struct known_strings_t {
  unsigned hash_id;
  const char** tests;
} known_strings_t;

known_strings_t known_strings[] = {
  { RHASH_CRC32, crc32_tests },
  { RHASH_MD4, md4_tests },
  { RHASH_MD5, md5_tests },
  { RHASH_SHA1, sha1_tests },
  { RHASH_TIGER, tiger_hashes },
  { RHASH_TTH, tth_tests },
  { RHASH_BTIH, btih_tests },
  { RHASH_ED2K, ed2k_tests },
  { RHASH_AICH, aich_tests },
  { RHASH_WHIRLPOOL, whirlpool_tests },
  { RHASH_RIPEMD160, ripemd_tests },
  { RHASH_GOST, gost_tests },
  { RHASH_GOST_CRYPTOPRO, gost_cryptopro_tests },
  { RHASH_HAS160, has160_tests },
  { RHASH_SNEFRU128, snefru128_tests },
  { RHASH_SNEFRU256, snefru256_tests },
  { RHASH_SHA224, sha224_tests },
  { RHASH_SHA256, sha256_tests },
  { RHASH_SHA384, sha384_tests },
  { RHASH_SHA512, sha512_tests },
  { RHASH_EDONR256, edonr256_tests },
  { RHASH_EDONR512, edonr512_tests },
  { 0, 0 }
};

/*==========================================================================*
 *                    Helper functions to hash messages                     *
 *==========================================================================*/

/**
 * Calculate hash of the message specified by chunk string, repeated until
 * the given length is reached.
 *
 * @param chunk a null-terminated string representing the chunk
 * @param msg_size the total message length
 * @param length the total length of the message
 * @param hash_id id of the hash algorithm to use
 */
static char* calc_sums_c(const char* chunk, size_t msg_size, size_t length, unsigned hash_id)
{
  struct rhash_context *ctx;
  static char out[130];
  size_t i;

  ctx = rhash_init(hash_id);
  for(i = 0; i < length; i += msg_size) {
    rhash_update(ctx, (const unsigned char*)chunk,
      ((i + msg_size) <= length ? msg_size : length % msg_size));
  }
  rhash_final(ctx, hash_id, 0);
  rhash_print(out, ctx, hash_id, RHPR_UPPERCASE);
  rhash_free(ctx);
  return out;
}

/**
 * Calculate hash of the given message.
 *
 * @param msg the message to hash
 * @param hash_id id of the hash algorithm to use
 */
static char* calc_sum(const char* msg, unsigned hash_id)
{
  return calc_sums_c(msg, strlen(msg), strlen(msg), hash_id);
}

static int n_errors = 0;  /* total number of errors occured */

#ifdef UNDER_CE /* if Windows CE */
static char *g_msg = NULL; /* string buffer to store errors */
#endif

/**
 * Verify processor endianness detected at compile-time against
 * with the actual cpu endianness in runtime.
 */
static void test_endianness(void)
{
  unsigned tmp = 1;
  if(*(char*)&tmp == IS_BIG_ENDIAN) {
    n_errors++;
#ifndef UNDER_CE
    printf("error: wrong endianness detected at compile time\n");
    fflush(stdout);
#else /* UNDER_CE */
    char* str = "error: wrong endianness detected at compile time";
    int add_nl = (g_msg == NULL);
    g_msg = (char*)realloc(g_msg, (g_msg ? strlen(g_msg) : 0) + strlen(str) + 3);
    if(add_nl) strcat(g_msg, "\r\n");
    strcat(g_msg, str);
#endif /* UNDER_CE */
  }
}

/**
 * Verify calculated hash value against expected one. If failed report
 * the comparision error in the platform-dependent manner.
 *
 * @param obtained calculated hash
 * @param expected expected value
 * @param name hash algorithm name
 * @param msg hashed message
 */
static int assert_equals(const char* obtained, const char* expected, const char* name, const char* msg)
{
  int success = (strcmp(obtained, expected) == 0);
  if(!success) {
#ifndef UNDER_CE
    printf("error: %-5s (\"%s\") = %s, expected: \"%s\"\n", name, msg, obtained, expected);
    fflush(stdout);

#else /* UNDER_CE */

    char str[100];
    int add_nl = (g_msg == NULL);

    sprintf(str, "error: %-5s (\"%s\") = %s, expected: \"%s\"\n", name, msg, obtained, expected);
    g_msg = (char*)realloc(g_msg, (g_msg ? strlen(g_msg) : 0) + strlen(str) + 3);
    if(add_nl) strcat(g_msg, "\r\n");
    strcat(g_msg, str);
#endif /* UNDER_CE */
    n_errors++;
  }
  return success;
}

/**
 * Test a hash algorithm on given message by comparing calculated result with
 * expected one. Report error on fail.
 *
 * @param message the message to hash
 * @param expected_hash the expected hash balue
 * @param hash_id id of the algorithm to test
 */
static void test_str(const char* message, const char* expected_hash, unsigned hash_id)
{
  char* obtained = calc_sum(message, hash_id);
  assert_equals(obtained, expected_hash, rhash_get_name(hash_id), message);
}

/**
 * Test a hash algorithm against a message of given length and consisting
 * of repeated chunks.
 * Report error if calculated hash doesn't coincide with expected value.
 *
 * @param hash_id id of the algorithm to test
 * @param chunk the message chunk as a null-terminated string
 * @param size the length of message of zeroes in bytes
 * @param expected_hash the expected hash balue
 */
static void test_long_msg(unsigned hash_id, const char* chunk, size_t size, const char* expected_hash)
{
  size_t chunk_length = strlen(chunk);
  char* obtained = calc_sums_c(chunk, chunk_length, size, hash_id);
  assert_equals(obtained, expected_hash, rhash_get_name(hash_id), chunk);
}

/**
 * Test a hash algorithm against a long messages of zeroes.
 * Report error if calculated hash doesn't coincide with expected value.
 *
 * @param hash_id id of the algorithm to test
 * @param size the length of message of zeroes in bytes
 * @param expected_hash the expected hash balue
 */
static void test_long_zero_msg(unsigned hash_id, size_t size, const char* expected_hash)
{
  char buffer[8192];
  char msg[80], *obtained;
  memset(buffer, 0, 8192);
  sprintf(msg, "\"\\0\"x%u", (unsigned)size);
  obtained = calc_sums_c(buffer, 8192, size, hash_id);
  assert_equals(obtained, expected_hash, rhash_get_name(hash_id), msg);
}

/*=======================================================================*
 *                            Test functions                             *
 *=======================================================================*/

/**
 * Test a hash algorithm on array of known short messages.
 *
 * @param hash_id id of the algorithm to test
 * @param ptr pointer to array of pairs <message,expected-hash>
 */
static void test_known_strings2(unsigned hash_id, const char** ptr)
{
  for(; ptr[0] && ptr[1]; ptr += 2) {
    test_str(ptr[0], ptr[1], hash_id);
  }
}

/**
 * Test a hash algorithm on known short messages.
 *
 * @param hash_id id of the algorithm to test
 */
static void test_known_strings(unsigned hash_id)
{
  int i;
  for(i = 0; known_strings[i].tests != 0; i++) {
    if(hash_id == known_strings[i].hash_id) {
      test_known_strings2(hash_id, known_strings[i].tests);
      break;
    }
  }
}

/**
 * Verify hash algorithms by testing them against known short messages.
 */
static void test_all_known_strings(void)
{
  int i;
  for(i = 0; known_strings[i].tests != 0; i++) {
    assert(i < (int)(sizeof(known_strings)/sizeof(known_strings_t)));
    test_known_strings2(known_strings[i].hash_id, known_strings[i].tests);
  }
}

/**
 * A pair <algorithm-id, expected-hash-value>.
 */
typedef struct id_to_hash_t {
  int hash_id;
  const char* hash;
} id_to_hash_t;

/**
 * Verify hash algorithms by testing them on long messages, like
 * 1,000,000 charaters of 'a'.
 */
static void test_long_strings(void)
{
  unsigned count;
  char* message  = "aaaaaaaaaaaaa"; /* a message with a prime-numbered length */

  struct id_to_hash_t tests[] = {
    { RHASH_CRC32, "DC25BFBC" }, /* verified with cksfv */
    { RHASH_MD4, "BBCE80CC6BB65E5C6745E30D4EECA9A4" }, /* checked by md4sum */
    { RHASH_MD5, "7707D6AE4E027C70EEA2A935C2296F21" }, /* checked by md5sum */
    { RHASH_SHA1, "34AA973CD4C4DAA4F61EEB2BDBAD27316534016F" }, /* checked by sha1sum */
    { RHASH_ED2K, "BBCE80CC6BB65E5C6745E30D4EECA9A4" }, /* checked by eMule' Link Creator (uses eMule algorithm) */
    { RHASH_AICH, "KSYPATEV3KP26FJYUEEBCPL5LQJ5FGUK" }, /* checked by eMule' Link Creator */
    { RHASH_TIGER, "6DB0E2729CBEAD93D715C6A7D36302E9B3CEE0D2BC314B41" }, /* from Tiger author's page (NESSIE test vector) */
    { RHASH_TTH, "KEPTIGT4CQKF7S5EUVNJZSXXIPNMB3XSOAAQS4Y" }, /* verified with Strong DC++ */
    { RHASH_WHIRLPOOL, "0C99005BEB57EFF50A7CF005560DDF5D29057FD86B20BFD62DECA0F1CCEA4AF51FC15490EDDC47AF32BB2B66C34FF9AD8C6008AD677F77126953B226E4ED8B01" }, /* taken from the algorithm reference */
    { RHASH_RIPEMD160, "52783243C1697BDBE16D37F97F68F08325DC1528" }, /* taken from the algorithm reference */
    { RHASH_GOST_CRYPTOPRO, "8693287AA62F9478F7CB312EC0866B6C4E4A0F11160441E8F4FFCD2715DD554F" }, /* verified with openssl */
    { RHASH_GOST, "5C00CCC2734CDD3332D3D4749576E3C1A7DBAF0E7EA74E9FA602413C90A129FA" }, /* verified with openssl */
    { RHASH_HAS160, "D6AD6F0608B878DA9B87999C2525CC84F4C9F18D" }, /* verified against jacksum implementation */
    { RHASH_SNEFRU128, "5071F647BC51CFD48F9A8F2D2ED84829" }, /* verified by mhash */
    { RHASH_SNEFRU256, "4A02811F28C121F2162ABB251A01A2A58E6CFC27534AAB10EA6AF0A8DF17FFBF" }, /* verified by mhash */
    { RHASH_SHA224, "20794655980C91D8BBB4C1EA97618A4BF03F42581948B2EE4EE7AD67" }, /* verified against jacksum implementation */
    { RHASH_SHA256, "CDC76E5C9914FB9281A1C7E284D73E67F1809A48A497200E046D39CCC7112CD0" }, /* from NESSIE test vectors */
    { RHASH_SHA384, "9D0E1809716474CB086E834E310A4A1CED149E9C00F248527972CEC5704C2A5B07B8B3DC38ECC4EBAE97DDD87F3D8985" }, /* from NESSIE test vectors */
    { RHASH_SHA512, "E718483D0CE769644E2E42C7BC15B4638E1F98B13B2044285632A803AFA973EBDE0FF244877EA60A4CB0432CE577C31BEB009C5C2C49AA2E4EADB217AD8CC09B" }, /* from NESSIE test vectors */
    { RHASH_EDONR256, "56F4B8DC0A41C8EA0A6A42C949883CD5DC25DF8CF4E43AD474FD4492A7A07966" }, /* verified by eBASH SUPERCOP implementation */
    { RHASH_EDONR512, "B4A5A255D67869C990FE79B5FCBDA69958794B8003F01FD11E90FEFEC35F22BD84FFA2E248E8B3C1ACD9B7EFAC5BC66616E234A6E938D3526DEE26BD0DE9C562" }, /* verified by eBASH SUPERCOP implementation */

    { RHASH_BTIH, "30CF71DEFC48D497D4C6DCA8FAB203C1E253A53F" }, /* not verified */
  };

  /* test all algorithms on 1,000,000 charaters of 'a' */
  for(count = 0; count < (sizeof(tests) / sizeof(id_to_hash_t)); count++) {
    test_long_msg(tests[count].hash_id, message, 1000000, tests[count].hash);
  }

  /* note: it would be better to check with more complex pre-generated messages */

  /* these messages verified by eMule LinkCreator (which uses eMule variant of ED2K hash) */
  test_long_zero_msg(RHASH_ED2K, 9728000, "FC21D9AF828F92A8DF64BEAC3357425D");
  test_long_zero_msg(RHASH_ED2K, 9728000 - 1, "AC44B93FC9AFF773AB0005C911F8396F");
  test_long_zero_msg(RHASH_ED2K, 9728000 + 1, "06329E9DBA1373512C06386FE29E3C65"); /* msg with: 9728000 < size <= 9732096 */
  test_long_zero_msg(RHASH_AICH, 9728000, "5D3N4HQHIUMQ7IU7A5QLPLI6RHSWOR7B");
  test_long_zero_msg(RHASH_AICH, 9728000 - 1, "L6SPMD2CM6PRZBGRQ6UFC4HJFFOATRA4");
  test_long_zero_msg(RHASH_AICH, 9728000 + 1, "HL3TFXORIUEPXUWFPY3JLR7SMKGTO4IH");
#if 0
  test_long_zero_msg(RHASH_ED2K, 9728000 * 5, "3B613901DABA54F6C0671793E28A1205");
  test_long_zero_msg(RHASH_AICH, 9728000 * 5, "EZCO3XF2RJ4FERRDEXGOSSRGL5NA5BBM");
#endif
}

/**
 * Verify that calculated hash doesn't depend on message alignment.
 */
static void test_alignment(void)
{
  int i, start, hash_id, alignment_size;

  /* loop by sums */
  for(i = 0, hash_id = 1; (hash_id & RHASH_ALL_HASHES); hash_id <<= 1, i++) {
    char expected_hash[130];
    assert(rhash_get_digest_size(hash_id) < (int)sizeof(expected_hash));

    alignment_size = (hash_id & (RHASH_TTH | RHASH_TIGER | RHASH_WHIRLPOOL | RHASH_SHA512) ? 8 : 4);

    /* start message with different alignment */
    for(start = 0; start < alignment_size; start++) {
      char message[30];
      char* obtained;
      int j, msg_length = 11 + alignment_size;

      /* fill the buffer fifth shifted letter sequence */
      for(j = 0; j < msg_length; j++) message[start + j] = 'a' + j;
      message[start + j] = 0;

      obtained = calc_sum(message + start, hash_id);

      if(start == 0) {
        /* save original sum */
        strcpy(expected_hash, obtained);
      } else {
        /* verify sum result */
        assert_equals(obtained, expected_hash, rhash_get_name(hash_id), message);
        fflush(stdout);
      }
    }
  }
}

/**
 * Find hash id by its name.
 *
 * @param name hash algorithm name
 * @return algorithm id
 */
static unsigned find_hash(const char* name)
{
  char buf[30];
  unsigned hash_id;
  int i;

  if(strlen(name) > (sizeof(buf) - 1)) return 0;
  for(i = 0; name[i]; i++) buf[i] = toupper(name[i]);
  buf[i] = 0;

  for(hash_id = 1; (hash_id & RHASH_ALL_HASHES); hash_id <<= 1) {
    if(strcmp(buf, rhash_get_name(hash_id)) == 0) return hash_id;
  }
  return 0;
}

/* define program entry point */

#ifndef UNDER_CE /* if not Windows CE */

/**
 * The application entry point under linux and windows.
 *
 * @param argc number of arguments including the program name
 * @param argv program arguments including the program name
 */
int main(int argc, char *argv[])
{
  /* rhash_transmit(RMSG_SET_OPENSSL_MASK, 0, RHASH_ALL_HASHES, 0); */

#ifndef USE_RHASH_DLL
  rhash_library_init();
#endif

  test_endianness();

  if(argc > 1 && strcmp(argv[1], "--speed") == 0) {
    unsigned hash_id = (argc > 2 ? find_hash(argv[2]) : RHASH_SHA1);
    if(hash_id == 0) {
      fprintf(stderr, "error: unknown hash_id: %s\n", argv[2]);
      return 1;
    }
    test_known_strings(hash_id);

    run_benchmark(hash_id, 0, stdout);
  } else if(argc > 1 && strcmp(argv[1], "--flags") == 0) {
    printf("%s", compiler_flags);
  } else {
    test_all_known_strings();
    test_long_strings();
    test_alignment();
    if(n_errors == 0) printf("All sums are working properly!\n");
    fflush(stdout);
  }

  if(n_errors > 0) printf("%s", compiler_flags);

  return (n_errors == 0 ? 0 : 1);
}
#else /* UNDER_CE */

#include <windows.h>
#include <commctrl.h>

/**
 * Convert a single-byte string to a two-byte (wchar_t*).
 *
 * @param str the string to convert
 * @return converted string of wchar_t
 */
wchar_t *char2wchar(char* str)
{
  size_t origsize;
  wchar_t *wcstring;

  origsize = strlen(str) + 1;
  wcstring = (wchar_t*)malloc(origsize * 2);
  mbstowcs(wcstring, str, origsize);
  return wcstring;
}

/**
 * The program entry point under Windows CE
 *
 * @param argc number of arguments including program name
 * @param argv program argumants including its name
 */
int _tmain(int argc, _TCHAR* argv[])
{
  wchar_t *wcstring;
  (void)argc;
  (void)argv;

  test_known_strings();
  test_alignment();

  wcstring = char2wchar(g_msg ? g_msg : "Success!\r\nAll sums are working properly.");
  MessageBox(NULL, wcstring, _T("caption"), MB_OK|MB_ICONEXCLAMATION);
  free(wcstring);

  return (n_errors == 0 ? 0 : 1);
}
#endif /* UNDER_CE */
