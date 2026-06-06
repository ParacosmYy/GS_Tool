#include "i9688/m9688.h"
QVector<double> m9688::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
