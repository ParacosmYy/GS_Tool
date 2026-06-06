#include "m32132/m32132.h"
QVector<double> m32132::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
