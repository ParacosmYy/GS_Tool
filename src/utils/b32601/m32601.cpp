#include "b32601/m32601.h"
QVector<double> m32601::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
