#include "k29350/m29350.h"
QVector<double> m29350::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
