#include "q8636/m8636.h"
QVector<double> m8636::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
