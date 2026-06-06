#include "o32314/m32314.h"
QVector<double> m32314::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
