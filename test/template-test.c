#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

#define SUITE_NAME "Suite"

void test1(void) {
    CU_ASSERT((2+2) == 2);
}

void test2(void) {
    CU_ASSERT((3+3) == 6);
}

/* The suite initialization function.
 * Put any init functionality here.
 */
int init_suite1(void) {
    return 0; // 0 is SUCCESS
}

/* The suite cleanup function.
 * Any cleanup goes here
 */
int clean_suite1(void) {
    return 0; // 0 is SUCCESS
}

/* The main() function for setting up and running the tests.
 * Returns a CUE_SUCCESS on successful running, another
 * CUnit error code on failure.
 */
int main(int argc, char *argv[]) {
   CU_pSuite pSuite = NULL;

   /* initialize the CUnit test registry */
   if (CUE_SUCCESS != CU_initialize_registry()) {
      return CU_get_error();
   }

   /* add a suite to the registry */
   pSuite = CU_add_suite(SUITE_NAME, init_suite1, clean_suite1);
   if (NULL == pSuite) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* add the tests to the suite */
   /* NOTE - ORDER IS IMPORTANT - MUST TEST fread() AFTER fprintf() */
   if (
       (NULL == CU_add_test(pSuite, "test of test1()", test1)) ||
       (NULL == CU_add_test(pSuite, "test of test2()", test2)))
       {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* Run all tests using the CUnit Basic interface */
   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   CU_cleanup_registry();
   return CU_get_error();
}
