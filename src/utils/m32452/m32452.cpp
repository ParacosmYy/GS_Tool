#include "m32452/m32452.h"
QVector<double> m32452::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
