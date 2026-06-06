#include "m32872/m32872.h"
QVector<double> m32872::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
