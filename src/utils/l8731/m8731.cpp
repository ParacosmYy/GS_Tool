#include "l8731/m8731.h"
QVector<double> m8731::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
