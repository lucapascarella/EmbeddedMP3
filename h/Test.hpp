/* 
 * File:   test.hpp
 * Author: Luca
 *
 * Created on 1 marzo 2017, 10.17
 */

#ifndef TEST_HPP
#define	TEST_HPP

class Test {
private:
    int gx;
    int gy;

public:
    Test(int x, int y);
    int testIt(void);
    void callException(int b);
    
private:
    void testThrow(int a);

};


#endif	/* TEST_HPP */

