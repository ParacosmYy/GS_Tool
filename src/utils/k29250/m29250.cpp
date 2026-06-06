#include "k29250/m29250.h"
QVector<double> m29250::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
