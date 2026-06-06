#include "s8418/m8418.h"
QVector<double> m8418::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
