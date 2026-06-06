#include "o32254/m32254.h"
QVector<double> m32254::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
