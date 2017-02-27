/* 
 * File:   SingleCharacterCommands.h
 * Author: Luca
 *
 * Created on 1 febbraio 2014, 19.40
 */

#ifndef SINGLECHARACTERCOMMANDS_H
#define	SINGLECHARACTERCOMMANDS_H

#ifdef	__cplusplus
extern "C" {
#endif

#define SCC_MODE                    1
#define SCC_ECHO_MODE               2


    BOOL InitSCC(BYTE mode);
    void SCCHandler();

#ifdef	__cplusplus
}
#endif

#endif	/* SINGLECHARACTERCOMMANDS_H */

