#include "m7872/m7872.h"
QVector<double> m7872::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
