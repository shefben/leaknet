#ifndef INFO_H
#define INFO_H
#ifdef _WIN32
#pragma once
#endif

// userinfo functions
const char *Info_ValueForKey ( const char *s, const char *key );
void Info_RemoveKey ( char *s, const char *key );
void Info_RemovePrefixedKeys ( char *start, char prefix );

#endif // INFO_H
