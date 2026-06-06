#include "m32952/m32952.h"
QVector<double> m32952::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
