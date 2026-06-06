#include "i21828/m21828.h"
QVector<double> m21828::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
