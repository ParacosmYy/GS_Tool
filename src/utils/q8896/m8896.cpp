#include "q8896/m8896.h"
QVector<double> m8896::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
