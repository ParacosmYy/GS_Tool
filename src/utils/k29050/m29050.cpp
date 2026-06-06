#include "k29050/m29050.h"
QVector<double> m29050::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
