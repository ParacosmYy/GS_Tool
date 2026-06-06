#include "m29912/m29912.h"
QVector<double> m29912::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
