#include "k29750/m29750.h"
QVector<double> m29750::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
