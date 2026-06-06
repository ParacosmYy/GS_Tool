#include "q8076/m8076.h"
QVector<double> m8076::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
