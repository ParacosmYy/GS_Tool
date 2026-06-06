#include "k29150/m29150.h"
QVector<double> m29150::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
